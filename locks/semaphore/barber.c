#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <semaphore.h>
  
#define N 10

time_t end_time;
pthread_mutex_t mutex;
sem_t customers,barbers;
int costomer=0;
void barber(void *args);
void customer(void *args);
 
int main(int argc,char *argv[]){
	pthread_t thbarber1,thbarber2,thbarber3,thcustomer;
	end_time=time(NULL)+20;
	
	srand(time(NULL));
 
	sem_init(&customers,0,0);
	sem_init(&barbers,0,3);

	if(pthread_create(&thbarber1,NULL,(void *)barber,1)!=0)
		perror("create barbers is failure!\n");
	if(pthread_create(&thbarber2,NULL,(void *)barber,2)!=0)
		perror("create barbers is failure!\n");
	if(pthread_create(&thbarber3,NULL,(void *)barber,3)!=0)
		perror("create barbers is failure!\n");
	if(pthread_create(&thcustomer,NULL,(void *)customer,NULL)!=0)
		perror("create customers is failure!\n");
 
	pthread_join(thcustomer,NULL);
	pthread_join(thbarber1,NULL);
	pthread_join(thbarber2,NULL);
	pthread_join(thbarber3,NULL);
 
	return 0;
}
 
void barber(void *args){
	int arg=(int)args;
	while(time(NULL)<end_time){
		pthread_mutex_lock(&mutex);
		int barb,cust;
		sem_getvalue(&barbers,&barb);
		fprintf(stderr, "Barber:%d of total %d available\n",arg,barb);
		sem_getvalue(&customers,&cust);
		fprintf(stderr, "Customers waiting: %d\n", cust);
		if(barb>0){
			if(barb<3 && cust>0){
			sem_wait(&customers); 
			sem_getvalue(&customers,&cust);
			fprintf(stderr,"Barber:%d cut hair,customers left: %d.\n",arg,cust);
			sleep(2);
			sem_post(&barbers);
			}
		}else{
			fprintf(stderr,"No Barbers Free\n");
		}
		pthread_mutex_unlock(&mutex);
		sleep(2);
	}
}
 
void customer(void *args) {
	while(time(NULL)<end_time){
		if(rand()%2==0){
			
			int barb,cust;
			sem_getvalue(&customers,&cust);
			fprintf(stderr, "Customers arriving\n");
			if(cust<N){
				pthread_mutex_lock(&mutex);
				sem_post(&customers);
				sem_getvalue(&customers,&cust);
				fprintf(stderr,"Customers waiting: %d\n",cust);
				sem_getvalue(&barbers,&barb);
				if(barb>0){
				sem_wait(&barbers);
				fprintf(stderr,"Barber started cutting hair\n");
				}
				pthread_mutex_unlock(&mutex);
			}else{
				fprintf(stderr, "Customer is leaving\n");
			}
		}else{
			fprintf(stderr,"No Customers arrived\n");
		}
		sleep(1);
	}
}
