#include "copy_move.h"

int moveNoTarToTar(char *source,char *destination, int delete){
  char *tar_name = get_tar_name(destination);

  char *non_tar_path_part = get_non_tar_path_part(destination);
  non_tar_path_part = concat(non_tar_path_part,tar_name);  //Name of tar to open
  free(tar_name);

  char *tar_path_part = get_tar_path_part(destination); //File inside tar

  int fd_destination = open_tar_descriptor(non_tar_path_part,O_RDWR,-1);
  if (fd_destination==-1) {
    perror("open source");
  }
  char *str;
  char *tmp;
  switch (check_unicity(source)) { //Check type of file
    // Case 0 don't exist because we can't have file and dir with same name out of tar
    case 1: //SOURCE IS DIR
    if (strcmp(tar_path_part,"")!=0) tar_path_part = concat(tar_path_part,"/");
    switch (check_unicity(destination)) {
      case -1: // dir -> no existing dir
        str= concat(concat(delete_last(copy(source)), "/"), tar_path_part);
        char **tokens = malloc(sizeof(char *) * (path_size(str) + 1));
        if (tokens == NULL)   {
          perror("Malloc");
          exit(errno);
        }
        tokens = setup_path_tokens(tokens,str,path_size(str));
        int i=0;
        char *alltokens=copy("");
        while(tokens[i]!=NULL){ //rename source
          if (i!=0 && !check_existence(alltokens,1)) {
            if(mkdir(alltokens,0755)==-1){
              perror("Mkdir");
              close(fd_destination);
              free_array(tokens, path_size(str));
              free_all(3, alltokens, non_tar_path_part, tar_path_part);
              return -1;
            }
          }
          alltokens=concat(concat(alltokens,"/"),tokens[i++]);
        }
        free_array(tokens, path_size(str));
        free(alltokens);
        rename(source,str);
        delete_last(tar_path_part);
        tmp=get_last(str);
        create_dir_content_in_tar(str,tmp,tar_path_part,fd_destination,delete);
        if (!delete) {
          rename(str,source);
        }
        free(tmp);
        free(str);
        break;
      case 0: //IF NOT UNIT MV ON DIR
      case 1: // dir -> dir
        tmp=get_last(source);
        str= concat((strcmp(tar_path_part,"")==0)?get_last(source):concat(copy(tar_path_part),tmp),"/");
        free(tmp);
        if (check_existence_in_tar(non_tar_path_part,str,1, 1)) { //Check if destination exist and is not empty
          if (dir_empty_in_tar(fd_destination,str)) {
            tmp=concat(copy(source),"/");
            free(str);
            str=get_last(source);
            create_dir_content_in_tar(tmp,str,tar_path_part,fd_destination,delete);
            free(tmp);
          }else{
            tmp=get_last(source);
            if (write(1, "mv: Unable to move '", 20) == -1
             || write(1, tmp, strlen(tmp)) == -1
             || write(1, "' destination is not empty\n",27) == -1) {
              perror("Write");
            }
            close(fd_destination);
            free(non_tar_path_part);
            free(tar_path_part);
            free(str);
            free(tmp);
            return -1;
          }
        }else{
          tmp=concat(copy(source),"/");
          free(str);
          str=get_last(source);
          create_dir_content_in_tar(tmp,str,tar_path_part,fd_destination,delete);
          free(tmp);
        }
        free(str);
        break;
      case 2: // dir -> file works
        if (write(1, "mv: Cannot move a folder to a file \n", 36) == -1) {
          perror("Write");
        }
        close(fd_destination);
        free(non_tar_path_part);
        free(tar_path_part);
        return -1;
      }
      break;
    case 2: //SOURCE IS FILE
      switch (check_unicity(destination)) {
        char *name_new_file;
        char **tokens;
        case -1: // file -> no existinf file WORKS
          create_dir_or_file_in_tar(source,tar_path_part,fd_destination,delete);
          break;
        case 0: // IF NOT UNIT MV ON DIR
        case 1: // file -> dir WORKS
          if(strcmp(tar_path_part,"")!=0) tar_path_part=concat(tar_path_part,"/");
          str=get_last(source);
          tmp=concat(copy(tar_path_part),str);
          if (!check_existence_in_tar(non_tar_path_part,tmp,0,0)){ //Check if destination exist
            name_new_file = copy(tar_path_part);
            tokens = malloc(sizeof(char *) * (path_size(source) + 1));
            if (tokens == NULL)   {
              perror("Malloc");
              exit(errno);
            }
            setup_path_tokens(tokens,source,path_size(source));
            free(str);
            free(tmp);
            str=get_last(source);
            tmp=concat_tokens_no_previous(tokens,str,path_size(source));
            free(str);
            name_new_file = concat(name_new_file,tmp);
            free(tmp);
            free_array(tokens,path_size(source));
            create_dir_or_file_in_tar(source,name_new_file,fd_destination,delete);
            free(name_new_file);
            break;
          }
          free(tar_path_part);
          tar_path_part=copy(tmp);
          free(str);
          free(tmp);
        case 2: // file -> file WAIT CLEM RM
          in_tar_rm(fd_destination,tar_path_part,0);
          lseek(fd_destination,0L,SEEK_SET);
          delete_last(tar_path_part);
          if(strcmp(tar_path_part,"")!=0) tar_path_part=concat(tar_path_part,"/");
          name_new_file = copy(tar_path_part);
          tokens = malloc(sizeof(char *) * (path_size(source) + 1));
          if (tokens == NULL)   {
            perror("Malloc");
            exit(errno);
          }
          setup_path_tokens(tokens,source,path_size(source));
          str=get_last(source);
          tmp=concat_tokens_no_previous(tokens,str,path_size(source));
          free(str);
          name_new_file = concat(name_new_file,tmp);
          free(tmp);
          free_array(tokens,path_size(source));
          create_dir_or_file_in_tar(source,name_new_file,fd_destination,delete);
          free(name_new_file);
          break;
      }
      break;
    }
  close(fd_destination);
  free(non_tar_path_part);
  free(tar_path_part);

  return 1;
}

 int moveTarToTar(char *source,char *destination, int delete){
  char *tar_name_source = get_tar_name(source);

  char *non_tar_path_part_source = get_non_tar_path_part(source);
  non_tar_path_part_source = concat(non_tar_path_part_source,tar_name_source); //Name of tar source to open
  free(tar_name_source);
  char *tar_path_part_source = get_tar_path_part(source);  //File inside tar of source
  char *tar_name_destination = get_tar_name(destination);

  char *non_tar_path_part_destination = get_non_tar_path_part(destination);
  non_tar_path_part_destination = concat(non_tar_path_part_destination,tar_name_destination);//Name of tar source to open
  free(tar_name_destination);

  char *tar_path_part_destination = get_tar_path_part(destination);//File inside tar of destination
  int fd_source = open_tar_descriptor(non_tar_path_part_source,O_RDWR,-1);
  free(non_tar_path_part_source);
  int fd_destination = open_tar_descriptor(non_tar_path_part_destination,O_RDWR,-1);
  char *str;
  char *tmp;
  switch (check_unicity(source)) { //Check type of file
    case 0: // IF NOT UNIT MV DIR
    case 1: //SOURCE IS DIR
    tar_path_part_source=concat(tar_path_part_source,"/");
    switch (check_unicity(destination)) {
      case -1: // dir -> no existinf dir WORK
        rename_in_tar(fd_source,tar_path_part_source,tar_path_part_destination,0);
        tar_path_part_destination[strlen(tar_path_part_destination)]='/';
        str=copy(tar_path_part_destination);
        delete_last(tar_path_part_destination);
        lseek(fd_source,0L,SEEK_SET);
        copy_tar_tar(fd_source,str,fd_destination,tar_path_part_destination);
        lseek(fd_source,0L,SEEK_SET);
        if (delete) {
          in_tar_rm(fd_source,str,1);
        }else{
          tar_path_part_source[strlen(tar_path_part_source)-1]='\0';
          rename_in_tar(fd_source,str,tar_path_part_source,1);
        }
        free(str);
        break;
      case 0: // IF NOT UNIT MV ON DIR
      case 1: // dir -> dir WORK
        if (strcmp(tar_path_part_destination,"")!=0) {
          tar_path_part_destination=concat(tar_path_part_destination,"/");
        }
        tmp=get_last(source);
        str= (strlen(tar_path_part_destination)==0)?get_last(source):concat(copy(tar_path_part_destination),tmp);
        free(tmp);
        str=concat(str,"/");
        if (check_existence_in_tar(non_tar_path_part_destination,str,1, 1)) {
          if (dir_empty_in_tar(fd_destination,str)) {
            copy_tar_tar(fd_source,tar_path_part_source,fd_destination,tar_path_part_destination);
            lseek(fd_source,0L,SEEK_SET);
            if (delete) {
              in_tar_rm(fd_source,tar_path_part_source,1);
            }
            free(str);
          }else{
            free(str);
            tmp=get_last(source);
            if (write(1, "mv: Cannot move '", 17) == -1
             || write(1, tmp, strlen(tmp)) == -1
             || write(1, "' destination is not empty \n",28) == -1 ) {
              perror("Write");
            }
            free(tmp);
            close(fd_destination);
            close(fd_source);
            free(tar_path_part_source);
            free(tar_path_part_destination);
            free(non_tar_path_part_destination);
            return -1;
          }
        }else{
          free(str);
          copy_tar_tar(fd_source,tar_path_part_source,fd_destination,tar_path_part_destination);
          lseek(fd_source,0L,SEEK_SET);
          if (delete) {
            in_tar_rm(fd_source,tar_path_part_source,1);
          }
        }
        break;
      case 2: // dir -> file WORK
        if (write(1, "mv: Cannot move a folder to a file \n", 36) == -1) {
          perror("Write");
        }
        close(fd_destination);
        close(fd_source);
        free(tar_path_part_source);
        free(tar_path_part_destination);
        free(non_tar_path_part_destination);
        return -1; // ERROR DIR -> FILE
      }
      break;
    case 2: //SOURCE IS FILE
      switch (check_unicity(destination)) {
        case -1: // file -> no existinf file WORK
          rename_in_tar(fd_source,tar_path_part_source,tar_path_part_destination,0);
          delete_last(tar_path_part_source);
          tmp=copy(tar_path_part_destination);
          str=concat(concat(copy(tar_path_part_source),"/"),tmp);
          free(tmp);
          delete_last(tar_path_part_destination);
          lseek(fd_source,0L,SEEK_SET);
          if(strcmp(tar_path_part_destination,"")!=0) tar_path_part_destination=concat(tar_path_part_destination,"/");
          copy_tar_tar(fd_source,str,fd_destination,tar_path_part_destination);
          lseek(fd_source,0L,SEEK_SET);
          if (delete) {
            in_tar_rm(fd_source,str,0);
          }else{
            tmp=get_last(source);
            tar_path_part_source=concat(concat(tar_path_part_source,"/"),tmp);
            free(tmp);
            rename_in_tar(fd_source,str,tar_path_part_source,1);
          }
          free(str);
          break;
        case 0: // IF NOT UNIT MV ON DIR
        case 1: // file -> dir WORK
          if(strcmp(tar_path_part_destination,"")!=0) tar_path_part_destination=concat(tar_path_part_destination,"/");
          str=get_last(source);
          tmp=concat(copy(tar_path_part_destination),str);
          if (!check_existence_in_tar(non_tar_path_part_destination,tmp,0,0)) {
            copy_tar_tar(fd_source,tar_path_part_source,fd_destination,tar_path_part_destination);
            lseek(fd_source,0L,SEEK_SET);
            if (delete) {
              in_tar_rm(fd_source,tar_path_part_source,0);
            }
            free(str);
            free(tmp);
            break;
          }
          free(tar_path_part_destination);
          tar_path_part_destination=copy(tmp);
          free(str);
          free(tmp);
        case 2: // file -> file WAIT CLEM RM
          in_tar_rm(fd_destination,tar_path_part_destination,0);
          lseek(fd_destination,0L,SEEK_SET);
          delete_last(tar_path_part_destination);
          if(strcmp(tar_path_part_destination,"")!=0) tar_path_part_destination=concat(tar_path_part_destination,"/");
          copy_tar_tar(fd_source,tar_path_part_source,fd_destination,tar_path_part_destination);
          lseek(fd_source,0L,SEEK_SET);
          if (delete) {
            in_tar_rm(fd_source,tar_path_part_source,0);
          }
          break;
      }
      break;
    }
    close(fd_destination);
    close(fd_source);
    free(tar_path_part_source);
    free(tar_path_part_destination);
    free(non_tar_path_part_destination);

  return 1;
}


