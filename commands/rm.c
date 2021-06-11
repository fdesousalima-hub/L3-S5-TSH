#include "../libs/utils.h"

int recursive = 0;

/**
 * @return 0 if error, else 1
*/
int rm(char *path);

int main(int argc, char *argv[]) {
    if (argc < 2)
        write(STDOUT_FILENO, "rm: missing operand\n", 20);
    else {
        // Check options
        recursive = check_recursive_option(argv,argc);
        // rm
        for (int i= 1; i < argc; i++) {
            // To remove a file whose name starts with a '-', for example '-foo', use the command: rm ./-foo
            if (argv[i][0] != '-') {
                if (!rm(argv[i]))
                    return EXIT_FAILURE;
            }
        }
    }
    return EXIT_SUCCESS;
}

int systemRm(char *path) {
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        return 0;
    }
    if (pid == 0) {
        // Since we have only 1 option we can do this instad of storing an array of options.
        if (recursive == 1) {
            char *argv[] = {"rm", "-r", path, NULL};
            execvp(argv[0], argv);
        }
        else {
            char *argv[] = {"rm", path, NULL};
            execvp(argv[0], argv);
        }
    }
    int status;
    wait(&status);
    if (status != 0)
        return 0;
    return 1;
}

int rm(char *path) {
    char *final_path = cd_util_all(path);
	if (final_path == NULL) {
		write(STDOUT_FILENO, "rm: cannot remove '", 19);
		write(STDOUT_FILENO, path, strlen(path));
		write(STDOUT_FILENO, "': No such file or directory\n", 29);
		return 0;
	}
	char *tar_path_part = get_tar_path_part(final_path);
	if (tar_path_part[0] != '\0' && check_unicity(final_path) == 1)
		tar_path_part = concat(tar_path_part, "/");

    int return_value;
    if (tar_path_part[0] != '\0') {
		char *non_tar_path_part = get_non_tar_path_part(final_path);
		char *tar_name = get_tar_name(final_path);
		if (non_tar_path_part[strlen(non_tar_path_part) - 1] != '/')
			non_tar_path_part = concat(non_tar_path_part, "/");
		non_tar_path_part = concat(non_tar_path_part, tar_name);
	    free(tar_name);

        int fd = open_tar_descriptor(non_tar_path_part, O_RDWR, -1);
		free(non_tar_path_part);
        if (fd == -1) {
            perror("open");
            return 0;
        }

        return_value = in_tar_rm(fd, tar_path_part, recursive);

        close(fd);
    }
    else
		{ return_value = systemRm(final_path); }
	
	free(final_path);
	free(tar_path_part);

    return return_value;
}
