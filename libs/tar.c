#include "tar.h"

void set_checksum(struct posix_header *hd) {
  memset(hd->chksum,' ',8);
  unsigned int sum = 0;
  char *p = (char *)hd;

  for (int i = 0; i < BLOCKSIZE; i++) {
    sum += p[i];
  }
  sprintf(hd->chksum,"%06o",sum);
}

int check_checksum(struct posix_header *hd) {
  unsigned int checksum;

  sscanf(hd->chksum,"%o ", &checksum);
  unsigned int sum = 0;
  char *p = (char *) hd;

  for (int i = 0; i < BLOCKSIZE; i++) {
    sum += p[i];
  }
  for (int i = 0; i < 8;i++) {
    sum += ' ' - hd->chksum[i];
  }
  return (checksum == sum);
}

/**
 * SEEK_CUR has to be at block start
 */
static void remove_blocks(int fd, int block_count) {
	char buff[BLOCKSIZE];
	unsigned int count = 0;
	while (pread(fd, buff, BLOCKSIZE, lseek(fd, 0, SEEK_CUR) + (block_count + count) * BLOCKSIZE) > 0) {
		pwrite(fd, buff, BLOCKSIZE, lseek(fd, 0, SEEK_CUR) + count * BLOCKSIZE);
		count++;
	}
	
	char t[BLOCKSIZE * block_count];
    memset(t, '\0', BLOCKSIZE * block_count);
    pwrite(fd, t, BLOCKSIZE * block_count, lseek(fd, 0, SEEK_CUR) + count * BLOCKSIZE);
}

// not verry optimized (remove_blocks should be called once for better performance) 
int in_tar_rm(int fd, char *path, int recursive) {
	struct posix_header ph;
    while (read(fd, &ph, BLOCKSIZE) > 0) {
		unsigned int ph_size;
		sscanf((char *) ph.size, "%o", &ph_size);
        size_t block_count = (ph_size + BLOCKSIZE - 1) >> BLOCKBITS;

        if (recursive == 1) {
            char sub[strlen(path) + 1];
            memcpy(sub, ph.name, strlen(path));
            sub[strlen(path)] = '\0';
            if (strcmp(sub, path) == 0) {
                lseek(fd, -BLOCKSIZE, SEEK_CUR);
				remove_blocks(fd, 1 + block_count);
				continue;
            }
        }
        else if (strcmp(ph.name, path) == 0) {
			if (ph.typeflag == '5') {
				write(STDOUT_FILENO, "rm: cannot remove '", 19);
				write(STDOUT_FILENO, path, strlen(path));
				write(STDOUT_FILENO, "': Is a directory\n", 18);
				return 0;
			}
			lseek(fd, -BLOCKSIZE, SEEK_CUR);
			remove_blocks(fd, 1 + block_count);
			break;
        }
        lseek(fd, BLOCKSIZE * block_count, SEEK_CUR);
    }
    return 1;
}

struct posix_header *find_in_tar(int fd, char *name) {
    struct posix_header *ph = malloc(sizeof(struct posix_header));
    while (read(fd, ph, BLOCKSIZE) > 0) {
        if (strcmp(ph->name, name) == 0)
            return ph;
        unsigned int ph_size;
		sscanf((char *) ph->size, "%o", &ph_size);
        size_t block_count = (ph_size + BLOCKSIZE - 1) >> BLOCKBITS;
        lseek(fd, BLOCKSIZE * block_count, SEEK_CUR);
    }
    return NULL;
}

struct posix_header *create_posix_header(char *name, char *file_dir){
  struct stat st;

  if(lstat(file_dir, &st)==-1){
    perror("lstat");
    return NULL;
  }
  struct posix_header *ph_new_dir_or_file = calloc(sizeof(struct posix_header),sizeof(struct posix_header));

  if (ph_new_dir_or_file == NULL)   {
    perror("Calloc");
    exit(errno);
  }

