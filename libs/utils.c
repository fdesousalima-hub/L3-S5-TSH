#include "utils.h"

char *concat(char *value, char *value_to_add) {
    size_t length = strlen(value) + strlen(value_to_add); // calculate length of the two "string"

    value = realloc(value, sizeof(char) * (length + 1)); // realloc length (+ 1 for the '\0')
    if (value == NULL) { // check realloc return value
        perror("Realloc.\n");
        exit(errno);
    }
    value = strcat(value, value_to_add); // add second "string" (realloc already copied the content of the first)
    return value;
}

char *copy(char *value) {
    char *temp = malloc(sizeof(char) * (strlen(value) + 1)); // malloc length (+ 1 for the '\0')

    if (temp == NULL) { // check malloc return value
        perror("Malloc.\n");
        exit(errno);
    }
    temp = strcpy(temp, value); // copy content
    return temp;
}

void display(int fd, int count, ...) {
    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; i++) { // for each arg
        char *temp = va_arg(args, char *); // get the arg

        if (write(fd, temp, strlen(temp)) == -1) { // write it and check write return value
            perror("Write.\n");
            exit(errno);
        }
    }
    va_end(args);
}

int tar_in_path(char *path) {
    char *temp = copy(path);
    char *token = strtok(temp, "/");

    while (token != NULL) { // while token
        char *str = strstr(token, ".tar"); // if the token contains ".tar"

        if (str != NULL && strcmp(str, ".tar") == 0) { // if the token contains only ".tar" (not ".targz" for example)
            free(temp);
            return 1;
        }
        token = strtok(NULL, "/"); // continue browsing
    }
    free(temp);
    return 0;
}

void free_array(char **array, size_t size) {
    for (int i = 0; i < size; i++) { // for each cell in array
        free(array[i]); // free content
    }
    free(array); // at the end free array
}

void free_all(int count, ...) {
    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; i++) { // for each arg
        free(va_arg(args, void *)); // free it
    }
    va_end(args);
}

size_t path_size(char *path) {
    char *temp = copy(path);
    char *token;
    int count = 0;

    if (strchr(path, '/') == NULL) { // if path containing no "/" then size is 1
        count = 1;
    } else {
        token = strtok(temp, "/");
        while (token != NULL) { // browse path
            count++; // increment
            token = strtok(NULL, "/");
        }
    }
    free(temp);
    return count;
}

int cut_path(char **tokens, char *path, size_t size) {
    char *temp = copy(path);
    char *token;
    int index = 0;

    if (strchr(path, '/') == NULL) { // if path ontaining no "/" then just add path
        tokens[index] = copy(path);
    } else {
        token = strtok(temp, "/");
        while (token != NULL) { // browse path
            tokens[index++] = copy(token); // add token
            token = strtok(NULL, "/");
        }
    }
    tokens[size] = NULL; // array always end by "NULL"
    free(temp);
    return 1;
}

char *concat_tokens(char **tokens, size_t size){
  char *res=copy(tokens[0]);
  for (size_t i = 1; i < size; i++) {
    if (strcmp(tokens[i],"")==0 || strcmp(res,"")==0) {
      res=concat(res,tokens[i]);
    }else{
      res=concat(concat(res,"/"),tokens[i]);
    }
  }
  return res;
}

char *concat_tokens_no_previous(char **tokens,char *str, size_t size){
  char *res=copy("");
  int boolean = 0;
  char *last = get_last(str);
  for (size_t i = path_size(str)-1; i < size; i++) {
    if (strcmp(tokens[i],last)==0) {
      boolean=1;
    }
    if (boolean) {
      res=concat(concat(res,tokens[i]),"/");
    }
  }

  if(strcmp(res,"")!=0) res[strlen(res)-1]='\0';
  if(strcmp(last,"")!=0) free(last);
  return res;
}

