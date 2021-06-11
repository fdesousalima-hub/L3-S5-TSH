#include "utils.h"

#define MAX_INPUT_LENGTH 256
// #define COMMANDS_PATH "commands/bin/"
char COMMANDS_PATH[PATH_MAX];

void printWaitInputMessage();
/**
 * Strip the string in place (remove start and end spaces).
 * @return str
 */
char *strip(char *str);
/**
 * @return The user input. // needs rework -> ctrl+D = segmentation fault
 */
char *getTypedCommand();
/**
 * @param cmd - The command to split.
 * @param length - Pointer of int for storing the length of the returned array.
 * @return The splited command.
 */
char **splitCommand(char *cmd, int *length);
/**
 * @param cmd - For exemple the command "ls > file".
 * @param argv - The array to fill from cmd (NULL at the end).
 * @param argc - The length of argv (doesn't count NULL).
 * @param in - The file descriptor of the opened input file or STDIN_FILENO.
 * @param out - The file descriptor of the opened output file or STDOUT_FILENO.
 * @param err - The file descriptor of the opened error file or STDERR_FILENO.
 * @return 1 if error during the parsing, 0 otherwise;
 */
int parseCommand(char *cmd, char *argv[], int *argc, int *in, int *out, int *err);
/**
 * @return The command path if we have a custom command with name "cmd", NULL otherwise.
 */
char *getCustomCommandPath(char *cmd);
/**
 * Close the file descriptors if they are not standars.
 */
void closeInOutErr(int in, int out, int err);
/**
 * @param argv: Second argument of execv.
 * @param in: File descriptor of the input.
 * @param out: File descriptor of the output.
 * @param err: File descriptor of the error.
 * @return The pid of the subprocess (so -1 if error).
 */
pid_t executeCommand(char *argv[], int in, int out, int err);

/**
 * @author Hugo
 * @version 1.0
 * The cd function.
 * @param path The tested path.
 * @return 1 if the current shell path as been modified, and 0 otherwise.
 **/
int cd(char *path);

int main(int argc, const char *argv[]) {
	realpath("commands/bin/", COMMANDS_PATH); // doesn't count the last '/'
	COMMANDS_PATH[strlen(COMMANDS_PATH)] = '/';
	COMMANDS_PATH[strlen(COMMANDS_PATH) + 1] = '\0';

	char temp_buf[BUFSIZE];
	getcwd(temp_buf, BUFSIZE); // for the init, env var set to the shell cwd
	if (setenv("CURRENT_PATH", temp_buf, 1) == -1) { // check if setenv is working
		perror("Setenv.\n");
		exit(errno);
	}

    while (1) {
        printWaitInputMessage();

        char *cmd = getTypedCommand();
        if (cmd == NULL) {
            continue;
        }
        cmd = strip(cmd);
        if (cmd[0] == '\0') {
            continue;
        }

        int last_read = -1;
        char *sub;
        while ((sub = strsep(&cmd, "|")) > 0) {
            sub = strip(sub);
            if (sub[0] == '\0') {
                write(STDOUT_FILENO, "Syntax error near \"|\"\n", 22);
                break;
            }

            int pipefd[2];
            if (pipe(pipefd) == -1) {
				perror("pipe");
				break;
			}

            char *argv_[strlen(sub)];
			int argc_;
            int in = (last_read == -1) ? STDIN_FILENO : last_read,
                out = pipefd[1],
                err = STDERR_FILENO;
            if (parseCommand(sub, argv_, &argc_, &in, &out, &err) != 0) {
                closeInOutErr(in, out, err);
                break;
            }

            if (strcmp(argv_[0], "exit") == 0) {
                closeInOutErr(in, out, err);
				return EXIT_SUCCESS;
			}
            if (strcmp(argv_[0], "cd") == 0) {
                if (argc_ > 2) { // if cd is used with more than 1 arg
                    display(1, 1, "cd: usage: cd [path]\n"); // display error message
                    continue;
                } else if (argc_ == 1) { // if cd is used with no arg
                    cd(getenv("HOME")); // cd HOME
                } else {
                    cd(argv_[1]); // cd arg
                }
				closeInOutErr(in, out, err);
				continue;
            }

            pid_t pid = executeCommand(argv_, in, out, err);

            closeInOutErr(in, out, err);

            if (pid == -1) {
                perror("fork");
                break;
            }
            int status;
            wait(&status);

            last_read = pipefd[0];

            if (status != 0) {
                break;
            }
        };

        // Print the final output if any.
        if (last_read != -1) {
    		char buff[128];
    		int r;
    		while ((r = read(last_read, buff, 128)) > 0)
    			write(STDOUT_FILENO, buff, r);
    		close(last_read);
        }
    }
    return EXIT_SUCCESS;
}

