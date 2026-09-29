#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

//input validation
//make sure there are correct number of arguments
//darts at least 5 million
//threads at least 2

//create separate threads to generate random points
//use thread_args struct to pass arguments to each thread
//distribute points equally among threads
//use mutex to safely update global counters

//use rand_r for rng
//points in range [-1,1]
//if points are in circle x^2 + y^2 <= 1

//pi estimate = 4* points in circle / all points
//error messages
//thread creation failure
//free memory
//end mutex

long long all_points = 0;
long long points_in_circle = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
    long long num_points;
    unsigned int seed;
} thread_args;

//helper for generating random num between 0 and 1
//use random number between 1 and ~32000 and then divide it 
// by ~32000 + 1 so its between 0 and 1
double random_double(unsigned int* seed) {
    return rand_r(seed) / ((double)RAND_MAX + 1);
}

void* create_points(void* args) {
    thread_args* arg = (thread_args*)args;
    long long points_in_circle_L = 0;
    for (long long i = 0; i < arg->num_points; i++) {
        double x = random_double(&arg->seed) *2.0 - 1.0;
        double y = random_double(&arg->seed) *2.0 - 1.0;
        if (x * x + y * y <= 1.0) {
            points_in_circle_L++;
        }
    }

    pthread_mutex_lock(&mutex);
    points_in_circle += points_in_circle_L;
    all_points += arg->num_points;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char* argv[]){
    //do error checking for arguments, number of darts and number of threads
    if (argc != 3) {
        fprintf(stderr, "usage: pi <integer value for NUMBER_OF_DARTS> <integer value for NUMBER_OF_THREADS>\n");
        return 1;
    }
    long long num_darts = atoll(argv[1]);
    int num_threads = atoi(argv[2]);

    if (num_darts < 5000000) {
        fprintf(stderr, "The number of darts must be >= 5000000\n");
        return 1;

    }
    if (num_threads < 2) {
        fprintf(stderr, "The number of threads must be >= 2\n");

    }
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    thread_args* args = malloc(num_threads * sizeof(thread_args));

    long long points_in_thread = num_darts / num_threads;
    long long extra = num_darts % num_threads;

    for (int i = 0; i < num_threads; i++) { //create the threads
        args[i].num_points = points_in_thread + (i < extra ? 1 : 0); //give each thread about the same points
        args[i].seed = rand(); //create a different seed for each thread
        if (pthread_create(&threads[i], NULL, create_points, &args[i]) != 0) {
            perror("pthread_create error");
            return 1;
        }
    }
    for (int i = 0; i < num_threads; i++){
        pthread_join(threads[i], NULL);
    }
    double kinda_pi = 4.0 * points_in_circle / all_points;
    printf("Pi = %f\n", kinda_pi);

    //free memory
    free(threads);
    free(args);
    pthread_mutex_destroy(&mutex);
    return 0;
}