  strcpy(ph_new_dir_or_file->name, name);
  sprintf(ph_new_dir_or_file->mode, "%o", st.st_mode);
  sprintf(ph_new_dir_or_file->uid, "%u", st.st_uid);
  sprintf(ph_new_dir_or_file->gid, "%u", st.st_gid);
  sprintf(ph_new_dir_or_file->size, "%lo", st.st_size);
  sprintf(ph_new_dir_or_file->mtime, "%lo", st.st_mtime);
  switch (st.st_mode & S_IFMT) {
    case S_IFBLK:  ph_new_dir_or_file->typeflag = '4'; break;
    case S_IFCHR:  ph_new_dir_or_file->typeflag = '3'; break;
    case S_IFDIR:  ph_new_dir_or_file->typeflag = '5';
                   sprintf(ph_new_dir_or_file->size, "%o", 0);
                   break;
    case S_IFIFO:  ph_new_dir_or_file->typeflag = '6'; break;
    case S_IFLNK:  ph_new_dir_or_file->typeflag = '1';  break;
    case S_IFREG:  ph_new_dir_or_file->typeflag = '0'; break;
    default:       return NULL;
  }
  //strcpy(ph_new_dir_or_file->linkname, " "); //TODO LINKS
  strcpy(ph_new_dir_or_file->magic,TMAGIC);
  strcpy(ph_new_dir_or_file->version,TVERSION);
  struct passwd *user = getpwuid(st.st_uid);

  strcpy(ph_new_dir_or_file->uname, user->pw_name);
  struct group *group = getgrgid(st.st_gid);

  strcpy(ph_new_dir_or_file->gname, group->gr_name);
  sprintf(ph_new_dir_or_file->devmajor, "%u", major(st.st_rdev));
  sprintf(ph_new_dir_or_file->devminor, "%u", minor(st.st_rdev));
  set_checksum(ph_new_dir_or_file);
  return ph_new_dir_or_file;
}

struct posix_header *create_dir_in_tar(char *name){
  struct posix_header *ph_dir= calloc(sizeof(struct posix_header),sizeof(struct posix_header));

  if (ph_dir == NULL)   {
    perror("Calloc");
    exit(errno);
  }
  strcpy(ph_dir->name, name);
  sprintf(ph_dir->mode, "%o", 000775);
  sprintf(ph_dir->uid, "%u", getuid());
  sprintf(ph_dir->gid, "%u", getgid());
  sprintf(ph_dir->size, "%o", 0);

  time_t current_time;
  time(&current_time);
  sprintf(ph_dir->mtime, "%lo", current_time);
  ph_dir->typeflag = '5';

  //strcpy(ph_new_dir_or_file->linkname, " "); //TODO LINKS
  strcpy(ph_dir->magic,TMAGIC);
  strcpy(ph_dir->version,TVERSION);
  struct passwd *user = getpwuid(getuid());
  strcpy(ph_dir->uname, user->pw_name);
  struct group *group = getgrgid(getgid());
  strcpy(ph_dir->gname, group->gr_name);
  sprintf(ph_dir->devmajor, "%u", 0); //TODO
  sprintf(ph_dir->devminor, "%u", 0);
  set_checksum(ph_dir);

  return ph_dir;
}

void write_end_of_file_tar(int fd){
  lseek(fd,0L,SEEK_END);
  char *buf = calloc(BLOCKSIZE*2,BLOCKSIZE*2);
  if (buf == NULL)   {
    perror("Calloc");
    exit(errno);
  }
  if (write(fd,buf,BLOCKSIZE*2) < 0) {
    perror("Write");
  }
  free(buf);
}

void lseek_befor_emptyblocks(int fd){
  lseek(fd,-BLOCKSIZE,SEEK_END);
  struct posix_header ph;
  while(read(fd,&ph,BLOCKSIZE) == BLOCKSIZE){
    if(!is_last_block((char *) &ph))break;
    lseek(fd,-BLOCKSIZE*2,SEEK_CUR);
  }
}

