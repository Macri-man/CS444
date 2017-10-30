#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#if defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#elif defined __WIN32
#include <windows.h>
#endif


#define MAXGRIDSIZE 	10
#define MAXTHREADS	1000
#define NO_SWAPS	20

extern int errno;

pthread_mutex_t grid_mutex;
pthread_mutex_t thread_count_mutex;
pthread_mutex_t row_mutex[MAXGRIDSIZE];
pthread_mutex_t cell_mutex[MAXGRIDSIZE][MAXGRIDSIZE];

typedef enum {GRID, ROW, CELL, NONE} grain_type;
int gridsize = 0;
int grid[MAXGRIDSIZE][MAXGRIDSIZE];
int threads_left = 0;

time_t start_t, end_t;

int PrintGrid(int grid[MAXGRIDSIZE][MAXGRIDSIZE], int gridsize)
{
	int i;
	int j;
	
	for (i = 0; i < gridsize; i++)
	{
		for (j = 0; j < gridsize; j++)
			fprintf(stderr, "%d\t", grid[i][j]);
		fprintf(stderr, "\n");
	}
	return 0;
}


long InitGrid(int grid[MAXGRIDSIZE][MAXGRIDSIZE], int gridsize)
{
	int i;
	int j;
	long sum = 0;
	int temp = 0;

	srand( (unsigned int)time( NULL ) );


	for (i = 0; i < gridsize; i++)
		for (j = 0; j < gridsize; j++) {
			temp = rand() % 100;			
			grid[i][j] = temp;
			sum = sum + temp;
		}

	return sum;

}

long SumGrid(int grid[MAXGRIDSIZE][MAXGRIDSIZE], int gridsize)
{
	int i;
	int j;
	long sum = 0;


	for (i = 0; i < gridsize; i++){
		for (j = 0; j < gridsize; j++) {
			sum = sum + grid[i][j];
		}
	}
	return sum;

}
void *do_swaps(void *args)
{

	int i, row1, column1, row2, column2;
	int temp;
	grain_type* gran_type = (grain_type*)args;

	pthread_mutex_lock(&thread_count_mutex);
	threads_left++;
	pthread_mutex_unlock(&thread_count_mutex);



	for(i=0; i<NO_SWAPS; i++){
		row1 = rand() % gridsize;
		column1 = rand() % gridsize;	
		row2 = rand() % gridsize;
		column2 = rand() % gridsize;
		if (*gran_type == ROW){
		  /* obtain row level locks*/
			if(row1!=row2){
				if(row1<row2){
					pthread_mutex_lock(&row_mutex[row1]);
					pthread_mutex_lock(&row_mutex[row2]);
				}else{
					pthread_mutex_lock(&row_mutex[row2]);
					pthread_mutex_lock(&row_mutex[row1]);
				}
			}else{
				pthread_mutex_lock(&row_mutex[row1]);
			}
		}else if (*gran_type == CELL){
		  /* obtain cell level locks */
			if(row1<row2){
				pthread_mutex_lock(&cell_mutex[row1][column1]);
				pthread_mutex_lock(&cell_mutex[row2][column2]);

			}else if(row1>row2){
				pthread_mutex_lock(&cell_mutex[row2][column2]);
				pthread_mutex_lock(&cell_mutex[row1][column1]);

			}else if(row1==row2 && column1!=column2){ 
				if(column1<column2){
					pthread_mutex_lock(&cell_mutex[row1][column1]);
					pthread_mutex_lock(&cell_mutex[row2][column2]);
				}else{
					pthread_mutex_lock(&cell_mutex[row2][column2]);
					pthread_mutex_lock(&cell_mutex[row1][column1]);
				}

			}else{
				pthread_mutex_lock(&cell_mutex[row1][column1]);

			}
		}else if (*gran_type == GRID){
			
  		/* obtain grid level lock*/
			if(pthread_mutex_lock(&grid_mutex)==EINVAL){
				fprintf(stderr, "%s\n","");
			}

		}

		temp = grid[row1][column1];
#if defined(__APPLE__) || defined(__linux__)		
		sleep(1);
#elif defined __WIN32
		Sleep(1000);
#endif

		grid[row1][column1]=grid[row2][column2];
		grid[row2][column2]=temp;


		if (*gran_type == ROW){
		  /* release row level locks */
			pthread_mutex_unlock(&row_mutex[row1]);
			if (row1 != row2) {
				pthread_mutex_unlock(&row_mutex[row2]);
			}

				
		}else if (*gran_type == CELL){
			pthread_mutex_unlock(&cell_mutex[row1][column1]);
			if (row1!=row2 || column1!=column2) {
				pthread_mutex_unlock(&cell_mutex[row2][column2]);
			}

		}else if (*gran_type == GRID){
			
		  /* release grid level lock */
			pthread_mutex_unlock(&grid_mutex);		


		}

	}

	pthread_mutex_lock(&thread_count_mutex);
	threads_left--;
	if (threads_left == 0) { /* if this is last thread to finish*/
		time(&end_t);         /* record the end time*/
	}
	pthread_mutex_unlock(&thread_count_mutex);

	return NULL;
}	




