// Authors: Maria Adams && Chris Miller 
// Jacobi Algorithm

#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include <math.h>
#include <semaphore.h>

#define NUM_OF_THREAD  4
#define MAX_SIZE  1024
#define THRESHOLD 0.00001


//Global Variables:
double **newMatrix;
double **matrix;
int answers[NUM_OF_THREAD];  
int flag = 0;

pthread_barrier_t theBarrier; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


/* Barrier 
 *
 * Our implementation of the barrier object is created with 
 * a count of threads inside the barrier, a lock, and two 
 * conditions used to make the threads wait, 'in' and 'out',
 * which indicate when threads are all coming in or leaving.
 */
typedef struct{
    int count;
    sem_t lock; 
    sem_t in;
    sem_t out;
} barrier_t;

barrier_t barrier;//Create our barrier object

/* Create Barrier 
 *
 * Initializes the barrier to its default values, setting the 
 * lock and the two conditions to zero.
 */
void create_barrier(barrier_t *b){// initialize the barrier
	b->count = 0;
	sem_init(&b->lock, 0, 1);
	sem_init(&b->in, 0, 0);
	sem_init(&b->out, 0, 0);
}

/* Barrier In
 *
 * Used to stop all threads until the number of threads 
 * inside the barrier is the same number of total threads.
 * 
 * As threads come in, they lock, increment the total number
 * of threads in the barrier, then check if it is full or not. 
 * If the all threads are in the barrier, the waiting threads 
 * are allowed to leave. 
 */
void barrierIn(barrier_t *b){
	sem_wait(&b->lock);// lock 
	b->count++;// thread coming in 
	if(b->count == NUM_OF_THREAD){// if full
		for(int i = 0; i < NUM_OF_THREAD; i++){
			sem_post(&b->in);// post for every thead 
		}
	}
	sem_post(&b->lock);//unlock 
	sem_wait(&b->in);
}

/* Barrier Out 
 *
 * Used to reset our barrier, setting the count back to zero. 
 * As threads are leaving the first barrier, they funnel into 
 * the second, decrementing the count until it is zero, so that 
 * future runs will be able to utilize the same barrier.
 */
void barrierOut(barrier_t *b){
	sem_wait(&b->lock);// lock 
	b->count--;// thread leaving 
	if(b->count == 0){// if empty 
		for(int i = 0; i < NUM_OF_THREAD; i++){
			sem_post(&b->out);// post every thread
		}
	}
	sem_post(&b->lock);// unlock
	sem_wait(&b->out);
}

/* Barrier Wait 
 *
 * The barrier wait function calls both parts 
 * of the barrier structure, first waiting until all
 * threads are in the barrier before allowing them to move on,
 * then decrementing the count until zero so that it can 
 * be reused in future iterations of the Jacobi.
 */
void barrier_wait(barrier_t *b){
    barrierIn(b);//threads coming in
    barrierOut(b); // threads coming out 
}

/* Print Matrix 
 *
 * The print matrix function takes in a two 
 * dimensional array and prints out each of its elements, 
 * using %.10lf to fully display all of the decimals.
 */
void print_matrix(double **m){
   int r, c; 
   for(r=0; r<MAX_SIZE; r++){ 
	for(c=0; c<MAX_SIZE; c++){
	   printf("%.10lf ", m[r][c]); 
     	}
	printf("\n");
   }
} 

/* Load Matrix
 *
 * The load matrix function takes in a file name string
 * as its parameter, in order to open the file and read 
 * all of its contents into the global 'matrix' variable.
 */ 
void load_matrix(char* str){
    int r, c; 
    FILE *f = fopen(str, "r"); 
    if(f == NULL){
        exit(1); 
    }
    for(r=0; r<MAX_SIZE; r++){
        for(c=0; c<MAX_SIZE; c++){     
            fscanf(f,"%lf ", &matrix[r][c]);
        }
    }
}

/* Thread Function
 *
 * As each thread is made, they funnel into this function
 * and each begin perforiming the Jacobi algorithm on the matrix. 
 * First summing the four directions, finding the largest change between
 * the old and new matrix for that rotation. 
 * 
 * Lastly each thread copies their work into the global matrix variable,
 * and then checks to see if the answers array is full of 1's 
 * (indicating each thread has reached the threshold of change). 
 */
