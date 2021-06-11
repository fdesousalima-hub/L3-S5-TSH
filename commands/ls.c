#include "../libs/utils.h"

/**
 * @author Hugo
 * @version 1.0
 * Check if there is an option specified and if it's "-l".
 * Example: if av = ["ls", "folder/", "-l", "file", "another_folder/", "-a"], the function will return -1.
 * @param ac The arguments count.
 * @param av The arguments array.
 * @return -1 if there is an invalid option, 0 is there is no option and 1 otherwise.
 **/
static int check_options(int ac, char **av) {
	for (int i = 1; i < ac; i++) { // for each arguments (except first)
		if (av[i][0] == '-') { // if the first char is a '-'
			if (av[i][1] != 'l') { // if the second char is not a 'l' (invalid option)
				return -1;
			}
			if (strlen(av[i]) > 2) { // if there is more than '-l' ('-la' for example)
				return -1;
			}
			return 1; // option present
		}
	}
	return 0; // no option present
}

/**
 * @author Hugo
 * @version 1.0
 * Retrieve the arguments and put them in the array passed as parameter.
 * All the spaces not used will be replaced by NULL.
 * The options ("-l") will not be put in the array.
 * Example: if av = ["ls", "folder/", "-l", "file", "another_folder/", "-l"], the function will return ["folder/", "file", "another_folder", NULL].
 * @param ac The arguments count.
 * @param av The arguments array,
 * @param res The array to be filled.
 * @return The modified array containing the arguments (to be free'd).
 **/
static char **get_args(int ac, char **av, char **res) {
	int index = 0;

	for (int i = 1; i < ac; i++) { // for each arguments (except first)
		if (av[i][0] == '-') { // if the first char is a '-' (ignore)
			continue;
		}
		res[index++] = copy(av[i]); // otherwise add him
	}
	for (int i = index; i < ac; i++) { // fill the rest of the array with NULL
		res[i] = NULL;
	}
	return res;
}

/**
 * @author Hugo
 * @version 1.0
 * Check if there are tarballs in the arguments.
 * Example: if args = ["folder/", "folder/another_folder/", "folder/tarball.tar/file"], the function will return 1.
 * @param args The arguments to be checked.
 * @return 1 if there is at least one tarball in the arguments, and 0 otherwise.
 **/
static int tar_in_args(char **args) {
	int index = 0;

	while (args[index] != NULL) { // for each args
		if (tar_in_path(args[index++])) { // if there is a tar in the arg
			return 1;
		}
	}
	return 0;
}

/**
 * @author Hugo
 * @version 1.0
 * Execute the ls command on non_tar arguments.
 * @param av The arguments to be exec.
 **/
static void exec_ls(char **av) {
	switch (fork()) {
		case -1:
			perror("Fork.\n"); // error on fork
			exit(errno);
		case 0:
			if (execvp("ls", av) == -1) { // exec
				perror("Execvp.\n");
				exit(errno);
			}
		default:
			wait(NULL); // wait child end
			break;
	}
}

/**
 * @author Hugo
 * @version 1.0
 * Simplify the name to be displayed by the ls command.
 * Example: if model = "folder/" and current = "folder/another_folder/file" the function will return "another_folder/file".
 * @param model The model (the tar path part).
 * @param current The current posix header block name.
 * @return The simplified name (to be free'd).
 **/
static char *simplify_name(char *model, char *current) {
	int start = strcmp(model, "") == 0 ? 0 : model[strlen(model) - 1] == '/' ? strlen(model) : strlen(model) + 1; // if model is empty, start at 0 else start after model
	int index = 0;
	char *res = malloc(sizeof(char) * strlen(current) - start + 1);

	if (res == NULL) { // check malloc return value
		perror("Malloc.\n");
		exit(errno);
	}
	for (int i = start; i < strlen(current); i++) { // for each char from start (init before)
		res[index++] = current[i]; // add char to res
	}
	res[index] = '\0'; // a "string" always end with '\0'
	return res;
}

