#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "sb.h"
#include "inode.h"
#include "userfs.h"
#include "blocks.h"
#include "bitmap.h"
#include "dir.h"


/*
 * Formats the virtual disk. Saves the superblock
 * bit map and the single level directory.
 */
int u_format(int diskSizeBytes, char* file_name)
{
	int i;
	int minimumBlocks;
	inode curr_inode;

	/* create the virtual disk */
	if ((virtual_disk = open(file_name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR)) < 0)
	{
		fprintf(stderr, "Unable to create virtual disk file: %s\n", file_name);
		return 0;
	}


	fprintf(stderr, "Formatting userfs of size %d bytes with %d block size in file %s\n",diskSizeBytes, BLOCK_SIZE_BYTES, file_name);

	minimumBlocks = 3+ NUM_INODE_BLOCKS+1;
	if (diskSizeBytes/BLOCK_SIZE_BYTES < minimumBlocks){
		fprintf(stderr, "Minimum size virtual disk is %d bytes %d blocks\n",BLOCK_SIZE_BYTES*minimumBlocks, minimumBlocks);
		fprintf(stderr, "Requested virtual disk size %d bytes results in %d bytes %d blocks of usable space\n",diskSizeBytes, BLOCK_SIZE_BYTES*minimumBlocks, minimumBlocks);
		return 0;
	}


	/*************************  BIT MAP **************************/

	assert(sizeof(BIT_FIELD)* BIT_MAP_SIZE <= BLOCK_SIZE_BYTES);
	fprintf(stderr, "%d blocks %d bytes reserved for bitmap (%d bytes required)\n", 1, BLOCK_SIZE_BYTES, sizeof(BIT_FIELD)* BIT_MAP_SIZE );
	fprintf(stderr, "\tImplies Max size of disk is %d blocks or %d bytes\n",BIT_MAP_SIZE*BITS_PER_FIELD, BIT_MAP_SIZE*BLOCK_SIZE_BYTES*BITS_PER_FIELD);
  
	if (diskSizeBytes >= BIT_MAP_SIZE* BLOCK_SIZE_BYTES){
		fprintf(stderr, "Unable to format a userfs of size %d bytes\n",diskSizeBytes);
		return 0;
	}

	init_bit_map();
  
	/* first three blocks will be taken with the 
	   superblock, bitmap and directory */
	allocate_block(BIT_MAP_BLOCK);
	allocate_block(SUPERBLOCK_BLOCK);
	allocate_block(DIRECTORY_BLOCK);
	/* next NUM_INODE_BLOCKS will contain inodes */
	for (i=3; i< 3+NUM_INODE_BLOCKS; i++){
		allocate_block(i);
	}
  
	write_bitmap();
	
	/***********************  DIRECTORY  ***********************/
	

	fprintf(stderr, "%d blocks %d bytes reserved for the userfs directory (%d bytes required)\n", 1, BLOCK_SIZE_BYTES, sizeof(dir));
	fprintf(stderr, "\tMax files per directory: %d\n",MAX_FILES_PER_DIRECTORY);
	fprintf(stderr,"Directory entries limit filesize to %d characters\n",MAX_FILE_NAME_SIZE);

	assert(sizeof(dir) <= BLOCK_SIZE_BYTES);

	init_dir();
	write_dir();
	read_dir();

	/***********************  INODES ***********************/
	fprintf(stderr, "userfs will contain %d inodes (directory limited to %d)\n",MAX_INODES, MAX_FILES_PER_DIRECTORY);
	fprintf(stderr,"Inodes limit filesize to %d blocks or %d bytes\n",MAX_BLOCKS_PER_FILE, MAX_BLOCKS_PER_FILE* BLOCK_SIZE_BYTES);

	curr_inode.free = 1;
	for (i=0; i< MAX_INODES; i++){
		write_inode(i, &curr_inode);
	}

	/***********************  SUPERBLOCK ***********************/
	assert(sizeof(superblock) <= BLOCK_SIZE_BYTES);
	fprintf(stderr, "%d blocks %d bytes reserved for superblock (%d bytes required)\n", 
		1, BLOCK_SIZE_BYTES, sizeof(superblock));
	init_superblock(diskSizeBytes);
	fprintf(stderr, "userfs will contain %d total blocks: %d free for data\n",
		sb.disk_size_blocks, sb.num_free_blocks);
	fprintf(stderr, "userfs contains %lu free inodes\n", MAX_INODES);
	
	write_block(SUPERBLOCK_BLOCK, &sb, sizeof(superblock));
	sync();


	/* when format complete there better be at 
	   least one free data block */
	assert( u_quota() >= 1);
	fprintf(stderr,"Format complete!\n");
	
	close(virtual_disk);
	return 1;
}

