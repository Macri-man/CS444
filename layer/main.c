#include "ss.h"

// This can be used to debug your program
void print(StorageSystem *ss);

int main(int argc, char** argv) {

	StorageSystem* ss = (StorageSystem*)malloc(sizeof(StorageSystem));
	init_ss(ss);

	write(ss, 1, 5);
	write(ss, 2, 3);
	write(ss, 3, 6);
	write(ss, 4, 8);
	write(ss, 5, 9);
	write(ss, 6, 10);
	write(ss, 7, 2);
	write(ss, 8, 3);
	write(ss, 9, 1);
	write(ss, 3, 17);
	write(ss, 5, 81);
	write(ss, 7, 43);

	int cpudata[] = {17, 81, 43};
	int memorydata[] = {81, 43, 3, 1, 17};
	int diskdata[] = {5, 3, 17, 8, 81, 10, 43, 3, 1};
	
	for(int i = 0 ; i < 3; i++) {
		assert(cpudata[i] == ss->topLayer->local[i]);
	}
	
	for(int i = 0 ; i < 5; i++) {
		assert(memorydata[i] == ss->topLayer->below->local[i]);
	}

	for(int i = 0 ; i < 9; i++) {
		assert(diskdata[i] == ss->topLayer->below->below->local[i]);
	}

	int value[] = {5, 3, 17, 8, 81, 10, 43, 3, 1};
	for(int i = 0 ; i < 9 ; i++) 
		assert(read(ss, i+1) == value[i]);

	return 0;
}

void print(StorageSystem* ss) {
	printf("%s\n","Data of CPU cache");

	for(int i = 0 ; i < 3; i++) {
		printf("%d\n", ss->topLayer->local[i]);	
	}
	printf("%s\n","Data of Memory");

	for(int i = 0 ; i < 5; i++) {
		printf("%d\n", ss->topLayer->below->local[i]);	
	}

	printf("%s\n","Data of Disk");

	for(int i = 0 ; i < 20; i++) {
		printf("%d\n", ss->topLayer->below->below->local[i]);	
	}
}