/**
 * @author Hugo
 * @version 1.0
 * Simplify the name to be displayed by the ls command in the form: name -> link_path.
 * @param model The model (the tar path part).
 * @param block The posix_header representing the file.
 * @return The simplified name (to be free'd).
 **/
static char *link_name(char *model, struct posix_header block) {
	char *res = simplify_name(model, block.name); // simplify the name

	res = concat(res, " -> ");
	res = concat(res, block.linkname); // concat simplified name to "-> linkname"
	return res;
}

/**
 * @author Hugo
 * @version 1.0
 * Create an array to execute the ls system command properly.
 * WARNING: The path must contain no tarball.
 * @param arg The path of the file/directory.
 * @param option 1 if the display is set to the -l option, and 0 otherwise.
 **/
static void build_and_exec(char *path, int option) {
	char *to_exec[option ? 4 : 3]; // if option need 4 length and 3 otherwise

	to_exec[0] = "ls"; // does this really need documentation ?
	to_exec[1] = path;
	to_exec[2] = option ? "-l" : NULL;
	if (option) {
		to_exec[3] = NULL;
	}
	exec_ls(to_exec);
}

/**
 * @author Hugo
 * @version 1.0
 * Check if the model and the current are equals (folder and folder/ are equals in fact but strcmp doesn't return 0 on this test).
 * Example: "folder" and "folder/" will be equals
 * @param model The model (the tar path part).
 * @param current The current posix header block name.
 * @return 1 if the arguments are "equals" (are the same folder) and 0 otherwise.
 **/
static int check_equality(char *model, char *current) {
	if (strcmp(model, current) == 0) { // if equals then end
		return 1;
	}
	char *temp = copy(current); // otherwise copy

	temp[strlen(temp) - 1] = '\0'; // remove "/" at the end 
	if (strcmp(model, temp) == 0) { // if equals without "/" then end
		free(temp);
		return 1;
	}
	free(temp); // otherwise
	return 0;
}

/**
 * @author Hugo
 * @version 1.0
 * Check if the current posix header starts by the form "model".
 * Example: if model = "folder/" and current = "folder/another_folder/file", the function will return 1.
 * @param model The model (the tar path part).
 * @param current The current posix header block name.
 * @return 1 if the name has the good form, and 0 otherwise.
 **/
static int check_form(char *model, char *current) {
	char *temp = copy(current); // copy the current (so we can modified it)
	char *last = strcmp(model, "") == 0 ? NULL : get_last(model); // if model is empty then NULL else the last token of the model
	char *token = strtok(temp, "/"); // strtok the current copy
	int count = 0;
	int found = 0;

	if (check_equality(model, current) || strstr(current, model) == NULL) { // if model == current or model doesn't appear in model then false
		free_all(2, temp, last);
		return 0;
	}
	while (token != NULL) { // for each token in current copy
		if (last == NULL || found != 0) { // if model's last == NULL or already found
			if (++count > 1) { // if already went in this conditional loop then false
				free_all(2, temp, last);
				return 0;
			}
		} else {
			if (strcmp(token, last) == 0) { // if found token in current copy that equals model's last then found
				found++;
			}
		}
		token = strtok(NULL, "/"); // continue browsing
	}
	free_all(2, temp, last); 
	return 1;
}

/**
 * @author Hugo
 * @version 1.0
 * Return the type of the file.
 * The function return "-" (regular file) if the typeflag is:
 * \0                        - other representation of a regular file
 * 1                         - hard links (only symbolics links are represented with l in ls -l)
 * 7                         - reserved typeflag
 * not between 0 and 7 or \0 - should't be possible
 * If you want to know more about the typeflag 2 and 7 (reserved), check documentation on: https://www.gnu.org/software/tar/manual/html_node/Standard.html
 * @param block The posix_header representing the file.
 * @return One of those (to be free'd):
 * d - directory
 * b - special file in block mode
 * c - special file in character mode
 * l - symbolic link
 * p - named pipe
 * - - other
 **/
