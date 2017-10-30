/*
  gcc -Wall fs.c `pkg-config fuse --cflags --libs` -o fusefs
*/

/*
http://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/unclear.html
http://fuse.sourceforge.net/doxygen/structfuse__operations.html

man errno.h
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include "src/userfs.h"
#include "src/dir.h"
#include "src/inode.h"
#include "src/blocks.h"
#include "src/sb.h"
#include "src/bitmap.h"
#include "fs.h"

/* Sets stbuf's properties based on file path
   man 3 stat
   man stat.h
   
   st_mode      Filemode 0666 rw-rw-rw
   st_nlink     Number of hard links to the file (1)
   st_mtime     Modified Time
   st_ctime     Created Time
   st_size      File size in bytes
*/

static int fs_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	file *f;
	inode in;
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		stbuf->st_mtime = time(NULL);
		stbuf->st_ctime = time(NULL);
	}else if(find_file(path,&f)){
		read_inode(f->inode_number,&in);
		stbuf->st_mode = f->mode;
		stbuf->st_nlink = 1;
		stbuf->st_mtime = in.last_modified;
		stbuf->st_ctime =time(NULL);
		stbuf->st_size = in.file_size_bytes;
		stbuf->st_blocks = in.num_blocks;
		stbuf->st_blksize = BLOCK_SIZE_BYTES;
		stbuf->st_ino = f->inode_number;
		stbuf->st_uid = f->uid;
		stbuf->st_gid = f->gid;
	}else{
		res = -ENOENT;
	}
	return res;
}

/* Reads all of the files in path into buf using the filler function
   int (*fuse_fill_dir_t)(void *buffer, char* filename, NULL, int offset 0);
   	filler will add a file to the result buffer (buf)
   
*/
static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
	(void) offset;
	(void) fi;

	if(strcmp(path, "/")!=0) return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	int i;
	for(i=0;i<MAX_FILES_PER_DIRECTORY;i++){
		if (root_dir.u_file[i].free) continue;
		filler(buf,root_dir.u_file[i].file_name+1,NULL,0);
	}
	return 0;
}

/* Create a empty file named path 
   
   Find a free inode
   Mark it as allocated
   Allocate file as our inode in root dir
   Writes relevent blocks
*/
static int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi){
	inode in;
	file *f;
	
	if(strlen(path) > MAX_FILE_NAME_SIZE) {
		return -ENAMETOOLONG;
	}

	if(is_dir_full()==true) {
		return -1;
	}

	if(find_file(path, &f)==true) {
		return -EEXIST;
	}
	
	int inode_number=find_free_inode();
	if(inode_number==-1){
		fprintf(stderr, "INVALID INODE\n");
		return -1;
	}

	
	allocate_inode(&in, 0, 0); 
	write_inode(inode_number, &in);	
	allocate_file(inode_number, path);

	return 0;
}

/* Checks that a file can be opened */
static int fs_open(const char *path, struct fuse_file_info *fi){
	file *file;

	if(find_file(path, &file)==true){
		return 0;
	}
	return -1;
}