void *threadfunc(void *arg){
    long tid= (long) arg; 
    int rowFrom = (( (tid * (MAX_SIZE-2))) / NUM_OF_THREAD) + 1;
    int rowTo= ( ((tid+1) * (MAX_SIZE-2)) / NUM_OF_THREAD) + 1;
    
    double temp;   
    double temp2;

     while(flag == 0){   
        temp = 0;
        temp2 = 0;
                    
        //do jacobi 
        for(int i = rowFrom; i < rowTo; i++){ 		 
            for(int j = 1; j < MAX_SIZE-1; j++){
                newMatrix[i][j] = (matrix[i][j+1]+matrix[i][j-1]+matrix[i+1][j]+matrix[i-1][j])/4;
                temp = fabs(newMatrix[i][j] - matrix[i][j]); 
                if(temp > temp2 && temp!=0){
                    temp2 = temp;
                }
            }
        }
        //printf("Thread #%ld: maxdiff = %.10lf \n",tid, temp2);
        
        if(temp2 < THRESHOLD){
            answers[tid] = 1;
        }
        barrier_wait(&barrier);
                
        //copy over new to old
        for(int i = rowFrom; i < rowTo; i++){
            for(int j = 1; j < MAX_SIZE-1; j++){     
                matrix[i][j] =  newMatrix[i][j]; 
            }
        }
        
        int count = 0;
        for(int i =0; answers[i] != 0 && i < NUM_OF_THREAD; i++){//keep counting while answers is not 0 
            printf("answers[%d] = %d \n", i, answers[i]);
            count++;           
        }
        if(count == NUM_OF_THREAD){//if the answers array is full of 1 
            pthread_mutex_lock(&mutex);
            flag = 1; // set the flag 
            pthread_mutex_unlock(&mutex);
        }
        barrier_wait(&barrier);
    }// end of while loop
   pthread_exit(NULL); 

}

/* Main 
 * 
 * The main function allocates space for both matrices, then 
 * fills the matrix with input based on our given input file. 
 * 
 * N amount of threads are then created and each goes about looping
 * through the matrix, doing the Jacobi algorithm until each have 
 * reached the limit. 
 * 
 * When every thread has finished its work, the 
 * main resumes by joining the threads and lastly printing the 
 * final matrix to output.
 */
int main(int argc, char* argv[]){
    newMatrix = (double **)malloc(1024 * sizeof(double *));
    matrix = (double **)malloc(1024 * sizeof(double *));
    
    for(int i = 0; i < 1024; i++){  // malloc space for both matrices
        matrix[i] = (double *)malloc(1024 * sizeof(double));
        newMatrix[i] = (double *)malloc(1024 * sizeof(double));
    }
    
    for(int i =0; i < NUM_OF_THREAD; i++){//fill the answers array with zero
        answers[i] = 0;
    }

   create_barrier(&barrier); //create our barrier 
   load_matrix("./input.mtx"); 
   
    //copy over the loaded matrix into newMatrix(to get the edges)
    for(int i = 0; i < MAX_SIZE; i++){
        for(int j = 0; j < MAX_SIZE; j++){
            newMatrix[i][j] =  matrix[i][j];       
        }
    } 
   
   pthread_t threads[NUM_OF_THREAD];
   long arrThreadID[NUM_OF_THREAD]; 
   
   long t; int ret=0; 
   for(t=0; t<NUM_OF_THREAD; t++){
       arrThreadID[t]= t; 
       ret = pthread_create(&threads[t], NULL, &threadfunc,(void*) arrThreadID[t]); 
        if(ret){
            printf("something's wrong with thread creation %d\n", ret);
            exit(1); 
        }
    } 
    
   for(int a=0; a<NUM_OF_THREAD; a++){
   	 pthread_join(threads[a], NULL);  
     }
    
    print_matrix(matrix); 
    printf("DONE! with #%d threads \n", NUM_OF_THREAD); 
   pthread_exit(NULL);     
   return 0; 
}