char **setup_path_tokens(char **tokens, char *path, size_t size) {
    if (tokens == NULL) { // check malloc return value
        perror("Malloc.\n");
        exit(errno);
    }
    cut_path(tokens, path, size); // fill array
    return tokens;
}

char *delete_last(char *path) {
    char *temp = copy(path);
    char *token = strtok(temp, "/");
    char *last_token;
    int index = strlen(path) - 1;
    size_t length;

    if (token == NULL) { // nothing to delete
        return NULL;
    }
    while (token != NULL) { // browse path
        last_token = token; // save last each loop
        token = strtok(NULL, "/");
    }
    length = strlen(last_token) + 1; // get the length of the last token
    while (length > 0 && index >= 0) { // remplace last token by '\0'
        path[index] = '\0';
        length--;
        index--;
    }
    free(temp);
    return path;
}

char *get_last(char *path) {
    char *temp = copy(path);
    char *token = strtok(temp, "/");
    char *last_token = NULL;

    if (token == NULL) {
        free(temp);
        return path;
    }
    while (token != NULL) {
        last_token = token;
        token = strtok(NULL, "/");
    }
    char *res = copy(last_token);

    free(temp);
    return res;
}

int is_last_block(char *block) {
    for (int i = 0; i < 512; i++) { // for each char in 512
        if (block[i] != '\0') { // if one is not '\0'
            return 0;
        }
    }
    return 1;
}

int check_existence(char *path, int check_folder) {
    if (access(path, F_OK) == -1) { // if we can't access then file doesn't exist
        return 0;
    }
    struct stat infos; // get infos

    stat(path, &infos); // stat call
    if (check_folder == 0) { // if we check folder existence
        if (S_ISDIR(infos.st_mode)) { 
            return 0;
        }
    } else { // otherwise
        if (!S_ISDIR(infos.st_mode)) {
            return 0;
        }
    }
    return 1;
}

int check_existence_in_tar(char *path_to_tar, char *name, int check_folder_only, int open_no_create) {
    int fd = (open_no_create) ? open(path_to_tar, O_RDONLY) : open_tar_descriptor(path_to_tar, O_RDWR, -1); // check open_tar_descriptor documentation
    int count = 0;
    unsigned int size;
    struct posix_header block;
    ssize_t res;

    if (fd < 0) { // check open return value
        perror("Open.\n");
        return 0;
    }
    while ((res = read(fd, &block, BLOCKSIZE)) > 0) { // while end not reached
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
        if (strcmp(name, block.name) == 0) { // if file found
            if (check_folder_only != 0) { // if we check folder existence
                if (block.typeflag == '5') {
                    close(fd);
                    return 1;
                }
            } else { // otherwise
                if (block.typeflag != '5') {
                    close(fd);
                    return 1;
                }
            }
        }
    }
    close(fd);
    return 0;
}

char *replace_tilde_by_home(char *path){
  if (path[0] == '~') {
    size_t length = strlen(path);
    char *home = getenv("HOME");
    char *res = malloc((length + strlen(home)) * sizeof(char));
    if (res == NULL) {
      perror("Malloc.\n");
      return path;
    }
    path = &path[1];
    res = strcpy(res, home);
    res = strcat(res, path);
    path = res;
  }
  return path;
}

char *get_non_tar_path_part(char *path) {
    char *temp = copy(path);
    int init = 0;
    char *token;
    char *str;
    char *res = "";

    token = strtok(temp, "/");
    while (token != NULL) { // browse path
        str = strstr(token, ".tar");
        if (str != NULL && strcmp(str, ".tar") == 0) { // if the token contains only ".tar" (and not ".targz" for example)
            break;
        }
        if (init == 0) { // if it's the first loop
            if (path[0] == '/') { // if path is absolute
                res = concat(concat(copy("/"), token), "/"); // then start with "/"
            } else {
                res = concat(copy(token), "/"); // otherwise
            }
            init++; // increment
            token = strtok(NULL, "/");
        } else {
            res = concat(concat(res, token), "/"); // concat token
            token = strtok(NULL, "/");
        }
    }
    free(temp);
    if (strcmp(res, "") == 0) {
        res = copy(""); // copy so we don't return a local variable
    }
    return res;
}

