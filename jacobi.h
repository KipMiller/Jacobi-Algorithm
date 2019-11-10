
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


/* Create Barrier 
 *
 * Initializes the barrier to its default values, setting the 
 * lock and the two conditions to zero.
 */
void create_barrier(barrier_t *b);


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
 
void barrierIn(barrier_t *b);


/* Barrier Out 
 *
 * Used to reset our barrier, setting the count back to zero. 
 * As threads are leaving the first barrier, they funnel into 
 * the second, decrementing the count until it is zero, so that 
 * future runs will be able to utilize the same barrier.
 */
void barrierOut(barrier_t *b);

/* Barrier Wait 
 *
 * The barrier wait function calls both parts 
 * of the barrier structure, first waiting until all
 * threads are in the barrier before allowing them to move on,
 * then decrementing the count until zero so that it can 
 * be reused in future iterations of the Jacobi.
 */
void barrier_wait(barrier_t *b);

/* Print Matrix 
 *
 * The print matrix function takes in a two 
 * dimensional array and prints out each of its elements, 
 * using %.10lf to fully display all of the decimals.
 */
  
void print_matrix(double **m);

/* Load Matrix
 *
 * The load matrix function takes in a file name string
 * as its parameter, in order to open the file and read 
 * all of its contents into the global 'matrix' variable.
 */ 
void load_matrix(char* str);


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
void *threadfunc(void *arg);




