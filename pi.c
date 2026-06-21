/*
 ---------------------------------------------------------
 File Name: pi.c
 Project: Assignment 3 - Question 1
 ---------------------------------------------------------
 Authors:  Raeya Sangha and Suvethan Yogathasan
 IDs:      169020312 and 169039244
 Emails:   sang0312@mylaurier.ca and yoga9244@mylaurier.ca
 Version:  2026-06-19
 ---------------------------------------------------------
 This program estimates the Pi value using the Monte Carlo
 method. It creates multiple POSIX threads divides the total
 number of darts among the threads and each thread generates
 random points in the range [-1, 1]. Each thread counts how many
 points fall inside the unit circle and adds that local_points_inside to a
 shared global variable. After all threads finish, the main
 thread calculates and prints the estimated value of pi.
 ---------------------------------------------------------
 */

#include <stdlib.h>

#include <stdio.h>

#include <pthread.h>

/*Shared global variable used to store the total number of random points that fall inside the unit circle*/
long points_in_circle = 0;

/*Stores the total number of darts user provides */
long number_of_darts;

/*Stores the total number of threads user provides */
int number_of_threads;


/*This helper function generates a random double value in the range [0,1).
   This value is later converted to the range [-1,1] when generating x and y coordinates. */
double random_double()
{
    return random()/((double)RAND_MAX + 1);
}

/*Each thread executes this function. The thread generates its assigned number of random
   points, counts how many fall inside the unit circle and contributes its local_points_inside to the shared total.*/
void *runner(void *param)
{

    /* Retrieves the number of darts assigned to this thread */
    long assigned_darts = *((long *)param);
    long i;
    long local_points_inside = 0;
    double y_coordinate;
    double x_coordinate;
    
/* Generate random points and determine whether they fall inside the circle centered at (0,0) with radius 1*/
    for (i = 0; i < assigned_darts; i++)
    {
    /* Generates the random x and y coordinates in the range [-1,1]*/
        x_coordinate = random_double() * 2.0 - 1.0;
        y_coordinate = random_double() * 2.0 - 1.0;

        /* A point lies inside the unit circle if x^2 + y^2 <= 1 */
        if ((x_coordinate * x_coordinate + y_coordinate * y_coordinate) <= 1.0)
        {
            local_points_inside++;
        }
    }

    /* Add this thread's local_points_inside to the shared total local_points_inside*/
    points_in_circle += local_points_inside;

/*Terminate the thread and return control to the parent thread*/
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
     double pi;
    int i;
    pthread_t *threads;
    long *thread_darts;
    pthread_attr_t attr;
   
    
/* Verifying that both required command line arguments were provided by the user */
    if (argc != 3)
    {
        printf("usage: pi <integer value for NUMBER_OF_DARTS> <integer value for NUMBER_OF_THREADS>\n");
        return 1;
    }
/* Convert command-line arguments into numeric values */
    number_of_threads = atoi(argv[2]);
    number_of_darts = atol(argv[1]);
    

/*Assignment requirement: the number of darts must be at least 5000000 */
    if (number_of_darts < 5000000)
    {
        printf("The number of darts must be >= 5000000\n");
        return 1;
    }

    /* Assignment requirement: at least two threads must be used */
    if (number_of_threads < 2)
    {
        printf("The number of threads must be >= 2\n");
        return 1;
    }

/* Dynamically allocate memory for the thread IDs and for the number of darts assigned to each thread */
    threads = malloc(sizeof(pthread_t) * number_of_threads);
    thread_darts = malloc(sizeof(long) * number_of_threads);

     /*Initialize thread attributes using the default settings*/
    pthread_attr_init(&attr);

/* Divides the total number of darts equally among all the  threads*/
    for (i = 0; i < number_of_threads; i++)
    {
        thread_darts[i] = number_of_darts / number_of_threads;
    }

/*If the division is not exact, assign any remaining darts to the last thread*/
    thread_darts[number_of_threads - 1] += number_of_darts % number_of_threads;

     /* Creates the worker threads */
    for (i = 0; i < number_of_threads; i++)
    {
        pthread_create(&threads[i], &attr, runner, &thread_darts[i]);
    }

    /*Wait until every thread has completed its work */
    for (i = 0; i < number_of_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

/* Estimates Pi using the Monte Carlo formula: pi = 4*(points inside circle)/(total darts) */
    pi = 4.0 * points_in_circle / number_of_darts;

    /*Print the estimated value of pi */
    printf("Pi = %f\n", pi);

    /* Display the estimated value of pi */
    free(thread_darts);
    free(threads);
    

    return 0;
}