char *get_tar_path_part(char *path) {
    char *temp = copy(path);
    int found = 0;
    char *token;
    char *str;
    char *res = copy("");

    token = strtok(temp, "/");
    while (token != NULL) { // browse path
        if (found == 0) { // if not found ".tar" already
            str = strstr(token, ".tar");
            if (str != NULL && strcmp(str, ".tar") == 0) { // if the token contains only ".tar" (and not ".targz" for example)
                found++;
            }
            token = strtok(NULL, "/");
            continue;
        } else if (found == 1) {
            found++;
        }
        res = concat(concat(res, token), "/"); // add token
        token = strtok(NULL, "/");
    }
    free(temp);
    if (!(strcmp(res, "") == 0) && path[strlen(path) - 1] != '/') { // if path don't end with "/" then don't add it
        res[strlen(res) - 1] = '\0';
    }
    return res;
}

char *get_tar_name(char *path){
    char *temp = copy(path);
    char *token;
    char *res = NULL;

    token = strtok(temp, "/");
    while (token != NULL) {
        if (strstr(token,".tar")) {
            res = copy(token);
            break;
        }
        token = strtok(NULL, "/");
    }
    free(temp);
    return res;
}

char *cd_util(char *path) {
    size_t size = path_size(path); // get the size
    char *future_path = (path[0] == '/') ? copy("/") : copy(getenv("CURRENT_PATH")); // if the path is absolute then start with "/" else start by tsh cwd
    char *tar_name = NULL;
    int index = 0;
    char *token;
    char **tokens = malloc(sizeof(char *) * (size + 1)); // malloc return value is check in setup_path_tokens function
    char *non_tar_path_part = NULL;
    char *tar_path_part = NULL;

    setup_path_tokens(tokens, path, size); // cf stup_path_tokens documentation
    token = tokens[index];
    while (token != NULL) { // browse path
        if (strcmp(token, ".") == 0) { // if token == "."
            token = tokens[++index]; // skip to next
            continue;
        } else if (strcmp(token, "..") == 0) { // if token == ".."
            if (delete_last(future_path) == NULL) { // if delete is not possible
                future_path = (strcmp(future_path, "~") == 0) ? replace_tilde_by_home(future_path) : copy("/"); // if tsh cwd = '~' then replace '~' by home else we are in root folder
            }
            if (future_path[strlen(future_path) - 1] != '/') { // add "/" for the next token
                future_path = concat(future_path, "/");
            }
        } else { // otherwise
            if (strcmp(future_path, "/") != 0 && strcmp(future_path, "~") != 0 && future_path[strlen(future_path) - 1] != '/') { // if the path is not "/" or "~" and doesn't end by "/"
                future_path = concat(future_path, "/"); // add "/"
            }
            future_path = concat(future_path, token); // concat next token
            if (tar_in_path(future_path)) { // if tar in path
                non_tar_path_part = get_non_tar_path_part(future_path);
                tar_path_part = get_tar_path_part(future_path);

                if (strcmp(tar_path_part, "") == 0) { // if the tar part is empty (for example "folder/another_folder/tarball.tar")
                    tar_name = copy(token); // get the tar_name
                    if (check_existence(future_path, 0) == 0 && check_existence((future_path = concat(future_path, "/")), 0) == 0) { // check the existence of the path with the tar in it
                        free_array(tokens, size);
                        free_all(4, future_path, non_tar_path_part, tar_path_part, tar_name);
                        return NULL;
                    }
                } else { // if tar part non empty
                    if (tar_name == NULL) { // if the tarname not set
                        if ((tar_name = get_tar_name(future_path)) == NULL) { // get the tarname and if it doesn't exist exist
                            free_array(tokens, size);
                            free_all(4, future_path, non_tar_path_part, tar_path_part, tar_name);
                            return NULL;
                        }
                    }
                    non_tar_path_part = concat(non_tar_path_part, tar_name);
                    if (tar_path_part[strlen(tar_path_part) - 1] != '/') { // if tar part doesn't end with '/' add it
                        tar_path_part = concat(tar_path_part, "/");
                    }
                    if (check_existence_in_tar(non_tar_path_part, tar_path_part, 1, 0) == 0) { // check the existence of the path 
                        free_array(tokens, size);
                        free_all(4, future_path, non_tar_path_part, tar_path_part, tar_name);
                        return NULL;
                    }
                }
                free_all(2, non_tar_path_part, tar_path_part);
            } else { // if no tar in the path
                if (check_existence(future_path, 1) == 0) { // check existence
                    free_array(tokens, size);
                    free_all(2, future_path, tar_name);
                    return NULL;
                }
            }
        }
        token = tokens[++index];
    }
    free_array(tokens, size);
    free(tar_name);
    return future_path;
}

