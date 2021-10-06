#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/unistd.h>
#include <pthread.h>

#define XMAX 2.0
#define XMIN -2.0
#define YMAX 2.0
#define YMIN -2.0
#define N 10000                //number of divisions for the grid
#define ITER 50                //number of iterations for each point

#define TEST_RESULTS

#define 	NUM_THREADS 8


int pixels[N][N];

struct timeval startTime;
struct timeval finishTime;

//The printing is only for fun :)
void printMandelBrot();

void verifyMatrixSum();

double timeIntervalLength;


int counter = 0;
double all_Sum = 0;
// mutex
pthread_mutex_t mutex_p = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;


void *Pthread_mandelbrot(void *threadid){
    long tid;
    tid = (long) threadid;

    int i, j, count;
    int my_work = N/NUM_THREADS;

    double x, y;            //(x,y) point on the complex plane
    double x0, y0, tempx;
    double dx, dy;



    int my_first_row = (tid) * my_work;
    int my_last_row = (tid+1) * my_work;



    //increments in the real and imaginary directions
    dx = (XMAX - XMIN) / N;
    dy = (YMAX - YMIN) / N;




    //Get the start time
    gettimeofday(&startTime, NULL); /* START TIME */
    //calculations for mandelbrot

    for (i = my_first_row+1; i <= my_last_row; i++) {
        for (j = 1; j <= N; j++) {
            // c_real
            x0 = XMAX -
                 (dx * (i));  //scaled i coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.00, 2.00))
            // c_imaginary
            y0 = YMAX -
                 (dy * (j));  //scaled j coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.00, 2.00))

            // z_real
            x = 0;

            // z_imaginary
            y = 0;
            count = 0;

            while ((x * x + y * y < 4) && (count < ITER)) {
                tempx = (x * x) - (y * y) + x0;
                y = (2 * x * y) + y0;
                x = tempx;
                count++;
            }
            pixels[i - 1][j - 1] = count;
        }
    }

    // barrier
    pthread_mutex_lock(&mutex_p); 
    counter++;
    if(counter == NUM_THREADS){
        counter = 0;
        pthread_cond_broadcast(&cond_var);
    }
    else{
        while(pthread_cond_wait(&cond_var, &mutex_p) != 0);
    }
    pthread_mutex_unlock(&mutex_p);


    
    // Normalize the result based on the avg value
    double my_Sum = 0.0;
    double avg;
    for (i = my_first_row; i < my_last_row; i++) {
        for (j = 0; j < N; j++) {
            my_Sum += (double) pixels[i][j];
        }
    }

    // barrier
    pthread_mutex_lock(&mutex_p); 
    counter++;
    all_Sum += my_Sum;
    if(counter == NUM_THREADS){
        counter = 0;
        pthread_cond_broadcast(&cond_var);
    }
    else{
        while(pthread_cond_wait(&cond_var, &mutex_p) != 0);
    }
    pthread_mutex_unlock(&mutex_p);


    avg = all_Sum/(N*N);

    for (i = my_first_row; i < my_last_row; i++) {
        for (j = 0; j < N; j++) {
            pixels[i][j]=pixels[i][j]/avg;
        }
    }
}


int main() {

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
    
        rc = pthread_create(&threads[t], NULL, Pthread_mandelbrot, (void *)t); //pthread_create is always inside the loop

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
    timeIntervalLength = (double) (finishTime.tv_sec - startTime.tv_sec) * 1000000
                         + (double) (finishTime.tv_usec - startTime.tv_usec);
    timeIntervalLength = timeIntervalLength / 1000;
    //Print the interval length
    printf("Interval length: %g msec.\n", timeIntervalLength);


#ifdef TEST_RESULTS
    verifyMatrixSum(pixels);
//    printMandelBrot(pixels);
#endif
    return 0;
}

// Helper function to verify if the sum from parallel and serial versions match
void verifyMatrixSum() {
    int i, j;

    double totalSum;
    totalSum = 0;
    //
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            totalSum += (double) pixels[i][j];
        }
    }

    printf("\nTotal Sum = %g\n", totalSum);
}

void printMandelBrot() {
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            printf("%d ", pixels[i][j]);
        }
        printf("\n");
    }
}

