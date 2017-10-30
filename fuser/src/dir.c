#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/stat.h>
#include "userfs.h"
#include "blocks.h"
#include "file.h"
#include "dir.h"
#include "inode.h"
#include <string.h>

void init_dir() {
	int i;
	root_dir.num_files = 0;
	for (i=0; i< MAX_FILES_PER_DIRECTORY; i++) {
    root_dir.u_file[i].inode_number=-1;
		root_dir.u_file[i].free = true;
	}
}

void read_dir(){
  read_block(DIRECTORY_BLOCK, &root_dir, sizeof(dir));
}


void write_dir() {
	write_block(DIRECTORY_BLOCK, &root_dir, sizeof(dir));
}


void allocate_file(int inode, const char *name){
  int i;
	if(is_dir_full()==false){
		for (i = 0; i < MAX_FILES_PER_DIRECTORY; i++) {
			if(root_dir.u_file[i].free==true){	
          root_dir.u_file[i].free = false;
          strcpy(root_dir.u_file[i].file_name, name);
          root_dir.u_file[i].inode_number = inode;
          root_dir.u_file[i].mode = S_IFREG | 0755;
          root_dir.u_file[i].uid = 1000;
          root_dir.u_file[i].gid = 1000;
          root_dir.num_files++;
			 	  break;
			}
		}
	}
	write_dir();
}

bool is_dir_full() {
	if(root_dir.num_files==MAX_FILES_PER_DIRECTORY){
		return true;
	}else{
		return false;
	}
}

bool find_file(const char * name, file **f){
  
	int i;
  read_dir();

	for(i=0;i<MAX_FILES_PER_DIRECTORY;i++){
		if(root_dir.u_file[i].free==true) continue;
	  if(!strcmp(root_dir.u_file[i].file_name,name)){
	  	*f=&root_dir.u_file[i];
			return true;
	  }
	}

	*f=NULL;
	return false;
}

void remove_file(file *f){
	int i;
	inode in;
	read_inode(f->inode_number,&in);
	
	for(i=0;i<MAX_BLOCKS_PER_FILE;i++){
		free_block(in.blocks[i]);
	}
	in.num_blocks=0;
	in.file_size_bytes=0;
	in.last_modified=time(NULL);
	in.free=true;
	write_inode(f->inode_number,&in);


	f->free=true;
	f->file_name[0]='\0';
	f->inode_number=-1;
	
	root_dir.num_files--;
	write_dir();
}

void rename_file(const char *old, const char *new){
  file *f;
	if(find_file(old, &f)==false) {
      printf("Error: unable to rename the file");
      return;
  }
	strcpy(f->file_name, new);
	write_dir();

}