char *cd_util_file(char *path) {
    if (path[strlen(path) - 1] == '/') { // if path end by "/" then it's not a file
        return NULL;
    }
    char *last = get_last(path); // get last
    char *simplified_path = delete_last(copy(path)); // delete last from path
    char *temp;

    if (strcmp(simplified_path, "") != 0) { // if path contained more than 1 token
        simplified_path = concat(simplified_path, "/");
        temp = copy(simplified_path); // this variable is here to avoid memory leak 
        free(simplified_path);
        simplified_path = cd_util(temp); // check if the path before the last token is valid
        free(temp);
        if (simplified_path == NULL) { // if not the entire path is invalid
            free(last);
            return NULL;
        }
    } else { // if path contained only 1 token
        simplified_path = cd_util("."); // get the tsh cwd
        if (simplified_path == NULL) { // impossible but here just in case
            free(last);
            return NULL;
        }

    }
    if (simplified_path[strlen(simplified_path) - 1] != '/') { // if simplified path doesn't end with '/' add it
        simplified_path = concat(simplified_path, "/");
    }
    simplified_path = concat(simplified_path, last); // concat simplified path and last
    free(last);
    if (tar_in_path(simplified_path)) { // if tar in simplified path
        char *tar_name = get_tar_name(simplified_path);
        char *non_tar_path_part = get_non_tar_path_part(simplified_path);
        char *tar_path_part = get_tar_path_part(simplified_path);

        non_tar_path_part = concat(non_tar_path_part, tar_name);
        if (strcmp(tar_path_part, "") != 0) { // if tar part is non empty
            if (check_existence_in_tar(non_tar_path_part, tar_path_part, 0, 0)) { // check existence
                path = copy(simplified_path);
                free_all(4, simplified_path, non_tar_path_part, tar_path_part, tar_name);
                return path;
            }
        }
        free_all(4, simplified_path, non_tar_path_part, tar_path_part, tar_name); // if tar part empty == last is a tarball and tarball are considered as folders in tsh
        return NULL;
    } else { // if no tar in simplified path
        if (check_existence(simplified_path, 0)) { // check existence
            path = copy(simplified_path);
            free(simplified_path);
            return path;
        }
        free(simplified_path);
        return NULL;
    }
}

