#include "../libs/utils.h"

/**
 * @author Fabio
 * @version 1.0
 * Exec cp from system
 * @param source Source of file
 * @param destination Destination to mv
 * @param recursive 1 if we want -r, else 0
 **/
static void copyNoTarToNoTar(char *source,char *destination,int recursive){
  int pid = fork();
  if (pid == -1) {
      perror("Fork");
      exit(1);
  }
  if (pid == 0) {
      if (recursive == 1) {
          char *argv[] = {"cp", "-r", source, destination, NULL};
          execvp(argv[0], argv);
      }
      else {
          char *argv[] = {"cp", source, destination, NULL};
          execvp(argv[0], argv);
      }
  }
  int status;
  waitpid(pid,&status,0);
}

/**
 * @author Fabio
 * @version 1.0
 * Check if we are on tar and exec custom cp or system cp
 * @param source Source for cp
 * @param source destination Destination for cp
 * @param recursive 1 if we want recursive, 0 else
 * @return 1 if success, -1 error
 **/
static int cpCustom(char *source,char *destination,int recursive){
  char *source_copy = copy(source);
  char *destination_copy = copy(destination);

  char *absolute_path_source = cd_util_all(source_copy); // Get absolute/simplified path for source
  free(source_copy);
  if (absolute_path_source == NULL) {
    free(destination_copy);
    char *last = get_last(source);
    if (write(1, "cp: Unable to move '", 20) == -1
     || write(1, last, strlen(last)) == -1
     || write(1, "' does not exist \n",18) == -1) {
      perror("Write");
    }
    free(last);
    return -1; //ERROR Source don't exist
  }
  if(check_unicity(absolute_path_source)==1 && !recursive){
    if (write(1, "cp: -r unspecified ; omission from directory '", 46) == -1
     || write(1, source, strlen(source)) == -1
     || write(1, "'\n",2) == -1) {
      perror("Write");
    }
    free(destination_copy);
    free(absolute_path_source);
    return -1; //ERROR Source don't exist
  }

  char *absolute_path_destination = cd_util_all(destination); // Get absolute/simplified path for destination

  if (absolute_path_destination == NULL) {// Check if path exist without last token
    char *last = get_last(destination_copy);
    delete_last(destination_copy);
    if (strcmp(destination_copy,"")!=0) {
      absolute_path_destination = cd_util_all(destination_copy);
      if (absolute_path_destination == NULL || tar_in_path(last)) {
        char *last_source = get_last(source);
        free(destination_copy);
        if (write(1, "cp: Unable to copy '", 20) == -1
         || write(1, last_source, strlen(last_source)) == -1
         || write(1, "', destination does not exist \n",31) == -1) {
          perror("Write");
        }
        free(absolute_path_source);
        free(last_source);
        free(last);
        return -1; //ERROR ALL PATH INCORCT
      }
    }else{
      absolute_path_destination = cd_util(".");
    }
    absolute_path_destination = concat(concat(absolute_path_destination,"/"),last);
    free(last);
  }
  free(destination_copy);
  char *last_source=get_last(absolute_path_source);
  if (tar_in_path(last_source) && tar_in_path(absolute_path_destination)) { // Check if we move root tar to other tar
    if (write(1, "cp: Cannot copy the root of a tar into another tar\n", 51) == -1) {
      perror("Write");
    }
    free(last_source);
    free(absolute_path_source);
    free(absolute_path_destination);
    return -1;
  }
  free(last_source);
  if (tar_in_path(absolute_path_destination)) { //Check if destination is in tar
    if (tar_in_path(absolute_path_source)) { //Check if source is in tar
      moveTarToTar(absolute_path_source,absolute_path_destination, 0);  // Exec copy tar to tar
    }else{
      moveNoTarToTar(absolute_path_source,absolute_path_destination, 0); // Exec copy no tar to tar
    }
  }else{
    if (tar_in_path(absolute_path_source)) { //Check if source is in tar
      char *tar_path_part = get_tar_path_part(absolute_path_source);
      if (strlen(tar_path_part)==0) { // Check if we copy a tar root
        if (check_unicity(absolute_path_destination)==-1) {
          if(mkdir(destination,0755)==-1){
            perror("Mkdir");
            free(absolute_path_source);
            free(absolute_path_destination);
            return -1;
          }
        }
        copyNoTarToNoTar(absolute_path_source,absolute_path_destination,recursive); // Exec system copy
      }else{
        moveTarToNoTar(absolute_path_source,absolute_path_destination,0); // Exec copy tar to no tar
      }
      free(tar_path_part);
    }else{
      copyNoTarToNoTar(absolute_path_source,absolute_path_destination,recursive);// Exec system copy
    }
  }
  free(absolute_path_source);
  free(absolute_path_destination);
  return 1;
}

/**
 * @author Fabio
 * @version 1.0
 * Main for cp
 **/
int main(int argc, char *argv[]) {
  if (argc < 3){
    if (write(1, "mkdir: missing operandy \n", 25) == -1) {
      perror("Write");
    }
    return 1;
  }
  int recursive = check_recursive_option(argv,argc);
  char *destination;
  if (strcmp(argv[argc-1], "-r") == 0 || strcmp(argv[argc-1], "-R") == 0 || strcmp(argv[argc-1], "--recursive") == 0){
    destination = argv[argc-2];
  }else{
    destination = argv[argc-1];
  }
  for (int i= 1; i < argc; i++) {
    if (argv[i][0] != '-' && strcmp(argv[i],destination)!=0) {
      cpCustom(argv[i],destination,recursive);
    }
  }
  return 0;
}
