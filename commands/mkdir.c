#include "../libs/utils.h"

/**
 * @author Fabio
 * @version 1.0
 * Exec mkdir in tar
 * @param path Path for the command
 * @return 1 if success, -1 error
 **/
static int mkdirInTar(char *path){
  char *tar_name = get_tar_name(path);
  char *non_tar_path_part = get_non_tar_path_part(path);
  non_tar_path_part = concat(non_tar_path_part,tar_name);  //Name of tar to open
  free(tar_name);
  char *tar_path_part = get_tar_path_part(path);  //File inside tar
  if(check_unicity(path)!=-1){ //Check if file or directory exists
    char *tmp=get_last(path);
    if (write(1, "mkdir: Unable to create directory '", 35) == -1
     || write(1, tmp, strlen(tmp)) == -1
     || write(1, "': File exists\n",15) == -1) {
      perror("Write");
    }
    free(tmp);
    free(non_tar_path_part);
    free(tar_path_part);
    return -1;
  }
  int fd = open_tar_descriptor(non_tar_path_part,O_RDWR,-1);
  free(non_tar_path_part);
  if (fd == -1) {
    perror("Open source");
    free(tar_path_part);
    return -1;
  }
  if (tar_path_part[strlen(tar_path_part)-1]!='/') {
    tar_path_part=concat(tar_path_part,"/");
  }
  struct posix_header *ph = create_dir_in_tar(tar_path_part); //create posix_header
  free(tar_path_part);
  lseek_befor_emptyblocks(fd);
  if (write(fd, ph, BLOCKSIZE) == -1) {  //write posix_header
    perror("Write");
    free(ph);
    return -1;
  }
  free(ph);
  write_end_of_file_tar(fd);  //create end of file
  close(fd);
  return 1;
}

/**
 * @author Fabio
 * @version 1.0
 * Exec mkdir from system
 * @param path The arguments for the command
 **/
static void mkdirNoCustom(char *path){
  char *args[]={"mkdir",path,NULL};
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
 * Check if we are on tar and exec custom mkdir or system mkdir
 * @param argv Path for the command
 * @return 1 if success, -1 error
 **/
static int mkdirCustom(char *path){
  char *path_copy = copy(path);
  char *last = get_last(path_copy);
  delete_last(path_copy);
  char *absolute_path = (strcmp(path_copy,"")==0)?cd_util_all("."):cd_util_all(path_copy); // Get absolute/simplified path
  free(path_copy);
  if (absolute_path == NULL) {
    if (write(1, "mkdir: Unable to create directory '", 35) == -1
     || write(1, path, strlen(path)) == -1
     || write(1, "': No such file or folder\n",26) == -1) {
      perror("Write");
    }
    free(last);
    free(absolute_path);
    return -1;
  }
  absolute_path = concat(concat(absolute_path,"/"),last);
  if (tar_in_path(absolute_path)) {//Check if path is in tar
    mkdirInTar(absolute_path); //Exec mkdir on tar
  }else{
    mkdirNoCustom(absolute_path); //Exec system mkdir
  }
  free(absolute_path);
  free(last);
  return 1;
}

/**
 * @author Fabio
 * @version 1.0
 * Main for mkdir
 **/
int main(int argc, char *argv[]) {
  if (argc < 2) {
    if (write(1, "mkdir: missing operandy \n", 25) == -1) {
      perror("Write");
    }
    return 1;
  }
  for (size_t i = 1; i < argc; i++) {
    mkdirCustom(argv[i]);
  }
  return 0;
}