static char *get_type(struct posix_header block) {
	switch (block.typeflag) { // match typeflag and associte it with a letter
		case '2':
			return copy("l");
		case '3':
			return copy("c");
		case '4':
			return copy("b");
		case '5':
			return copy("d");
		case '6':
			return copy("p");
		default:
			return copy("-");
	}
}

/**
 * @author Hugo
 * @version 1.0
 * Return a char pointer representing the file rights.
 * @param block The posix_header representing the file.
 * @return A char pointer (to be free'd).
 **/
static char *get_rights(struct posix_header block) {
	int temp = atoi(block.mode); // convert mode to int
	char rights[3];
	char *res = copy("");

	sprintf(rights, "%d", temp); // fill rights[3] with mode
	for (int i = 0; i < 3; i++) {
		switch (rights[i]) { // for each group, match right with his representation
			case '0':
				res = concat(res, "---");
				break;
			case '1':
				res = concat(res, "--x");
				break;
			case '2':
				res = concat(res, "-w-");
				break;
			case '3':
				res = concat(res, "-wx");
				break;
			case '4':
				res = concat(res, "r--");
				break;
			case '5':
				res = concat(res, "r-x");
				break;
			case '6':
				res = concat(res, "rw-");
				break;
			case '7':
				res = concat(res, "rwx");
				break;
		}
	}
	return res;
}

/**
 * @author Hugo
 * @version 1.0
 * Get the size of the file.
 * The size is stored in a pointer of size 12 (end by a \0), so the -l display keeps a good form, even if all sizes have differents length.
 * @param block The posix_header representing the file.
 * @return A pointer representing the size (to be free'd).
 **/
static char *get_size(struct posix_header block) {
	char *type = get_type(block); // get the type (for "-" files)
	char *size_tmp;
	char *res = malloc(sizeof(char) * 19); // size can be maximum 2 * 8 (size of devmajor or devminor) + ", " so 18 + '\0' so 19
	unsigned int size;
	int index = 17; // start writting at 17 (18 is for the '\0')

	if (res == NULL) { // check malloc return
		perror("Malloc.\n");
		exit(errno);
	}
	memset(res, ' ', 17); // set the return "string" to full ' '
	if (block.typeflag == '5') { // size is 4096 for folders
		size_tmp = copy("4096");
	} else if (block.typeflag == '2') { // size is 9 for sym links
		size_tmp = copy("9");
	} else if (block.typeflag == '6') { // size is 0 for FIFO
		size_tmp = copy("0");
	} else if (block.typeflag == '3' || block.typeflag == '4') { // size is "<devmajor>, <devminor>" for special files
		char *temp = copy(""); // has to be malloc'd for sprintf

		sscanf((char *) &block.devmajor, "%o", &size); // convert devmajor to int (octal to decimal)
		sprintf(temp, "%u", size); // convert int to char *
		size_tmp = copy(temp); // add it to res
		size_tmp = concat(size_tmp, ", "); // add ", " to res
		sscanf((char *) &block.devminor, "%o", &size); // convert devminor to int (octal to decimal)
		sprintf(temp, "%u", size); // convert int to char *
		size_tmp = concat(size_tmp, temp); // add it to res
		free(temp);
	} else if (strcmp(type, "-") == 0) { // otherwise it's just the size
		size_tmp = copy("");
		sscanf((char *) &block.size, "%o", &size); // convert size to int (octal to decimal)
		sprintf(size_tmp, "%u", size); // convert int to char *
	}
	free(type);
	for (int i = strlen(size_tmp) - 1; i >= 0; i--) { // write the size from the end into res (so they all have the same display)
		res[index--] = size_tmp[i];
	}
	res[18] = '\0'; // a "string" always end with '\0'
	return copy(res); // copy so the variable is not local
}

/**
 * @author Hugo
 * @version 1.0
 * Get the block usage (second column in the -l display).
 * If the file is a folder, then the block usage is 2 + the number of folders in it.
 * Otherwise, the block usage is 1.
 * @param model The model (the tar path part).
 * @param type The file type.
 * @param path The path to the archive.
 * @return A pointer representing the number of block used (to be free'd).
 **/