int u_fsck(){/*
	read_dir();
	int i,j,k;
	inode in;
fprintf(stderr,"start fcshk for!\n");
	for(i=0;i<root_dir.num_files;i++){
		fprintf(stderr,"first for!\n");
		if(root_dir.u_file[i].free==true&&root_dir.u_file[i].inode_number!=-1){
			read_inode(root_dir.u_file[i].inode_number,&in);
			for(j=in.num_blocks;j<MAX_BLOCKS_PER_FILE;j++){
				free_block(j);
			}
			in.free=true;
			in.file_size_bytes=0;
			in.last_modified=time(NULL);
			write_inode(i,&in);
		}
	}
	for(i=root_dir.num_files;i<MAX_FILES_PER_DIRECTORY;i++){
fprintf(stderr,"second for!\n");
		if(root_dir.u_file[i].inode_number!=-1){
			read_inode(root_dir.u_file[i].inode_number,&in);
			for(j=in.num_blocks;j<MAX_BLOCKS_PER_FILE;j++){
				free_block(j);
			}
			in.free=true;
			in.file_size_bytes=0;
			in.last_modified=time(NULL);
			write_inode(i,&in);
		}
		root_dir.u_file[i].free=true;
		root_dir.u_file[i].file_name[0]='\0';
		root_dir.u_file[i].inode_number=-1;
	}
	for(i=0;i<MAX_INODES;i++){
fprintf(stderr,"third for!\n");
		read_inode(i,&in);
		if(in.free==true){
			continue;
		}else{
			for(j=0;j<root_dir.num_files;j++){
				if(i==root_dir.u_file[j].inode_number&&root_dir.u_file[j].free==true){
					break;
				}else{
					for(k=0;k<MAX_BLOCKS_PER_FILE;k++){
						free_block(in.blocks[k]);
					}
				}		
			}
		}
		write_inode(i,&in);	
	}
	write_dir();*/
	return 0;
}

/*
 * Attempts to recover a file system given the virtual disk name
 */
int recover_file_system(char *file_name){

	if ((virtual_disk = open(file_name, O_RDWR)) < 0)
	{
		printf("virtual disk open error\n");
		return 0;
	}

	read_block(SUPERBLOCK_BLOCK, &sb, sizeof(superblock));
	read_block(BIT_MAP_BLOCK, bit_map, sizeof(BIT_FIELD)*BIT_MAP_SIZE);
	read_block(DIRECTORY_BLOCK, &root_dir, sizeof(dir));

	if (!superblockMatchesCode()){
		fprintf(stderr,"Unable to recover: userfs appears to have been formatted with another code version\n");
		return 0;
	}
	if (!sb.clean_shutdown)
	{
		/* Try to recover your file system */
		fprintf(stderr, "u_fsck in progress......");
		if (u_fsck()){
			fprintf(stderr, "Recovery complete\n");
			return 1;
		}else {
			fprintf(stderr, "Recovery failed\n");
			return 0;
		}
	}
	else{
		fprintf(stderr, "Clean shutdown detected\n");
		return 1;
	}
}

int u_clean_shutdown()
{
	/* write code for cleanly shutting down the file system
	   return 1 for success, 0 for failure */
  
	sb.num_free_blocks = u_quota();
	sb.clean_shutdown = 1;

	lseek(virtual_disk, BLOCK_SIZE_BYTES* SUPERBLOCK_BLOCK, SEEK_SET);
	crash_write(virtual_disk, &sb, sizeof(superblock));
	sync();

	close(virtual_disk);
	/* is this all that needs to be done on clean shutdown? */
	return !sb.clean_shutdown;
}
