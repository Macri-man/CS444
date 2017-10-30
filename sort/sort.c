#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <time.h>

void sort_file(char *filename);

pthread_mutex_t mutex;

typedef struct list_s {
	struct list_s *next;
	char *filename;
}list_t;

list_t *ls=NULL;

list_t *make_list(char *dirname,char *filename){
	assert(dirname != NULL  && filename != NULL);
	list_t *l=malloc(sizeof(list_t));
	l->filename=malloc(((strlen(dirname)+1)+(strlen(filename)+1))*sizeof(char));
	strcpy(l->filename,strdup(dirname));
	strcat(l->filename,strdup(filename));
	l->next=NULL;
	return l;
}

list_t *list_append(list_t *curr,char *dirname,char *filename){
	list_t *l=make_list(dirname,filename);
	l->next=curr;
	return l;
}
list_t *list_pop(list_t *head){
	list_t *temp;
	temp = head;
	if(temp){
		temp = temp->next;
	}
	return temp;
}

void print(list_t *head){
	assert(head!=NULL);
	list_t *temp=NULL;
	for(temp=head;temp!=NULL;temp=temp->next){
		fprintf(stderr, "%s\n",temp->filename);
	}
}
 
int main(int argc,char *argv[]){

	if(argc != 3) {
		fprintf(stderr,"Usage: a.out <directory where files are not sorted> <number of threads>\n");
		exit(0);
	}

	if(pthread_mutex_init(&mutex,NULL)){
		fprintf(stderr, "MUTEX IS INVALID\n");
	}

	int numpthread=atoi(argv[2]);
	pthread_t threads[numpthread];

	fprintf(stderr,"Using %d threads...\n",numpthread);
	fprintf(stderr,"Directory of files %s\n",argv[1]);

	struct dirent *dirent;
	DIR *dir;

	dir = opendir(argv[1]);
	if (dir == NULL){
		fprintf(stderr, "Failed to open directory '%s'\n",argv[1]);
		return -errno;
	}
	int numfiles=0;
	while ((dirent = readdir(dir)) != NULL) {
		if(strcmp(dirent->d_name,"..") && strcmp(dirent->d_name, ".")){
			numfiles++;
			if(ls==NULL){
				ls=make_list(argv[1],dirent->d_name);
			}else{
				ls=list_append(ls,argv[1],dirent->d_name);
			}
		}
	}
	fprintf(stderr, "Number of files in directory: %d\n",numfiles);

	closedir(dir);
	assert(ls!=NULL);

	//print(ls);

	for (int i = 0; i < numpthread; i++) {
		if(pthread_create(&(threads[i]), NULL, (void*(*)(void*))sort_file,NULL) != 0) {
			perror("thread creation failed \n");
			exit(-1);
		}
	}

	for (int i = 0; i < numpthread; i++) {
		pthread_join(threads[i],NULL);

	}
	 
	return 0;
}

void sort_file(){
	while(1){
		pthread_mutex_lock(&mutex);
		if(!ls){
			pthread_mutex_unlock(&mutex);
			break; 
		}
		char *filename=strdup(ls->filename);
		ls=list_pop(ls);
		pthread_mutex_unlock(&mutex);
		FILE *f;
		char ** lines;
		char line[50];
		char newfilename[50];
		char *data;
		int size=0;
		int i,j;
		int linenum=0;
		int numlines=0;
		assert(filename!=NULL);
		f = fopen(filename, "r");
   		if(f==NULL){
   			fprintf(stderr,"ERROR FILE DOES NOT EXIST %s \n",filename);
   			exit(1);
   		}
   		assert(f!=NULL);

		while(fgets(line, 50, f)!= NULL){
			numlines++;
		}
		lines = calloc(numlines,sizeof(char *));
		assert(lines!=NULL);

		for(i=0;i<numlines;i++){
			lines[i] = calloc(50,sizeof(char));
		}	
		assert(lines!=NULL);
		rewind(f);
		for(i=0;fgets(line, 50, f)!= NULL;i++){
			strcpy(lines[i],line);
		}
		fclose(f);

		for(i=0; i < numlines-1; i++){
			for(j=i+1; j < numlines; j++){
				if(strcmp(lines[i],lines[j]) > 0){
					strcpy(line,lines[i]);
					strcpy(lines[i],lines[j]);
					strcpy(lines[j],line);
				}
			}
		}

		sprintf(newfilename, "%s.sorted",filename);
		f = fopen(newfilename,"w");
		assert(f != NULL);
		for(i=0;i<numlines;i++){
			fputs(lines[i], f);
		}
		fclose(f);

		for(i=0;i<numlines;i++){
			free(lines[i]);
		}
		free(lines);
	}
		return;
}