int copy_tar_tar(int fd_source,char *tar_path_part_source,int fd_destination, char *tar_path_part_destination){
  lseek_befor_emptyblocks(fd_destination);
  struct posix_header ph;
  size_t read_size_tar;
  while ((read_size_tar = read(fd_source,&ph, BLOCKSIZE)) > 0) {
    int blocksize ;
    unsigned int size;
    sscanf((char *) ph.size, "%o", &size);
    blocksize = (size + BLOCKSIZE - 1) >> BLOCKBITS;
    if (start_by(ph.name,tar_path_part_source)){
      char *new = copy(tar_path_part_destination);
      char **tokens = malloc(sizeof(char *) * (path_size(ph.name) + 1));
      setup_path_tokens(tokens,ph.name,path_size(ph.name));
      char *tmp = concat_tokens_no_previous(tokens,tar_path_part_source,path_size(ph.name));
      new = concat(new,tmp);
      free(tmp);
      free_array(tokens,path_size(ph.name));
      if (ph.name[strlen(ph.name)-1]=='/') {
        new=concat(new,"/");
      }
      strcpy(ph.name,new);
      free(new);
      set_checksum(&ph);
      if (write(fd_destination,&ph,BLOCKSIZE) < 0) {
        perror("Write");
        return -1;
      }
      copy_fdcontent_to_fddestination(fd_source,blocksize,fd_destination,1);
    }else{
      lseek(fd_source,BLOCKSIZE*blocksize,SEEK_CUR);
    }
  }
  if (read_size_tar < 0) {
    perror("Read");
    return -1;
  }
  write_end_of_file_tar(fd_destination);
  return 1;
}

int dir_empty_in_tar(int fd,char *tar_path_part){
  struct posix_header ph;
  size_t read_size_tar;
  int nb_files = 0;
  while ((read_size_tar = read(fd,&ph, BLOCKSIZE)) > 0) {
    if (nb_files>1) {
      return 0;
    }
    int blocksize ;
    unsigned int size;
    sscanf((char *) ph.size, "%o", &size);
    blocksize = (size + BLOCKSIZE - 1) >> BLOCKBITS;
    if (start_by(ph.name,tar_path_part)){
      nb_files++;
    }
    lseek(fd,BLOCKSIZE*blocksize,SEEK_CUR);
  }
  if (read_size_tar < 0) {
    perror("Read");
    return -1;
  }
  return 1;
}

int create_dir_or_file_in_tar(char *dir_or_file,char *name, int fd_destination, int delete){
  struct posix_header *header = create_posix_header(name,dir_or_file);
  if(header==NULL){ //ERROR ON LSTAT
    return -1;
  }
  lseek_befor_emptyblocks(fd_destination);
  size_t write_size = write(fd_destination,header,BLOCKSIZE);
  if (write_size < 0) {
    perror("Write");
    free(header);
    return -1;
  }
  if (header->typeflag != '5') {
    char *read_buf = calloc(BLOCKSIZE,BLOCKSIZE);
    if (read_buf == NULL)   {
      perror("Calloc");
      exit(errno);
    }
    int fd_source = open(dir_or_file, O_RDONLY);
    size_t read_size;
    while ((read_size = read(fd_source,read_buf,BLOCKSIZE))>0) {
      size_t write_size = write(fd_destination,read_buf,BLOCKSIZE);
      if (write_size < 0) {
        perror("Write");
        free(header);
        free(read_buf);
        close(fd_source);
        return -1;
      }
      memset(read_buf,'\0',BLOCKSIZE);
    }
    free(read_buf);
    close(fd_source);
    if (read_size < 0) {
      perror("Read");
      free(header);
      return -1;
    }
  }
  free(header);
  write_end_of_file_tar(fd_destination);
  if (delete) {
    unlink(dir_or_file);
  }
  return 1;
}

int create_dir_content_in_tar(char *source,char *last_source,char *dir_or_file_on_tar ,int fd_destination, int delete){
  DIR * dir_source = opendir(source);
  if(dir_source == NULL){
      perror("OpendDir");
      return -1;
  }
  char *name_new_file = copy(dir_or_file_on_tar);
  char **tokens = malloc(sizeof(char *) * (path_size(source) + 1));
  setup_path_tokens(tokens,source,path_size(source));
  char *all_tokens=concat_tokens_no_previous(tokens,last_source,path_size(source));
  name_new_file = concat(name_new_file,all_tokens);
  free(all_tokens);
  free_array(tokens,path_size(source));
  name_new_file=concat(name_new_file,"/");
  if (create_dir_or_file_in_tar(source,name_new_file,fd_destination,delete)==-1) {
    return -1;
  }
  struct dirent *entry;
  while((entry = readdir(dir_source))!=NULL ){
    if(strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0){
      if ((entry->d_type & DT_DIR)) {
        char *newdirpath= copy(source);
        newdirpath= concat(newdirpath,entry->d_name);
        if (create_dir_content_in_tar(newdirpath, last_source, dir_or_file_on_tar,fd_destination,delete)==-1) {
          free(entry);
          free(name_new_file);
          free(newdirpath);
          closedir(dir_source);
          return -1;
        }
        free(newdirpath);

      }else{
        char *newfile = concat(copy(name_new_file),entry->d_name);
        char *tmp=concat(copy(source),entry->d_name);
        if (create_dir_or_file_in_tar(tmp,newfile,fd_destination,delete)==-1) {
          free(tmp);
          free(entry);
          free(name_new_file);
          free(newfile);
          closedir(dir_source);
          return -1;
        }
        free(tmp);
        free(newfile);
      }
    }
  }
  free(name_new_file);
  closedir(dir_source);
  if (delete) {
    rmdir(source);
  }
  return 1;
}

