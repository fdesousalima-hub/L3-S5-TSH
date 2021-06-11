#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdarg.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <linux/limits.h>
#include <grp.h>
#include <pwd.h>

#include "tar.h"
#include "copy_move.h"

/**
 * @author Hugo
 * @version 1.0
 * Macro defining the size of the buffers.
 **/
#define BUFSIZE 512

/**
 * @author Hugo
 * @version 1.0
 * Concatenate the content of the second parameter after that of the first one.
 * @param value The first pointer.
 * @param value_to_add The second pointer.
 * @return The concatenation of the two parameters content.
 **/
char *concat(char *value, char *value_to_add);

/**
 * @author Hugo
 * @version 1.0
 * Copy the content of the given pointer to a new one.
 * @param value The content to copy.
 * @return The copied content in a new pointer.
 **/
char *copy(char *value);

/**
 * @author Hugo
 * @version 1.0
 * Display the arguments on the specified output.
 * INFO: This function was created for the sole purpose of avoiding overloading the code of our functions each time we use the write function (we must tests its return).
 * @param fd The output descriptor.
 * @param count The number of tokens to print.
 * @param ... The arguments (WARNING: Those must have (char *) type).
 **/
void display(int fd, int count, ...);

/**
 * @author Hugo
 * @version 1.0
 * Search in the given path if there is a tarball.
 * This function only checks if there is a file ending with ".tar" (without checking the posix header...).
 * @param path The tested path.
 * @return 0 if there is no tarball in the given path, and 1 otherwise.
 **/
int tar_in_path(char *path);

/**
 * @author Hugo
 * @version 1.0
 * Free the given array content and the given array.
 * @param array The array to free.
 * @param size The length of the array.
 **/
void free_array(char **array, size_t size);

/**
 * @author Hugo
 * @version 1.0
 * Free the arguments.
 * @param count The number of tokens to free.
 * @param .. The arguments (WARNING: Those must be malloc'd/realloc'd/calloc'd to avoid invalid free: This function will not check if the args can be free'd).
 **/
void free_all(int count, ...);

/**
 * @author Hugo
 * @version 1.0
 * Calculate the number of tokens in a path (separated by '/').
 * @param path The tested path.
 * @return The number of tokens.
 **/
size_t path_size(char *path);

/**
 * @author Hugo
 * @version 1.0
 * Cut the given path in tokens (separated by '/') and fill the array passed as parameter with them. End by NULL.
 * @param tokens The pointer to be filled.
 * @param path The path to cut.
 * @param size The number of tokens in the path.
 * @return Always 1.
 **/
int cut_path(char **tokens, char *path, size_t size);

/**
 * @author Fabio
 * @version 1.0
 * Concatenate tokens to a path
 * @param tokens The tokens to concatenate
 * @param size The number of tokens in the path.
 * @return Path of concatenation
 **/
char *concat_tokens(char **tokens, size_t size);

/**
 * @author Fabio
 * @version 1.0
 * Concatenate tokens to a path without tokens before a string
 * @param tokens The tokens to concatenate
 * @param tokens The string for start concatenate
 * @param size The number of tokens in the path.
 * @return Path of concatenation
 **/
char *concat_tokens_no_previous(char **tokens,char *str, size_t size);

/**
 * @author Hugo
 * @version 1.0
 * Setup the given array by filling it, place a  NULL at the end etc...
 * @param tokens The pointer to be filled.
 * @param path The path to cut.
 * @param size Tne number of tokens in the path.
 * @return The array setup.
 **/
char **setup_path_tokens(char **tokens, char *path, size_t size);

/**
 * @author Hugo
 * @version 1.0
 * Delete the last token of the given path if possible.
 * @param path The tested path.
 * @return NULL if there is nothing to delete (empty path) and the new path with the last token deleted otherwise.
 **/
char *delete_last(char *path);

/**
 * @author Fabio
 * @version 1.0
 * Get the last token of path
 * @param path Path
 * @return The last token
 **/
char *get_last(char *path);

/**
 * @author Hugo
 * @version 1.0
 * Test if the given tar block if an end block (containing only '\0').
 * @param block The tested tar block.
 * @return 1 if the tested block is an end block, and 0 otherwise.
 **/
int is_last_block(char *block);

/**
 * @author Hugo
 * @version 1.0
 * Test if the file pointed by the given path exists.
 * @param path The tested path.
 * @param check_folder Non-zero if you want to check the existence of a folder, and 0 otherwise.
 * @return 1 if it's an existing file/folder, and 0 otherwise.
 **/
int check_existence(char *path, int check_folder);

/**
 * @author Hugo
 * @version 1.0
 * Test if the file pointed by the given path exists.
 * The given path has to be in a tar.
 * @param path_to_tar The tested path.
 * @param name The path in the tar of the requested file/folder.
 * @param check_folder_only Non-zero if you want to check the existence of a folder, and 0 otherwise.
 * @param open_no_create Zero if you want to create missing folders, and non-zero otherwise.
 * @return 1 if it's an existing file/folder, and 0 otherwise.
 **/