void printWaitInputMessage() {
	char *path = getenv("CURRENT_PATH");
	write(STDOUT_FILENO, path, strlen(path));

	write(STDOUT_FILENO, "$ ", 2);
}

char *strip(char *str) {
    for (int i= 0; i < strlen(str); i++) {
        if (str[i] != ' ') {
            str += i;
            break;
        }
    }
    for (int i= strlen(str) - 1; i >= 0; i--) {
        if (str[i] != ' ') {
            str[i + 1] = '\0';
            break;
        }
    }
    return str;
}

char *getTypedCommand() {
	char *cmd = malloc(sizeof(char) * MAX_INPUT_LENGTH);
	if (cmd == NULL) {
		perror("malloc");
		return NULL;
	}

	int l = read(STDIN_FILENO, cmd, sizeof(char) * MAX_INPUT_LENGTH);
	cmd = realloc(cmd, sizeof(char) * (l + 1));
	if (cmd == NULL) {
		perror("realloc");
		return NULL;
	}
	if (cmd[l - 1] == '\n')
		cmd[l - 1] = '\0';

	return cmd;
}

char **splitCommand(char *cmd, int *length) {
    char **splited = malloc(sizeof(char *) * strlen(cmd));
    *length = 0;

    regex_t preg;
    int err = regcomp(&preg, "(<|(2>)|(2>>)|>|(>>))", REG_EXTENDED);
    if (err != 0)
        return NULL;

    size_t nmatch = preg.re_nsub;
    regmatch_t *pmatch = malloc(sizeof(regmatch_t) * nmatch);
    int match = regexec(&preg, cmd, nmatch, pmatch, 0);
    int last_end = 0;
    while (match == 0) {
        int start = pmatch[0].rm_so,
            end = pmatch[0].rm_eo;
        size_t size = end - start;
        char *separator = malloc(size + 1);
        if (separator == NULL) {
            perror("malloc");
            return NULL;
        }
        sprintf(separator, "%.*s", (int) size, &cmd[(last_end + start)]); 

        char *before = malloc((last_end + end) - last_end - strlen(separator) + 1); 
        if (before == NULL) {
            perror("malloc");
            return NULL;
        }
        sprintf(before, "%.*s", (last_end + end) - last_end - (int) strlen(separator), &cmd[last_end]); 
        before = strip(before); 

        splited[(*length)++] = before;
        splited[(*length)++] = separator;

        match = regexec(&preg, &cmd[(last_end + end)], nmatch, pmatch, 0);
        last_end += end;
    }
    if (match == REG_ESPACE) {
        return NULL;
    }
    if (cmd[last_end] != '\0') {
		char *end = malloc(strlen(cmd) - last_end + 1);
		if (end == NULL) {
			perror("malloc");
			return NULL;
		}
        sprintf(end, "%s", &cmd[last_end]);
        end = strip(end);

        splited[(*length)++] = end;
    }

    regfree(&preg);
	free(pmatch);

    splited = realloc(splited, sizeof(char *) * (*length));
	if (splited == NULL) {
		perror("realloc");
		return NULL;
	}
    return splited;
}

