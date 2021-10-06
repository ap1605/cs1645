#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define		NROW	1024
#define		NCOL	NROW

#define 	NUM_THREADS 8

#define TEST_RESULTS


//Matrix A
int matrixA  [NROW][NCOL];
//Matrix B
int matrixB  [NROW][NCOL];
//Matrix C
int matrixC [NROW][NCOL];

//Temp array
int tempMatrix [NROW][NCOL];

//Output Array C
int outputMatrix [NROW][NCOL];

struct timeval startTime;
struct timeval finishTime;
double timeIntervalLength;

void verifyMatrixSum();


// mutex
pthread_mutex_t mutex_p = PTHREAD_MUTEX_INITIALIZER;


// Pthread_matrix_mult

void *Pthread_matrix_mult(void *threadid){ // only needs to do one matrix, can call again for other?
    long tid;
    int temp_val;
    tid = (long) threadid;

    int i, j, k;


    //pthread_mutex_lock(&mutex_p);
    int my_work = NROW/NUM_THREADS; // might want to lock this as it is accessing a global
     // amount of work is equal to Nrow because we go through each of the positions in the
     // row and multiplies and sum up each multiply
    //pthread_mutex_unlock(&mutex_p);

    
    int my_first_row = tid * my_work;
    int my_last_row = ((tid +1) * my_work);
 

    // maybe lock bc accessing global matrices
    for(i = my_first_row; i < my_last_row; i++){ // this will be matrix A row and then temp Matrix's row
        for(j = 0; j <  NCOL; j++){ // this will be Matrix B column            
            for(k = 0; k < NROW; k++){
                //pthread_mutex_lock(&mutex_p);
                tempMatrix[i][j] += matrixA[i][k] * matrixB[k][j];
                //pthread_mutex_unlock(&mutex_p);
            }
        }

        for(j = 0; j < NCOL; j++){ // this will be matrix C column
            for(k = 0; k < NROW; k++){
                //pthread_mutex_lock(&mutex_p);
                outputMatrix[i][j] += tempMatrix[i][k] * matrixC[k][j];
                //pthread_mutex_unlock(&mutex_p);
            }    
        }
    }
}



int main(int argc, char* argv[])
{
    int i,j,k;

    // Matrix initialization. Just filling with arbitrary numbers.
    for(i=0;i<NROW;i++)
    {
        for(j=0;j<NCOL;j++)
        {
            matrixA[i][j]= (i + j)/128;
            matrixB[i][j]= (j + j)/128;
            matrixC[i][j]= (i + j)/128;
            tempMatrix[i][j] = 0;
            outputMatrix[i][j]= 0;
        }
    }

    //Get the start time
    gettimeofday(&startTime, NULL); /* START TIME */

	// DECLARE THREADS
    pthread_t threads[NUM_THREADS]; // Handles for our threads
    int rc;
    long t;
    void *status;
    
    // CREATE MULTIPLE THREADS
    for(t=0; t<NUM_THREADS; t++){
    
       printf("In main: creating thread %ld\n", t);
    
        rc = pthread_create(&threads[t], NULL, Pthread_matrix_mult, (void *)t); //pthread_create is always inside the loop

        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    
    
    // JOIN THREADS
    for(t=0; t<NUM_THREADS; t++) {
        rc = pthread_join(threads[t], &status);
    }

    //Get the end time
    gettimeofday(&finishTime, NULL);  /* END TIME */

    //Calculate the interval length
    timeIntervalLength = (double)(finishTime.tv_sec-startTime.tv_sec) * 1000000
                         + (double)(finishTime.tv_usec-startTime.tv_usec);
    timeIntervalLength=timeIntervalLength/1000;

    #ifdef TEST_RESULTS
    //[Verifying the matrix summation]
    verifyMatrixSum();
    #endif

    //Print the interval length
    printf("Interval length: %g msec.\n", timeIntervalLength);

    return 0;
}


// Helper function to verify if the sum from parallel and serial versions match
void verifyMatrixSum() {
    int i, j;

    double totalSum;
    totalSum=0;
    //
    for(i=0;i<NROW;i++){
        for(j=0;j<NCOL;j++)
        {
            totalSum+=(double)outputMatrix[i][j];
        }
    }
    printf("\nTotal Sum = %g\n",totalSum);
}



