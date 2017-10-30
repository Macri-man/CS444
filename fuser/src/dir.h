#ifndef U_DIR
#define U_DIR

#define MAX_FILES_PER_DIRECTORY 100
#define DIRECTORY_BLOCK 2

#include "file.h"

typedef struct dir_struct_s{
	int num_files;
	file u_file[MAX_FILES_PER_DIRECTORY];
} dir;

void init_dir();
void allocate_file(int, const char *);
void write_dir();
void read_dir();
bool is_dir_full();
bool find_file(const char *, file **);
void remove_file(file *);
void rename_file(const char *, const char *);

dir root_dir;

#endif
