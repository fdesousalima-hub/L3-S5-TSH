#include "../libs/utils.h"

/**
 * @author Fabio
 * @version 1.0
 * Exec cat from system
 * @param arg The arguments for the command
 **/
static void execNoCustomCat(char *arg){
  char *args[]={"cat",arg,NULL};
  int status;
  pid_t pid = fork();
  switch (pid) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      execvp(args[0], args);
      break;
    default:
      waitpid(pid, &status, 0);
      break;
  }
}

/**
 * @author Fabio
 * @version 1.0
 * Exec cat in tar
 * @param argv Path for the command
 * @return 1 if success, -1 error
 **/
static int catArgInTar(char *path){
  char *tar_name = get_tar_name(path);
  char *non_tar_path_part = get_non_tar_path_part(path);
  non_tar_path_part = concat(non_tar_path_part,tar_name); //Name of tar to open
  free(tar_name);

  char *tar_path_part = get_tar_path_part(path); //File inside tar


  int fd = open(non_tar_path_part,O_RDONLY);
  free(non_tar_path_part);

  if (fd < 0) {
    perror("Open.\n");
    return -1;
  }

  struct posix_header ph;
  while (read(fd, &ph, BLOCKSIZE) > 0) { //Browse the tar and find file
    int blocksize ;
    unsigned int size;
    sscanf((char *) ph.size, "%o", &size);
    blocksize = (size + BLOCKSIZE - 1) >> BLOCKBITS;
    if (strcmp(ph.name, tar_path_part) == 0) { //Check if is file wanted
      copy_fdcontent_to_fddestination(fd, blocksize, 1, 0); //Write content in stdout
      return 1;
    }
    lseek(fd, blocksize * BLOCKSIZE, SEEK_CUR); //Go to other posix header
  }

  return 1;
}

/**
 * @author Fabio
 * @version 1.0
 * Check if we are on tar and exec custom cat or system cat
 * @param argv Path for the command
 * @return 1 if success, -1 error
 **/
static int catCustom(char *argv){
  char *arg = copy(argv);

  char *absolute_path = cd_util_file(arg); // Get absolute/simplified  path
  if (absolute_path == NULL) {
    free(arg);
    if (write(1, "cat: No such file \n", 19) == -1) {
      perror("Write");
    }
    return -1;
  }

  if (tar_in_path(absolute_path)) { //Check if path is in tar
    catArgInTar(absolute_path); //Exec cat on tar
  }else{
    execNoCustomCat(absolute_path); //Exec system cat
  }
  free(absolute_path);
  free(arg);
  return 1;
}

/**
 * @author Fabio
 * @version 1.0
 * Main for cat
 **/
int main(int argc, char *argv[]) {
  if (argc>1) {
    for (size_t i = 1; i < argc; i++) {
      catCustom(argv[i]);
    }
  }else{
    execNoCustomCat(NULL);
  }
  return 0;
}
