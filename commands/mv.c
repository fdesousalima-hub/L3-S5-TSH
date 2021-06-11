#include "../libs/utils.h"

/**
 * @author Fabio
 * @version 1.0
 * Exec mv from system
 * @param source Source of file
 * @param destination Destination to mv
 **/
static void moveNoTarToNoTar(char *source,char *destination){
  char *args[]={"mv",source,destination,NULL};
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
 * Check if we are on tar and exec custom mv or system mv
 * @param source Source for mv
 * @param source destination Destination for mv
 * @return 1 if success, -1 error
 **/
static int mvCustom(char *source,char *destination){
  char *source_copy = copy(source);
  char *destination_copy = copy(destination);

  char *absolute_path_source = cd_util_all(source_copy);// Get absolute/simplified path for source
  free(source_copy);
  if (absolute_path_source == NULL) {
    free(destination_copy);
    char *last = get_last(source);
    if (write(1, "mv: Unable to move '", 20) == -1
     || write(1, last, strlen(last)) == -1
     || write(1, "' does not exist \n",18) == -1) {
      perror("Write");
    }
    free(last);
    return -1; //ERROR Source don't exist
  }

  char *absolute_path_destination = cd_util_all(destination); // Get absolute/simplified path for destination

  if (absolute_path_destination == NULL) { // Check if path exist without last token
    char *last = get_last(destination_copy);
    delete_last(destination_copy);
    if (strcmp(destination_copy,"")!=0) {
      absolute_path_destination = cd_util_all(destination_copy);
      if (absolute_path_destination == NULL || tar_in_path(last)) {
        char *last_source = get_last(source);
        free(destination_copy);
        if (write(1, "mv: Unable to move '", 20) == -1
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
    if (write(1, "mv: Cannot move the root of a tar into another tar\n", 51) == -1) {
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
      moveTarToTar(absolute_path_source,absolute_path_destination, 1); // Exec move tar to tar
    }else{
      moveNoTarToTar(absolute_path_source,absolute_path_destination, 1); // Exec move no tar to tar
    }
  }else{
    if (tar_in_path(absolute_path_source)) { //Check if source is in tar
      char *tar_path_part = get_tar_path_part(absolute_path_source);
      if (strlen(tar_path_part)==0) { // Check if we move a tar root
        if (check_unicity(absolute_path_destination)==-1) {
          if(mkdir(destination,0755)==-1){
            perror("Mkdir");
            free(absolute_path_source);
            free(absolute_path_destination);
            return -1;
          }
        }
        moveNoTarToNoTar(source,destination); // Exec system move
      }else{
        moveTarToNoTar(absolute_path_source,absolute_path_destination,1);// Exec move tar to no tar
      }
      free(tar_path_part);
    }else{
      moveNoTarToNoTar(absolute_path_source,absolute_path_destination); // Exec system move
    }
  }
  free(absolute_path_source);
  free(absolute_path_destination);
  return 1;
}

/**
 * @author Fabio
 * @version 1.0
 * Main for mv
 **/
int main(int argc, char *argv[]) {
    char temp_buf[BUFSIZE];
    getcwd(temp_buf, BUFSIZE); // for the init, env var set to the shell cwd
    setenv("CURRENT_PATH", temp_buf, 1);
  if (argc < 3) {
    if (write(1, "mv: missing operandy \n", 22) == -1) {
      perror("Write");
    }
    return 1;
  }
  char *destination = argv[argc-1];
  for (size_t i = 1; i < argc-1; i++) {
    mvCustom(argv[i],destination);
  }
  return 0;
}