int parseCommand(char *cmd, char *argv[], int *argc, int *in, int *out, int *err) {
    char *cmd_cmd;

    int length;
    char **splited_cmd = splitCommand(cmd, &length);
    for (int i= 0; i < length; i++) {
        char *next = (i + 1 < length) ? splited_cmd[i + 1] : "";
        if (strcmp(splited_cmd[i], "<") == 0) {
            if (next[0] == '\0') {
                // print an error
                return 1;
            }
            if (*in != STDIN_FILENO)
                close(*in);
            *in = open(next, O_RDONLY);
            if (*in == -1) {
                perror("input open");
                return 1;
            }
            i++;
        }
        else if (strcmp(splited_cmd[i], ">") == 0) {
            if (next[0] == '\0') {
                // print an error
                return 1;
            }
            if (*out != STDOUT_FILENO)
                close(*out);
            *out = open(next, O_CREAT | O_WRONLY, 0644);
            if (*out == -1) {
                perror("output open");
                return 1;
            }
            i++;
        }
        else if (strcmp(splited_cmd[i], ">>") == 0) {
            if (next[0] == '\0') {
                // print an error
                return 1;
            }
            if (*out != STDOUT_FILENO)
                close(*out);
            *out = open(next, O_CREAT | O_APPEND | O_WRONLY, 0644);
            if (*out == -1) {
                perror("output open");
                return 1;
            }
            i++;
        }
        else if (strcmp(splited_cmd[i], "2>") == 0) {
            if (next[0] == '\0') {
                // print an error
                return 1;
            }
            if (*err != STDERR_FILENO)
                close(*err);
            *err = open(next, O_CREAT | O_WRONLY, 0644);
            if (*err == -1) {
                perror("error open");
                return 1;
            }
            i++;
        }
        else if (strcmp(splited_cmd[i], "2>>") == 0) {
            if (next[0] == '\0') {
                // print an error
                return 1;
            }
            if (*err != STDERR_FILENO)
                close(*err);
            *err = open(next, O_CREAT | O_APPEND | O_WRONLY, 0644);
            if (*err == -1) {
                perror("error open");
                return 1;
            }
            i++;
        }
        else { // should be called once
            cmd_cmd = strdup(splited_cmd[i]);
            if (cmd_cmd == NULL) {
                perror("malloc");
                return 1;
            }
        }
    }
	free(splited_cmd);

    *argc = 0;
    char *sub;
    while ((sub = strsep(&cmd_cmd, " ")) > 0) {
        if (sub[0] != '\0')
            argv[(*argc)++] = sub;
    }
	free(cmd_cmd);
    if (*argc == 0)
        return 1;
    argv[*argc] = NULL;

    return 0;
}

void closeInOutErr(int in, int out, int err) {
    if (in != STDIN_FILENO)
        close(in);
    if (out != STDOUT_FILENO)
        close(out);
    if (err != STDERR_FILENO)
        close(err);
}

char *getCustomCommandPath(char *cmd) {
	char *command_path = malloc(strlen(COMMANDS_PATH) + strlen(cmd) + 1);
	if (command_path == NULL) {
		perror("malloc");
		return NULL;
	}
	strcpy(stpcpy(command_path, COMMANDS_PATH), cmd);
	if (access(command_path, F_OK) == 0)
		return command_path;
	return NULL;
}

pid_t executeCommand(char *argv[], int in, int out, int err) {
    pid_t pid = fork();
    if (pid == 0) {
        dup2(in, STDIN_FILENO);
        dup2(out, STDOUT_FILENO);
        dup2(err, STDERR_FILENO);
        closeInOutErr(in, out, err);

        char *command_path = getCustomCommandPath(argv[0]);
        if (command_path != NULL)
            execv(command_path, argv);
        execvp(argv[0], argv);

        free(command_path);
        write(STDOUT_FILENO, argv[0], strlen(argv[0]));
        write(STDOUT_FILENO, ": command not found\n", 20);
        _exit(EXIT_FAILURE);
    }
    return pid;
}

int cd(char *path) {
	char *temp = cd_util(path); // simplify path
    char *non_tar_path_part;

	if (temp == NULL) { // invalid path
        display(1, 1, "cd: invalid path.\n"); // display error message
		return 0;
	}
	if (setenv("CURRENT_PATH", temp, strlen(temp)) == -1) { // update tsh cwd
		perror("Setenv.\n");
		exit(errno);
	}
    if (chdir((non_tar_path_part = get_non_tar_path_part(temp))) == -1) { // if no path in tsh cwd, change bash cwd
        perror("Chdir.\n");
        exit(errno);
    }
	free(temp); 
    free(non_tar_path_part);
	return 1;
}