int check_unicity(char *path) {
    char *last = get_last(path); // get last
    char *simplified_path = delete_last(copy(path)); // delete last
    char *temp;
    int res = -1;

    if (strcmp(simplified_path, "") != 0) { // if path contained more than 1 token
        simplified_path = concat(simplified_path, "/");
        temp = copy(simplified_path); // this variable is here to avoid memory leak 
        free(simplified_path);
        simplified_path = cd_util(temp); // check if the path before the last token is valid
        free(temp);
        if (simplified_path == NULL) { // if not the entire path is invalid
            free(last);
            return -1;
        }
        if (simplified_path[strlen(simplified_path) - 1] != '/') { // if simplified path doesn't end with '/' add it
            simplified_path = concat(simplified_path, "/");
        }
        simplified_path = concat(simplified_path, last); // concat simplified path and last
    } else { // if path contained only 1 token
        free(simplified_path);
        simplified_path = cd_util("."); // get the tsh cwd
        if (simplified_path == NULL) { // impossible but here just in case
            free(last);
            return -1;
        }
        if (strcmp(last, "..") == 0) { // if last is .. then delete last
            simplified_path = delete_last(simplified_path);
        } else {
            if (strcmp(last, ".") != 0) {
                if (simplified_path[strlen(simplified_path) - 1] != '/') { // if simplified path doesn't end with '/' add it
                    simplified_path = concat(simplified_path, "/");
                }
                simplified_path = concat(simplified_path, last); // concat simplified path and last
            }
        }
    }
    free(last);
    if (tar_in_path(simplified_path)) { // if tar in simplified path
        char *tar_name = get_tar_name(simplified_path);
        char *non_tar_path_part = get_non_tar_path_part(simplified_path);
        char *tar_path_part = get_tar_path_part(simplified_path);

        non_tar_path_part = concat(non_tar_path_part, tar_name);
        if (strcmp(tar_path_part, "") == 0) { // if tar part is empty then folder
            free_all(4, simplified_path, non_tar_path_part, tar_path_part, tar_name);
            return 1;
        }
        if (check_existence_in_tar(non_tar_path_part, tar_path_part, 1, 0)) { // else check folder existence (without "/")
            res = 1;
        }
        if (check_existence_in_tar(non_tar_path_part, tar_path_part, 0, 1)) { // check file existence
            res = 2;
        }
        tar_path_part = concat(tar_path_part, "/"); // add "/"
        if (check_existence_in_tar(non_tar_path_part, tar_path_part, 1, 1)) { // check folder existence (can't be file with "/" so no need to check)
            res = (res == 2) ? 0 : 1;
        }
        free_all(4, simplified_path, non_tar_path_part, tar_path_part, tar_name);
        return res;
    } else { // if no tar in simplified path
        if (check_existence(simplified_path, 1)) { // check folder existence
            res = 1;
        }
        if (check_existence(simplified_path, 0)) { // check file existence
            res = (res == 1) ? 0 : 2;
        }
        free(simplified_path);
        return res;
    }
}

char *cd_util_all(char *path){
  char *res;
  switch (check_unicity(path)) {
    case 0:
      if (path[strlen(path)-1]=='/') {
        res = cd_util(path);
        return res;
      }
      res = cd_util_file(path);
      return res;
    case 1:
      res = cd_util(path);
      return res;
   case 2:
      res = cd_util_file(path);
      return res;
   default:
      return NULL;
    }
}

int copy_fdcontent_to_fddestination(int fd_source,int nb_blocks,int fd_destination, int in_tar){
  char *read_buf = calloc(sizeof(char) * BLOCKSIZE,BLOCKSIZE);
  if (read_buf == NULL) {
    perror("Calloc");
    exit(errno);
  }
  for (size_t i = 0; i < nb_blocks; i++) {
    if (read(fd_source,read_buf,BLOCKSIZE) < 0) {
      perror("Read\n");
      return -1;
    }
    if (write(fd_destination, read_buf, (in_tar)? BLOCKSIZE : strlen(read_buf)) < 0) {
      perror("Write");
      return -1;
    }
    memset(read_buf,'\0',BLOCKSIZE);
  }
  free(read_buf);
  return 1;
}

int start_by(char *str,char *otherstr){
  if (strlen(str)<strlen(otherstr)) {
    return 0;
  }
  for (size_t i = 0; i < strlen(otherstr); i++) {
    if (str[i]!=otherstr[i]) {
      return 0;
    }
  }
  return 1;
}