static char *get_block_usage(char *model, char *type, char *path) {
	if (strcmp(type, "-") == 0) { // if it's a regular file then usage is 1
		return copy("1");
	}
	int fd = open(path, O_RDONLY);
	int total = 2;
	int count = 0;
	unsigned int size;
	struct posix_header block;

	if (open < 0) { // check open return value
		perror("Open.\n");
		exit(errno);
	}
	while (read(fd, &block, BLOCKSIZE) > 0) { // while end not reached
		if (count != 0) { // if we are not in a header
			count--; // ignore block
			continue;
		} else {
			sscanf((char *) &block.size, "%o", &size); // calculate number of blocks since next header (convert size to int (octal to decimal))
			count = (size + BLOCKSIZE - 1) >> BLOCKBITS;
		}
		if (is_last_block((char *) &block)) { // if the header is a full '\0' block
			break;
		}
		if (block.typeflag == '5') { // if folder
			if (check_form(model, block.name)) { // if the current header represent a file in the model
				total++;
			}
		}
	}
	int temp = floor(log10(abs(total))) + 1; // calculate the length of the number for display (if total = 100 then 3, if total = 12345 then 5)
	char *res = malloc(sizeof(char) * (temp + 1)); // malloc size of temp + 1 (for the '\0')

	if (res == NULL) { // check malloc return value
		perror("Malloc.\n");
		exit(errno);
	} 
	sprintf(res, "%d", total); // fill the pointer with the value
	return res;
}

/**
 * @author Hugo
 * @version 1.0
 * Get the month of the last modication date/creation date and return it in a good form.
 * @param block The posix_header representing the file.
 * @return A pointer of the form "xxx" where xxx are the 3 first letter of the month (to be free'd).
 **/
static char *get_month(struct posix_header block) {
	time_t time;
	struct tm ts;
	char buf[3];
	char *res;

	sscanf((char *) &block.mtime, "%lo", &time); // convert mtime to int (octal to decimal)
	ts = *localtime(&time); // convert time (time is expressed in Unix Epoch Time in tarball)
	strftime(buf, sizeof(buf), "%m", &ts); // convert time to a month of the year (in number)

	if (buf[0] == '0') { 
		switch (buf[1]) {
			case '1': // 01
				res = copy("jan.");
				break;
			case '2': // 02
				res = copy("feb.");
				break;
			case '3': // 03
				res = copy("mar.");
				break;
			case '4': // 04
				res = copy("apr.");
				break;
			case '5': // 05
				res = copy("may ");
				break;
			case '6': // 06
				res = copy("jun.");
				break;
			case '7': // 07
				res = copy("jul.");
				break;
			case '8': // 08
				res = copy("aug.");
				break;
			case '9': // 09
				res = copy("sep.");
				break;
		}
	} else {
		switch (buf[1]) {
			case '0': // 10
				res = copy("oct.");
				break;
			case '1': // 11
				res = copy("nov.");
				break;
			case '2': // 12
				res = copy("dec.");
				break;
		}
	}
	return res;
}

/**
 * @author Hugo
 * @version 1.0
 * Get the last modification date/creation date.
 * @param block The posix_header representing the file.
 * @return A pointer of the form "dd hh:mm" where xx, hh and mm represents the days, the hours and the minutes (to be free'd).
 **/
static char *get_time(struct posix_header block) {
	time_t time;
	struct tm ts;
	char buf[9];

	sscanf((char *) &block.mtime, "%lo", &time); // convert mtime to int (octal to decimal)
	ts = *localtime(&time); // convert time (time is expressed in Unix Epoch Time in tarball)
	strftime(buf, sizeof(buf), "%d %H:%M", &ts); // convert time to "day hours:minutes" (in number)
	return copy(buf);
}

/**
 * @author Hugo
 * @version 1.0
 * Browse the tar archive and return the total number of blocks taken up by the files.
 * @param fd The file descriptor to the archive.
 * @param path The path to the archive.
 * @param model The model (the tar path part).
 * @return The number of blocks otherwise.
 **/
