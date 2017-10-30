#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {
	if(argc != 2) {
		fprintf(stderr,"Usage: gen <number of lines>\n");
		exit(0);
	}
	int number = atoi(argv[1]);
	srand ( time(NULL) );
	char* buffer;
	/* Generate a random number: */
	for(int i = 0 ; i < number ;i++) {
		int length = rand() % 10 + 5;
		buffer = (char*)malloc(sizeof(char)*(length+1));	
		for(int j = 0 ; j < length;j++) {
			buffer[j] = 'a'+(rand() % 26);
		}
		buffer[length] = 0;
		printf("%s\n",buffer);
		free(buffer);
	}
}