int create_dirs_out_tar(int fd_source,char *dir_on_tar , char *destination){
  char *name_dir = (strcmp(dir_on_tar,"")==0)?"":copy(dir_on_tar);
  char *new_name = copy("");
  struct posix_header ph;
  size_t read_size_tar;
  while ((read_size_tar = read(fd_source,&ph, BLOCKSIZE)) > 0) {
    int blocksize ;
    unsigned int size;
    sscanf((char *) &ph.size, "%o", &size);
    blocksize = (size + BLOCKSIZE - 1) >> BLOCKBITS;
    if (start_by(ph.name,name_dir)) {
      if (ph.name[strlen(ph.name)-1]== '/') {
        if(strcmp(dir_on_tar,"")!=0 && strcmp(name_dir,dir_on_tar)==0){
          free(name_dir);
        }
        name_dir = ph.name;
        char *last=get_last(ph.name);
        if (strcmp(new_name,"")==0) {
          free(new_name);
          new_name = concat(copy(last),"/");
        }else{
          new_name = concat(concat(new_name,last),"/");
        }
        free(last);
        char *name_new_file = concat(concat(copy(destination),"/"),new_name);
        if(mkdir(name_new_file, 0755)==-1){
          perror("Mkdir");
          free(new_name);
          return -1;
        }
        free(name_new_file);
      }
    }
    lseek(fd_source,BLOCKSIZE*blocksize,SEEK_CUR);
  }
  free(new_name);
  if (read_size_tar < 0) {
    perror("Read");
    return -1;
  }
  return 1;
}

int create_files_out_tar(int fd_source,char *dir_or_file_on_tar , char *destination){
  struct posix_header ph;
  size_t read_size_tar;

  while ((read_size_tar = read(fd_source,&ph, BLOCKSIZE)) > 0) {
    int blocksize ;
    unsigned int size;
    sscanf((char *) &ph.size, "%o", &size);
    blocksize = (size + BLOCKSIZE - 1) >> BLOCKBITS;
    if (start_by(ph.name,dir_or_file_on_tar) && ph.name[strlen(ph.name)-1]!= '/' && strlen(ph.name)!=0) {
      char **tokens = malloc(sizeof(char *) * (path_size(ph.name) + 1));
      setup_path_tokens(tokens,ph.name,path_size(ph.name));
      char *all_tokens = (strcmp(dir_or_file_on_tar,"")==0)?concat_tokens(tokens,path_size(ph.name)):concat_tokens_no_previous(tokens,dir_or_file_on_tar,path_size(ph.name));
      char *name_new_file = (strcmp(destination,"")==0)? copy(all_tokens) : concat(concat(copy(destination),"/"),all_tokens);
      free(all_tokens);
      free_array(tokens,path_size(ph.name));
      int fd_destination = open(name_new_file,O_WRONLY | O_CREAT, 0755);

      free(name_new_file);
      if(fd_destination<0){
        perror("Open");
        return -1;
      }
      copy_fdcontent_to_fddestination(fd_source,blocksize,fd_destination,0);
      close(fd_destination);
    }else{
      lseek(fd_source,BLOCKSIZE*blocksize,SEEK_CUR);
    }
  }
  if (read_size_tar < 0) {
    perror("Read");
    return -1;
  }
  return 1;
}

int dir_out_tar_empty(char *dir){
  struct dirent *d;
  int n=0;
  DIR *dir_d = opendir(dir);
  if(dir_d == NULL){
      perror("OpendDir");
      return -1;
  }
  while ((d = readdir(dir_d)) != NULL) {
    if(++n > 2)
      break;
  }
  closedir(dir_d);
  if (n >= 2){
    return 0; //ERROR NOT EMPTY
  }
  return 1;
}

int check_recursive_option(char *argv[], int argc){
  for (int i= 0; i < argc; i++) {
      if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "-R") == 0 || strcmp(argv[i], "--recursive") == 0)
          return 1;
  }
  return 0;
}