int rename_in_tar(int fd_source,char *dir_or_file_on_tar , char *newname, int all){
  struct posix_header ph;
  size_t read_size_tar;
  while ((read_size_tar = read(fd_source,&ph, BLOCKSIZE)) > 0) {
    int blocksize ;
    unsigned int size;
    sscanf((char *) ph.size, "%o", &size);
    blocksize = (size + BLOCKSIZE - 1) >> BLOCKBITS;
    if (start_by(ph.name,dir_or_file_on_tar)) {
      char **tokens = malloc(sizeof(char *) * (path_size(ph.name) + 1));
      int posChange= path_size(dir_or_file_on_tar)-1;
      setup_path_tokens(tokens,ph.name,path_size(ph.name));
      if (all) {
        for (size_t i = 0; i < posChange ; i++) {
          free(tokens[i]);
          tokens[i]=copy("");
        }
      }
      free(tokens[posChange]);
      tokens[posChange]=copy(newname);
      char *new_name=concat_tokens(tokens,path_size(ph.name));
      free_array(tokens,path_size(ph.name));
      if (ph.name[strlen(ph.name)-1]=='/') {
        new_name=concat(new_name,"/");
      }
      strcpy(ph.name,new_name);
      free(new_name);
      set_checksum(&ph);
      lseek(fd_source,-BLOCKSIZE,SEEK_CUR);
      if (write(fd_source,&ph, BLOCKSIZE) < 0) {
        perror("Write");
        return -1;
      }
    }
    lseek(fd_source,BLOCKSIZE*blocksize,SEEK_CUR);
  }
  if (read_size_tar < 0) {
    perror("Read");
    return -1;
  }
  return 1;
}

int open_tar_descriptor(char *pathname, int flags,  mode_t mode){
  int fd = (mode==-1)? open(pathname,flags):open(pathname,flags,mode);
  if (fd==-1) {
    perror("Open tar");
    return -1;
  }
  struct posix_header ph;
  size_t read_size_tar;
  while ((read_size_tar = read(fd,&ph, BLOCKSIZE)) > 0) {
    int blocksize ;
    unsigned int size;
    sscanf((char *) &ph.size, "%o", &size);
    blocksize = (size + BLOCKSIZE - 1) >> BLOCKBITS;
    if(path_size(ph.name)>1){
      char *copy_name=copy(ph.name);
      delete_last(copy_name);
      if (copy_name[strlen(copy_name)-1]!='/') {
        copy_name=concat(copy_name,"/");
      }
      if (!check_existence_in_tar(pathname,copy_name,1, 1)) {
        off_t current_seek = lseek(fd,0L,SEEK_CUR);
        lseek_befor_emptyblocks(fd);
        struct posix_header *newdir = create_dir_in_tar(copy_name);
        size_t write_size = write(fd,newdir,BLOCKSIZE);
        free(newdir);
        if (write_size < 0) {
          perror("Write");
          free(copy_name);
          return -1;
        }
        lseek(fd,current_seek,SEEK_SET);
      }
      free(copy_name);
    }
    lseek(fd,BLOCKSIZE*blocksize,SEEK_CUR);
  }
  if (read_size_tar < 0) {
    perror("Read");
    return -1;
  }
  lseek(fd,0L,SEEK_SET);
  return fd;
}