static int get_total(int fd, char *path, char *model) {
	int count = 0;
	int total = 0;
	unsigned int size;
	struct posix_header block;

	while (read(fd, &block, BLOCKSIZE) > 0) { // while end not reached
		if (count != 0) { // if we are not in a header
			count--; // ingore block
			continue;
		} else {
			sscanf((char *) &block.size, "%o", &size); // calculate number of blocks since next header (convert size to int (octal to decimal))
			count = (size + BLOCKSIZE - 1) >> BLOCKBITS;
		}
		if (is_last_block((char *) &block)) { // if the header is a full '\0' block
			break;
		}
		if (check_form(model, block.name)) { // if the current header represent a file in the model
			char *POSIXLY_CORRECT = getenv("POSIXLY_CORRECT"); // check tarball documentation
			char *temp = get_size(block);
			int BLOCKLENGTH = (POSIXLY_CORRECT == NULL) ? 1024 : 512; // if the env var is set then 512 else 1024 by default
			int size =  atoi(temp); // convert size to int (already convert in decimal)

			total += (size < BLOCKLENGTH) ? 4 : (size % BLOCKLENGTH == 0) ? size / BLOCKLENGTH : (size / BLOCKLENGTH) + 1; // if size < blocklength then 4 else size / blocklength (+ 1)
			free(temp);
		}
	}
	lseek(fd, 0L, SEEK_SET); // reset cursor
	return total;
}

/**
 * @author Hugo
 * @version 1.0
 * Long display (for the -l option).
 * @param block The posix_header representing the file.
 * @param model The model (the tar path part).
 * @param path The path to the archive.
 **/
static void long_display(struct posix_header block, char *model, char *path) {
	// get infos
	char *res = copy(""); 
	char *type = get_type(block);
	char *rights = get_rights(block); 
	char *block_usage = get_block_usage(block.name, type, path);
	char *size = get_size(block);
	char *month = get_month(block);
	char *time = get_time(block);
	char *simplified_name = (strcmp(type, "l") == 0) ? link_name(model, block) : simplify_name(model, block.name);

	// concat infos one after other
	res = concat(concat(concat(res, type), rights), " "); 
	res = concat(concat(res, block_usage), " ");
	res = concat(concat(res, block.uname), " ");
	res = concat(concat(res, block.gname), " ");
	res = concat(concat(res, size), " ");
	res = concat(concat(res, month), " ");
	res = concat(concat(res, time), " ");
	res = concat(concat(res, simplified_name), "\n");

	display(1, 1, res); // display

	free_all(8, res, type, rights, block_usage, size, month, time, simplified_name); // free
}

/**
 * @author Hugo
 * @version 1.0
 * Browse the tar archive and display what it corresponds to the content of the model.
 * @param path The path to the archive.
 * @param model The model (the tar path part).
 * @param option 1 if the display is set to the -l option, and 0 otherwise.
 * @return 0 if the path is invalid and 1 otherwise.
 **/
static int read_infos(char *path, char *model, int option) {
	int fd = open_tar_descriptor(path, O_RDWR, -1); // check open_tar_descriptor documentation 
	int count = 0;
	unsigned int size;
	struct posix_header block;

	if (fd < 0) { // check open return value
		perror("Open.\n");
		return 0;
	}
	if (option) { // if option then browse tarball to find total and display it
		char temp[50];

		sprintf(temp, "%d", get_total(fd, path, model));
		display(1, 3, "total ", temp, "\n");
	}
	while (read(fd, &block, BLOCKSIZE) > 0) { // while end not reached
		if (count != 0) { // if we are not in a header
			count--; // ignore block
			continue;
		} else {
			sscanf((char *) &block.size, "%o", &size); // calculate number of blocks since next header (convert size to int (octal to decimal))
			count = (size + BLOCKSIZE - 1) >> BLOCKBITS;
		}
		if (is_last_block((char *) &block)) { // if the header is a full '\0' block
			break;
		}
		if (check_form(model, block.name)) { // if the current header represent a file in the model
			if (option) {
				long_display(block, model, path); // if "-l" long display
			} else {
				char *temp = simplify_name(model, block.name); // else display name and jump next line

				display(1, 2, temp, "\n");
				free(temp);
			}
		}
	}
	close(fd);
	return 1;
}