/* Reads the contents of file into buf
   man 3 read
   
   finds the file for path
   reads contents of file into buf starting at offset  (recommended: implement offset after you have the rest working)
   returns the number of bytes read
*/
static int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	size_t len;
	(void) fi;
	
	int block;
	int read_bytes;
	int current_bytes;
	inode in;
	
	file *file;
	//FUSE Should have called open to check that the file exists ahead of time
	assert(find_file(path, &file));
	read_bytes = 0;
	read_inode(file->inode_number,&in);


	current_bytes = offset % BLOCK_SIZE_BYTES; 
	block=offset/BLOCK_SIZE_BYTES;

	if(current_bytes>0){
		if(BLOCK_SIZE_BYTES>size-read_bytes){
			read_block_offset(in.blocks[block], buf + read_bytes , BLOCK_SIZE_BYTES, current_bytes);
			read_bytes +=BLOCK_SIZE_BYTES;
			if (read_bytes == size) return 0;
			block++;
		}else{
			read_block_offset(in.blocks[block], buf + read_bytes , size-read_bytes, current_bytes);
			read_bytes +=size-read_bytes;
			if (read_bytes == size) return 0;
			block++;
		}
	}
	for(block;block<in.num_blocks;block++){
		if(BLOCK_SIZE_BYTES>size-read_bytes){
			read_block(in.blocks[block], buf + read_bytes, BLOCK_SIZE_BYTES);
			read_bytes +=BLOCK_SIZE_BYTES;
			if (read_bytes == size) break;
		}else{
			read_block(in.blocks[block], buf + read_bytes, size-read_bytes);
			read_bytes +=size-read_bytes;
			if (read_bytes == size) break;
		}
	}
	return read_bytes;
}

/* Writes contents of buf to file
   man 3 write
   
   find file for path
   writes contents of buf into file starting at offset  (recommended: implement offset after you have the rest working)
      figure out how many blocks you need
      for each required block
      	find a free block
      	update inode to have new block
		write buf to blocks
	write relevent blocks
*/
static int fs_write(const char * path, const char * buf, size_t buff_size, off_t offset, struct fuse_file_info * fi) {
	inode in;
	int block_size;
	int i;

	DISK_LBA current_block;
	int buf_rem;
	int block;
	file *file;

	int new_size=buff_size + offset;
	int new_blocknum=floor(new_size/BLOCK_SIZE_BYTES) + 1;

	assert(find_file(path, &file));
	read_inode(file->inode_number, &in);
	
	if(in.file_size_bytes < new_size) {

		if (new_blocknum - in.num_blocks > u_quota()) {
			return -ENOSPC;
		}

		if (!valid_file_size(new_blocknum)) {
			return -EFBIG;
		}

		for (i = in.num_blocks; i < new_blocknum; i++) {
			in.blocks[i] = find_free_block();
			allocate_block(in.blocks[i]);
		}

		in.file_size_bytes = new_size;
		in.num_blocks = new_blocknum;
		write_inode(file->inode_number, &in);
	}
	buf_rem = buff_size;
	block = offset / BLOCK_SIZE_BYTES;
	block_size=buff_size-buf_rem;
	if(offset>0){
		if(buf_rem>BLOCK_SIZE_BYTES){
			assert(block < in.num_blocks);
			write_block_offset(in.blocks[block], buf + block_size, buf_rem, offset);
			block++;
			buf_rem -= BLOCK_SIZE_BYTES;
		}else{
			assert(block < in.num_blocks);
			write_block_offset(in.blocks[block], buf + block_size, BLOCK_SIZE_BYTES, offset);
			block++;
			buf_rem -= BLOCK_SIZE_BYTES;
		}
	}
	for(buf_rem;buf_rem > 0;buf_rem -= BLOCK_SIZE_BYTES,block_size=buff_size-buf_rem,block++){
		if(buf_rem>BLOCK_SIZE_BYTES){
			assert(block < in.num_blocks);
			write_block(in.blocks[block], buf + block_size, buf_rem);
		}else{
			assert(block < in.num_blocks);
			write_block(in.blocks[block], buf + block_size, buf_rem);
		}
	}
	return buff_size;
}

/* Trims file to offset length
   figure out which blocks to free
   free relevent blocks
   update inode
*/
static int fs_truncate(const char *path, off_t offset) {
	inode in;
	int i;
	int fh;
	int offsetblock;
	DISK_LBA current_block;
	file *file;
	assert(find_file(path, &file));
	read_inode(file->inode_number,&in);

	in.file_size_bytes = offset;

	offsetblock=offset/BLOCK_SIZE_BYTES;
	for(i=offsetblock;i<in.num_blocks;i++){
		free_block(in.blocks[i]);
	}
	in.num_blocks=offsetblock;
	write_inode(file->inode_number,&in);

	return 0;
}