int main(int argc, char **argv){


	int nthreads = 0;
	pthread_t threads[MAXTHREADS];


	grain_type rowGranularity = NONE;
	long initSum = 0, finalSum = 0;
	int i,j;

	
	if (argc > 3)
	{
		gridsize = atoi(argv[1]);					
		if (gridsize > MAXGRIDSIZE || gridsize < 1)
		{
			printf("Grid size must be between 1 and 10.\n");
			return(1);
		}
		nthreads = atoi(argv[2]);
		if (nthreads < 1 || nthreads > MAXTHREADS)
		{
			printf("Number of threads must be between 1 and 1000.");
			return(1);
		}

		if (argv[3][1] == 'r' || argv[3][1] == 'R')
			rowGranularity = ROW;
		if (argv[3][1] == 'c' || argv[3][1] == 'C')
			rowGranularity = CELL;
		if (argv[3][1] == 'g' || argv[3][1] == 'G')
		  rowGranularity = GRID;
			
	}
	else
	{
		printf("Format:  gridapp gridSize numThreads -cell\n");
		printf("         gridapp gridSize numThreads -row\n");
		printf("         gridapp gridSize numThreads -grid\n");
		printf("         gridapp gridSize numThreads -none\n");
		return(1);
	}

	pthread_mutex_init(&grid_mutex, NULL);
	pthread_mutex_init(&thread_count_mutex, NULL);
	for (i = 0; i < MAXGRIDSIZE; ++i) {
		pthread_mutex_init(&row_mutex[i], NULL);
		for (j = 0; j < MAXGRIDSIZE; ++j) {
			pthread_mutex_init(&cell_mutex[i][j], NULL);
		}
	}

	printf("Initial Grid:\n\n");
	initSum =  InitGrid(grid, gridsize);
	PrintGrid(grid, gridsize);
	printf("\nInitial Sum:  %ld\n", initSum);
	printf("Executing threads...\n");

	/* better to seed the random number generator outside
	   of do swaps or all threads will start with same
	   choice
	   */
	srand(time(NULL));

	time(&start_t);
	for (i = 0; i < nthreads; i++) {
		if (pthread_create(&(threads[i]), NULL, do_swaps, (void *)(&rowGranularity)) != 0) {
			perror("thread creation failed:");
			exit(-1);
		}

	}

	for (i = 0; i < nthreads; i++) {
		pthread_detach(threads[i]);

	}

	while(1){
#if defined(__APPLE__) || defined(__linux__)
		sleep(2);
#elif defined __WIN32
		Sleep(2000);
#endif
		if(threads_left == 0){
		    fprintf(stdout, "\nFinal Grid:\n\n");
		    PrintGrid(grid, gridsize);
		    finalSum = SumGrid(grid, gridsize); 
		    fprintf(stdout, "\n\nFinal Sum:  %ld\n", finalSum);
		    if (initSum != finalSum){
		      fprintf(stdout,"DATA INTEGRITY VIOLATION!!!!!\n");
		    } else {
		      fprintf(stdout,"DATA INTEGRITY MAINTAINED!!!!!\n");
		    }
		    fprintf(stdout, "Secs elapsed:  %g\n", difftime(end_t, start_t));

		    exit(0);
		  }
	}	
	
	
	return(0);
	
}






