#include "../libs/utils.h"

/**
 * @return 0 if error, else 1
*/
int rmdir_(char *path);

int main(int argc, char *argv[]) {
    if (argc < 2)
        write(STDOUT_FILENO, "rmdir: missing operand\n", 23);
    else {
        for (int i= 1; i < argc; i++) {
            if (!rmdir_(argv[i]))
                return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int systemRmdir(char *path) {
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        return 0;
    }
    if (pid == 0) {
        char *argv[] = {"rmdir", path, NULL};
        execvp(argv[0], argv);
    }
    int status;
    wait(&status);
    if (status != 0)
        return 0;
    return 1;
}

int rmdir_(char *path) {
    char *final_path = cd_util_all(path);
	if (final_path == NULL) {
		write(STDOUT_FILENO, "rmdir: failed to remove '", 25);
		write(STDOUT_FILENO, path, strlen(path));
		write(STDOUT_FILENO, "': No such file or directory\n", 29);
		return 1;
	}
	if (check_unicity(final_path) != 1) {
		write(STDOUT_FILENO, "rmdir: failed to remove '", 25);
		write(STDOUT_FILENO, path, strlen(path));
		write(STDOUT_FILENO, "': Not a directory\n", 19);
		return 1;
	}
	char *tar_path_part = get_tar_path_part(final_path);
	if (tar_path_part[0] != '\0') 
		tar_path_part = concat(tar_path_part, "/");

    int return_value;
    if (tar_path_part[0] != '\0') {
		char *non_tar_path_part = get_non_tar_path_part(final_path);
		char *tar_name = get_tar_name(final_path);
		// if (non_tar_path_part[strlen(non_tar_path_part) - 1] != '/') 
			// non_tar_path_part = concat(non_tar_path_part, "/");
		non_tar_path_part = concat(non_tar_path_part, tar_name);
	    free(tar_name);

        int fd = open_tar_descriptor(non_tar_path_part, O_RDWR, -1); // open(non_tar_path_part, O_RDWR);
		free(non_tar_path_part);
        if (fd == -1) {
            perror("open");
            return 0; 
        }
		int dir_empty = dir_empty_in_tar(fd, tar_path_part);
		if (dir_empty == -1) {
			perror("dir_empty_in_tar");
			return 0;
		}
		if (dir_empty == 0) {
			write(STDOUT_FILENO, "rmdir: failed to remove '", 25);
			write(STDOUT_FILENO, path, strlen(path));
			write(STDOUT_FILENO, "': Directory not empty\n", 23);
			return 1;
		}
		else {
			lseek(fd, 0L, SEEK_SET);
			return_value = in_tar_rm(fd, tar_path_part, 1);
		}

        close(fd);
    }
    else
		{ return_value = systemRmdir(final_path); } // why guard ?
	
	free(final_path);
	free(tar_path_part);

    return return_value;
}