/* Remove file 
   Save relevent blocks
*/
int fs_unlink(const char * path) {
	file *file;
	if (find_file(path, &file)==true){
		remove_file(file);
		return 0;
	}
	return -ENOENT;
}

//Extra credit
static int fs_chown(const char * path, uid_t uid, gid_t gid) {
	file *f;	
	if(find_file(path,&f)==false){
		return -ENOENT;
	}
	
	f->uid=uid;
	f->gid=gid;

	write_dir();

	return 0;
}

//Extra credit
static int fs_chmod(const char * path, mode_t mode) {

	file *f;
	if(find_file(path,&f)==false){
		return -ENOENT;
	}

	f->mode=mode;

	write_dir();

	return 0;
}

//Extra credit
static int fs_utimens(const char * path, const struct timespec tv[2] ) {
	return 0;
}

static int fs_rename(const char * oldpath, const char * newpath) {
	file *file;
	
	if(strlen(newpath) > MAX_FILE_NAME_SIZE) {
		return -ENAMETOOLONG;
	}
	
	if (find_file(oldpath, &file)==true){
		rename_file(file->file_name, newpath);
		return 0;
	}
	return -ENOENT;
}

//Creates a structure to tell fuse about the operations we have implemented
static struct fuse_operations fs_oper = {
	.getattr	= fs_getattr,
	.readdir	= fs_readdir,
	.open		= fs_open,
	.read		= fs_read,
	.create	= fs_create,
	.chown	= fs_chown,
	.chmod	= fs_chmod,
	.utimens	= fs_utimens,
	.truncate	= fs_truncate,
	.write	= fs_write,
	.unlink	= fs_unlink,
	.rename	= fs_rename,
};

int u_quota() {
	int freeCount=0;
	int i;
	
	assert(BIT_MAP_SIZE > sb.disk_size_blocks);
	
	for (i=0; i < sb.disk_size_blocks; i++ )
	{
		if (bit_map[i]==0)
		{
			freeCount++;
		}
	}
	return freeCount;
}

int main(int argc, char **argv)
{
	int ret;
	char ** fuse_argv;
	int fuse_argc = 0;
	
	bool do_format = false;
	int size_format = 0;
	
	int argi;
	char * arg;
	char * disk = NULL;
	
	bool disable_crash = false;
	
	//Copy prog name
	fuse_argv = malloc(sizeof(char *) * argc);
	fuse_argv[0] = argv[0];
	fuse_argc++;
	
	for (argi = 1; argi < argc; argi++) {
		arg = argv[argi];
		
		if (strcmp(arg, "--help") == 0) {
			printf("Usage:\n");
			printf("\t--disk [diskfile]\n");
			printf("\t--format [size]\n");
			printf("\t--no-crash\n");
			printf("\t--help\n");
			return 0;
		} else if (strcmp(arg, "--disk") == 0) {
			argi++;
			disk = argv[argi];
		} else if (strcmp(arg, "--format") == 0) {
			do_format = true;
			argi++;
			size_format = atoi(argv[argi]);
		} else if (strcmp(arg, "--no-crash") == 0) {
			disable_crash = true;
		} else {
			fuse_argv[fuse_argc] = arg;
			fuse_argc++;
		}
	}
	
	if (disk == NULL) {
		fprintf(stderr, "Must specify disk\n");
		return -1;
	}
	
	if (do_format) {
		fprintf(stderr, "Formatting %s (size %i)\n", disk, size_format);
		u_format(size_format, disk);
		return 0;
	}
	if(!u_fsck()){
		recover_file_system(disk);
		sb.clean_shutdown = 0;
	}

	if(!disable_crash) {
		init_crasher();
	}
	
	ret = fuse_main(fuse_argc, fuse_argv, &fs_oper, NULL);
	u_clean_shutdown();
	
	free(fuse_argv);
	return ret;
}