/**
 * @author Hugo
 * @version 1.0
 * Display the content of the specified path.
 * @param path The complete path.
 * @param option 1 if the display is set to the -l option, and 0 otherwise.
 **/
static void display_content(char *path, int option) {
	char *temp = cd_util_all(path); // check path

	if (temp == NULL) { // if NULL then doesn't exist
		display(1, 3, "ls: cannot acces '", path, "': No such file or directory\n"); // print error message
		return;
	}
	if (!tar_in_path(temp)) { // if no tar after simplification of the path
		build_and_exec(temp, option); // exec
		free(temp);
		return;
	}
	char *non_tar_path_part = get_non_tar_path_part(temp);
	char *tar_path_part = get_tar_path_part(temp);
	char *tar_name = get_tar_name(temp);
	char *last;

	non_tar_path_part = concat(non_tar_path_part, tar_name);
	switch (check_unicity(temp)) { // check unicity of the path
		case -1: // impossible but here just in case
			display(1, 3, "ls: cannot access '", path, "': No such file or directory\n"); // print error message
			break;
		case 0: // if the path is not unique (folder and file in the same directory (possible in a tarball))
			if (path[strlen(path) - 1] == '/') { // if path end by "/"
				read_infos(non_tar_path_part, tar_path_part, option); // display content
				break;
			} else {
				display(1, 1, path); // display name of the file
				break;
			}
		case 1: // if the path represent a directory
			read_infos(non_tar_path_part, tar_path_part, option); // display content
			break;
		default: // if the path represent a file
			last = get_last(path); // get the name of the file
			display(1, 2, last, "\n"); // display name of the file
			free(last);
			break;
	}
	free_all(4, temp, non_tar_path_part, tar_path_part, tar_name);
}

/**
 * @author Hugo
 * @version 1.0
 * List content.
 * @param ac The arguments count.
 * @param av The arguments.
 * @param option 1 if the display is set to the -l option, and 0 otherwise.
 * @return Always 1.
 **/
int ls(int ac, char **av, int option) {
	char *args[ac];
	int index = 0;
	int size = 0;
	
	get_args(ac, av, args); // setup args array (cf get_args documentation)
	if (!tar_in_args(av) && !tar_in_path(getenv("CURRENT_PATH"))) { // if no tar in args and no tar in tsh cwd
		for (int i = 0; i < ac; i++) { // for each arg
			free(args[i]); // free him
		}
		exec_ls(av); // sys ls
		return 1;
	}
	if ((option && ac == 2) || (!option && ac == 1)) { // if "ls" or "ls -l" (no args after == display cwd content)
		display_content(".", option);
		return 1;
	}
	while (args[index++] != NULL) { // calculate the number of arguments
		size++;
	}
	index = 0;
	while (args[index] != NULL) { // for each arg
		if (index != 0) { // jump line for clean display (except for first one)
			display(1, 1, "\n");
		}
		if (size > 1) { // if more than one display, display the name of the arg and ":"
			display(1, 2, args[index], ":\n");
		}
		display_content(args[index++], option); // display content
	}
	for (int i = 0; i < ac; i++) { // at then end free all args (cf get_args documentation)
		free(args[i]);
	}
	return 1;
}

int main(int ac, char **av) {
	switch (check_options(ac, av)) { // check the options in args
		case -1: // option present and/or not "-l"
			if (tar_in_args(av)) { // if tar in args then invalid option
				display(1, 1, "ls: invalid option\n");
				return 0;
			}
			exec_ls(av); // otherwise sys ls
			return 0;
		case 0: // no option
			return ls(ac, av, 0) == 1 ? 0 : 1; 
		default: // ls -l display
			return ls(ac, av, 1) == 1 ? 0 : 1;
	}
}