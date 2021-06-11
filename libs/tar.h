#ifndef TAR_H
#define TAR_H

#include "utils.h"

/* tar Header Block, from POSIX 1003.1-1990.  */

#define BLOCKSIZE 512
#define BLOCKBITS 9

/* POSIX header.  */

/* Note that sizeof(struct posix_header) == BLOCKSIZE */

struct posix_header {           /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
  char junk[12];                /* 500 */
};                              /* Total: 512 */

#define TMAGIC   "ustar"        /* ustar and a null */
#define TMAGLEN  6
#define TVERSION "00"           /* 00 and no null */
#define TVERSLEN 2

/* ... */

#define OLDGNU_MAGIC "ustar  "  /* 7 chars and a null */

/* ... */


/* Compute and write the checksum of a header, by adding all
   (unsigned) bytes in it (while hd->chksum is initially all ' ').
   Then hd->chksum is set to contain the octal encoding of this
   sum (on 6 bytes), followed by '\0' and ' '.
*/

void set_checksum(struct posix_header *hd);

/* Check that the checksum of a header is correct */

int check_checksum(struct posix_header *hd);

/**
 * @author Clement
 * @version 1.0
 * @param fd - The opened tar file descriptor.
 * @param path - The path to search, for exemple "foo" or "foo/bar".
 * @param recursive - 1 for recursive.
 * @return 0 if error, else 1.
 */
int in_tar_rm(int fd, char *path, int recursive);

/**
 * @author Clement
 * @version 1.0
 * Find a file or a folder inside a .tar file. At the end of the function, SEEK_CUR
 * is just after the posix_header returned or at SEEK_END if not found.
 * @param fd - The opened tar file descriptor.
 * @param name - The path to search, for exemple "foo" or "foo/bar".
 * @return A pointer to a posix_header if found, NULL otherwise.
 */
struct posix_header *find_in_tar(int fd, char *name);

/**
 * @author Fabio
 * @version 1.0
 * Create a posix header based on a file or directory (Need free)
 * @param name The name of posix header
 * @param file_dir The path to file or directory
 * @return The posix_header created
 **/
struct posix_header *create_posix_header(char *name,char *file_dir);

/**
 * @author Fabio
 * @version 1.0
 * Create a posix header for new directory (Need free)
 * @param name The name of new folder
 * @return The posix_header created
 **/
struct posix_header *create_dir_in_tar(char *name);

/**
 * @author Fabio
 * @version 1.0
 * Create existing directory or file in tar
 * @param dir_or_file Existing file or directory
 * @param name Name of file or directory in tar
 * @param fd_destination Descriptor for tar
 * @return 1 Creation succes, -1 error
 **/
 int create_dir_or_file_in_tar(char *dir_or_file,char *name, int fd_destination, int delete);

/**
 * @author Fabio
 * @version 1.0
 * Create existing directory with content in tar
 * @param source Path to directory
 * @param last_source Last token of source
 * @param dir_or_file_on_tar Path to destination inside tar
 * @param fd_destination Descriptor for tar
 * @return 1 Creation succes, -1 error
 **/
 int create_dir_content_in_tar(char *source,char *last_source,char *dir_or_file_on_tar ,int fd_destination, int delete);

/**
 * @author Fabio
 * @version 1.0
 * Rename Directory or file in tar
 * @param fd_source Descriptor for tar
 * @param dir_or_file_on_tar Directory or file to rename
 * @param newname New name for file or directory
 * @param all Boolean if we rename all dir_or_file_on_tar 
 * @return 1 Rename succes, -1 error
 **/
int rename_in_tar(int fd_source,char *dir_or_file_on_tar , char *newname, int all);

/**
 * @author Fabio
 * @version 1.0
 * Write 2 empty blocks to end of tar
 * @param fd Descriptor for tar
 **/
void write_end_of_file_tar(int fd);

  /**
   * @author Fabio
   * @version 1.0
   * lseek before empty blocks
   * @param fd Descriptor for tar
   **/
void lseek_befor_emptyblocks(int fd);

/**
 * @author Fabio
 * @version 1.0
 * Check if a directory in tar is empty
 * @param fd Descriptor for tar
 * @param tar_path_part Directory in tar
 * @return 1 if is empty, 0 if is not , -1 if error
 **/
int dir_empty_in_tar(int fd,char *tar_path_part);

/**
 * @author Fabio
 * @version 1.0
 * Copy file or directory from a tar to a directory or file in a other(or same) tar
 * @param fd_source Descriptor for first tar
 * @param tar_path_part_source Directory or file to copy
 * @param fd_destination Descriptor for second tar
 * @param tar_path_part_destination Directory or file where is paste the copy
 * @return 1 Rename succes, -1 error
 **/
int copy_tar_tar(int fd_source,char *tar_path_part_source,int fd_destination, char *tar_path_part_destination);

/**
 * @author Fabio
 * @version 1.0
 * Open a tar and correct if is missing dir headers (Need close)
 * @param fd pathname Path to tar
 * @param flags Rigth for open
 * @param mode Permission if is created, -1 if no mode
 * @return Descriptor of tar
 **/
int open_tar_descriptor(char *pathname, int flags,  mode_t mode);

struct posix_header *find_in_tar(int fd, char *name);

#endif