int check_existence_in_tar(char *path_to_tar, char *name, int check_folder_only, int open_no_create);

/**
 * @author Fabio
 * @version 1.0
 * Replace ~ by user home
 * @param path Path with ~
 * @return Path without ~
 **/
char *replace_tilde_by_home(char *path);

/**
 * @author Hugo
 * @version 1.0
 * Cut the given path to obtain a pointer (char *) containing the non-tar part of the path.
 * For example, if you call the function with 'folder/sub_folder/tarball.tar/folder_in_tar/' this function will return 'folder/sub_folder/'.
 * @param path The path to cut.
 * @return NULL if a problem occurred (malloc or realloc type), empty string if there is no non-tar part (""), and the non-tar part otherwise.
 **/
char *get_non_tar_path_part(char *path);

/**
 * @author Hugo
 * @version 1.0
 * Cut the given path to obtain a pointer (char *) containing the tar part of the path.
 * For example, if you call the function with 'folder/sub_folder/tarball.tar/folder_in_tar/' this function will return 'folder_in_tar/'.
 * @param path The path to cut.
 * @return NULL if a problem occurred (malloc or realloc type), empty string if there is no tar part (""), and the tar part otherwise.
 **/
char *get_tar_path_part(char *path);

/**
 * @author Fabio
 * @version 1.0
 * Cut the given path to obtain a pointer (char *) containing the tar name of the path.
 * @param path The path to cut.
 * @return The tar name
 **/
char *get_tar_name(char *path);

/**
 * @author Hugo
 * @version 1.0
 * Return the new path of the shell (without modifying the shell current path) from the concatenation of the current shell path and the given path.
 * Also check the given path validity.
 * Also remove '.' and applicate '..' if possible.
 * @param path The tested path.
 * @return NULL if the path is invalid, and the new current path (to be add) otherwise.
 **/
char *cd_util(char *path);

/**
 * @author Hugo
 * @version 1.0
 * Act like cd_util but for path pointing to a file.
 * WARNING: Don't modify the shell current path with a path returned by this function: this has to be done with the cd_util function.
 * @param path The tested path.
 * @return NULL if the path is invalid or if the path points to a folder, and the simplified path otherwise.
 **/
char *cd_util_file(char *path);

/**
 * @author Hugo
 * @version 1.0
 * Check if the file at the end of the path is unique or not.
 * If not, tell if it's a folder or a file.
 * @param path The tested path.
 * @return -1 if the path is invalid, 0 if the file is not unique, 1 if it's a folder and 2 otherwise.
 **/
int check_unicity(char *path);

/**
 * @author Fabio
 * @version 1.0
 * Transform path to absolute path simplified for file or directory
 * @param path The path to file or directory
 * @return The absolute path
 **/
char *cd_util_all(char *path);

/**
 * @author Fabio
 * @version 1.0
 * Read nb_blocks on source descriptor and write on destination descriptor
 * @param fd_source Descriptor to read (Need open with read)
 * @param nb_blocks Length to read
 * @param fd_destination Descriptor to write (Need open with write)
 * @param in_tar 1 if we write in tar, 0 other
 * @return 1 if is succes, -1 error
 **/
int copy_fdcontent_to_fddestination(int fd_source,int nb_blocks,int fd_destination,int in_tar);

/**
 * @author Fabio
 * @version 1.0
 * Check if first string start by the second string
 * @param str First string
 * @param otherstr Second string
 * @return 1 if start, 0 don't start
 **/
int start_by(char *str,char *otherstr);

/**
 * @author Fabio
 * @version 1.0
 * Extract existing directory in tar
 * @param fd_source Descriptor for tar
 * @param dir_on_tar Directory for extract
 * @param destination Path to destination
 * @return 1 Creation succes, -1 error
 **/
int create_dirs_out_tar(int fd_source,char *dir_on_tar , char *destination);

/**
 * @author Fabio
 * @version 1.0
 * Extract existing file in tar
 * @param fd_source Descriptor for tar
 * @param dir_or_file_on_tar File for extract
 * @param destination Path to destination
 * @return 1 Creation succes, 0 error
 **/
int create_files_out_tar(int fd_source,char *dir_or_file_on_tar , char *destination);

/**
 * @author Fabio
 * @version 1.0
 * Check if a directory is emtpy
 * @param dir Path to dir
 * @return 1 if is empty,0 is not, -1 error
 **/
int dir_out_tar_empty(char *dir);

/**
 * @author Fabio, Clement
 * @version 1.0
 * Check if option -r is present on arguments
 * @param argv All arguments
 * @param argc Number of arguments
 * @return 1 if -r is present 0 is not
 **/
int check_recursive_option(char *argv[],int argc);

#endif
