#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define		NSTEPS	2000
#define		P_START	0
#define		P_END	5

#define 	NUM_THREADS 8


struct timeval startTime;
struct timeval finishTime;
double timeIntervalLength;
double area = 0.0;
int counter = 0;

double polynomial(double x);

// mutex
pthread_mutex_t mutex_p = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;



void *Pthread_trap(void *threadid){
    long tid;
    tid = (long) threadid;

    int my_work = NSTEPS/NUM_THREADS;

    int my_first_row = tid * my_work;
    int my_last_row = (tid+1) * my_work;

    double p_current = P_START;
    double step_size;
    double f_result_low, f_result_high;
    int i;

    // Calculating intermediary step sizes
    step_size = (double)(P_END - P_START) / NSTEPS;

    //Initial step position
    p_current = P_START;

    double my_area = 0.0;

    for(i = my_first_row; i < my_last_row; i++)
    {
        p_current = i * step_size;

        f_result_low = polynomial(p_current);
        f_result_high = polynomial(p_current + step_size);

        my_area += (f_result_low + f_result_high) * step_size / 2;
    
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
    area += my_area;
    pthread_mutex_unlock(&mutex_p);




}

int main() {

    //Get the start time
    gettimeofday(&startTime, NULL);
    
    // DECLARE THREADS
    pthread_t threads[NUM_THREADS]; // Handles for our threads
    int rc;
    long t;
    void *status;
    
    // CREATE MULTIPLE THREADS
    for(t=0; t<NUM_THREADS; t++){
    
       printf("In main: creating thread %ld\n", t);
    
        rc = pthread_create(&threads[t], NULL, Pthread_trap, (void *)t); //pthread_create is always inside the loop

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
    gettimeofday(&finishTime, NULL);  /* after time */

    //Calculate the interval length
    timeIntervalLength = (double)(finishTime.tv_sec-startTime.tv_sec) * 1000000
                         + (double)(finishTime.tv_usec-startTime.tv_usec);
    timeIntervalLength=timeIntervalLength/1000;

    //Print the interval length
    printf("Interval length: %g msec.\n", timeIntervalLength);

    printf("Result: %f \n",area);

    return 0;
}

// Calculates x->y values of a fixed polynomial
// Currently is https://www.desmos.com/calculator/swxvru1xxn
double polynomial(double x){

    // x^2
    double numerator = pow(x, 2);
    //(-4x^3+2x^4)
    double temp_poly = -4 * pow(x, 3) + 2 * pow(x, 4);

    //(-4x^3+2x^4)^2
    double temp_poly_2 = pow(temp_poly, 4);

    // x^3 + 2x^2 * (-4x^3+2x^4)^2
    double temp_poly_3 = pow(x, 3) + 2 * pow(x, 2) * temp_poly_2;

    // root square of (x^3 + 2x^2 * (-4x^3+2x^4)^2)
    double denominator = sqrt(temp_poly_3);

    double y = 0;
    if (denominator != 0)
        y = numerator / denominator;

    return y;
}