int moveTarToNoTar(char *source,char *destination, int delete){
  char *tar_name = get_tar_name(source);

  char *non_tar_path_part = get_non_tar_path_part(source);  //Name of tar to open
  non_tar_path_part = concat(non_tar_path_part,tar_name);
  free(tar_name);

  char *tar_path_part = get_tar_path_part(source); //File inside tar

  int fd_source = open_tar_descriptor(non_tar_path_part,O_RDWR,-1);
  free(non_tar_path_part);
  if (fd_source==-1) {
    perror("open source");
  }
  char *str="";
  char *tmp;
  switch (check_unicity(source)) { //Check type of file
    case 0: // IF NOT UNIT MV  DIR
    case 1: //SOURCE IS DIR
    switch (check_unicity(destination)) {
      case -1: // dir -> no existinf dir WORK
        str=get_last(destination);
        tmp=get_last(source);
        if (strcmp(str,tmp)!=0) {
          rename_in_tar(fd_source,tar_path_part,str,0);
        }
        free(tmp);
        str=concat(str,"/");
        delete_last(tar_path_part);
        tmp=copy(str);
        free(str);
        str=concat(concat(copy(tar_path_part),"/"),tmp);
        free(tmp);
        delete_last(destination);
        lseek(fd_source,0,SEEK_SET);
        create_dirs_out_tar(fd_source,str,destination);
        lseek(fd_source,0L,SEEK_SET);
        create_files_out_tar(fd_source,str,destination);
        lseek(fd_source,0L,SEEK_SET);
        if (delete) {
          in_tar_rm(fd_source,str,1);
        }else{
          rename_in_tar(fd_source,str,tar_path_part,1);
        }
        break;
      case 1: // dir -> dir WORKS
        str=copy(destination);
        tmp=get_last(source);
        if (!tar_in_path(tmp)) {
          str=concat(concat(str,"/"),tmp);
        }
        if (check_existence(str,1)) {
          if (!dir_out_tar_empty(str)) { // NO EMPTY
            if (write(1, "mv: Unable to move '", 20) == -1
             || write(1, tmp, strlen(tmp)) == -1
             || write(1, "' destination is not empty\n",27) == -1) {
               perror("Write");
            }
            free(str);
            free(tmp);
            free(tar_path_part);
            return -1;
          }
        }
        free(tmp);
        create_dirs_out_tar(fd_source,tar_path_part,destination);
        lseek(fd_source,0L,SEEK_SET);
        create_files_out_tar(fd_source,tar_path_part,destination);
        lseek(fd_source,0L,SEEK_SET);
        if (delete) {
          in_tar_rm(fd_source,tar_path_part,1);
        }
        break;
      case 2: // dir -> file works
        if (write(1, "mv: Cannot move a folder to a file \n", 36) == -1) {
          perror("Write");
        }
        free(tar_path_part);
        close(fd_source);
        return -1; // ERROR DIR -> FILE
      }
      break;
    case 2: //SOURCE IS FILE
      switch (check_unicity(destination)) {
        case -1: // file -> no existinf file WORKS
          str=get_last(destination);
          rename_in_tar(fd_source,tar_path_part,str,0);
          delete_last(tar_path_part);
          tmp=copy(str);
          free(str);
          if (strcmp(tar_path_part,"")==0) {
            str=concat(copy(tar_path_part),tmp);
          }else{
            str=concat(concat(copy(tar_path_part),"/"),tmp);
          }
          free(tmp);
          delete_last(destination);
          lseek(fd_source,0L,SEEK_SET);
          create_files_out_tar(fd_source,str,destination);
          lseek(fd_source,0L,SEEK_SET);
          if (delete) {
            in_tar_rm(fd_source,str,0);
          }else{
            tmp=get_last(source);
            if (strcmp(tar_path_part,"")==0) {
              tar_path_part=concat(tar_path_part,tmp);
            }else{
              tar_path_part=concat(concat(tar_path_part,"/"),tmp);
            }
            free(tmp);
            rename_in_tar(fd_source,str,tar_path_part,1);
          }
          break;
        case 1: // file -> dir WORKS
          create_files_out_tar(fd_source,tar_path_part,destination);
          lseek(fd_source,0L,SEEK_SET);
          if (delete) {
            in_tar_rm(fd_source,tar_path_part,0);
          }
          break;
        case 2: // file -> file WORKS
          str=get_last(destination);
          rename_in_tar(fd_source,tar_path_part,str,0);
          delete_last(tar_path_part);
          tmp=copy(str);
          free(str);
          if (strcmp(tar_path_part,"")==0) {
            str=concat(copy(tar_path_part),tmp);
          }else{
            str=concat(concat(copy(tar_path_part),"/"),tmp);
          }
          free(tmp);
          delete_last(destination);
          lseek(fd_source,0L,SEEK_SET);
          create_files_out_tar(fd_source,str,destination);
          lseek(fd_source,0L,SEEK_SET);
          if (delete) {
            in_tar_rm(fd_source,str,0);
          }else{
            tmp=get_last(source);
            if (strcmp(tar_path_part,"")==0) {
              tar_path_part=concat(tar_path_part,tmp);
            }else{
              tar_path_part=concat(concat(tar_path_part,"/"),tmp);
            }
            free(tmp);
            rename_in_tar(fd_source,str,tar_path_part,1);
          }
          break;
      }
      break;
    }
  if (strcmp(str,"")!=0) free(str);
  free(tar_path_part);
  close(fd_source);
  return 1;
}
