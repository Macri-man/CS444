#ifndef U_FILE
#define U_FILE
#include <time.h>
#include <sys/types.h>
#define MAX_FILE_NAME_SIZE 15

typedef struct file_struct{
	int inode_number;
	//time_t created;
	mode_t mode;
	gid_t uid;
	uid_t gid;
	char file_name[MAX_FILE_NAME_SIZE+1];
	bool free;
} file;

bool valid_file_size(int);

#endif
