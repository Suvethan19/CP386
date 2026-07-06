 /*
 ---------------------------------------------------------
 File Name: thread_synchronization
 Project: Assignment 4 - Question 2
 ---------------------------------------------------------
 Authors:  Raeya Sangha and Suvethan Yogathasan
 IDs:      169020312 and 169039244
 Emails:   sang0312@mylaurier.ca and yoga9244@mylaurier.ca
 Version:  2026-07-03
 ---------------------------------------------------------
*/

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

#define N 10

sem_t mutex;
sem_t even_sem;
sem_t odd_sem;

int first_thread = 1;
int last_parity = -1; // 0 = even, 1 = odd

void* thread_func(void* arg) {
    int id = *(int*)arg;
    int parity = id % 2;

    // First thread always enters
    sem_wait(&mutex);

    if (first_thread) {
        first_thread = 0;
    } else {
        // enforce alternating parity
        if (parity == 0)
            sem_wait(&even_sem);
        else
            sem_wait(&odd_sem);
    }

    // critical section
    printf("Thread %d entering critical section\n", id);

    last_parity = parity;

    // release opposite parity
    if (parity == 0)
        sem_post(&odd_sem);
    else
        sem_post(&even_sem);

    sem_post(&mutex);

    pthread_exit(NULL);
}

int main() {
    pthread_t threads[N];
    int ids[N];

    sem_init(&mutex, 0, 1);
    sem_init(&even_sem, 0, 0);
    sem_init(&odd_sem, 0, 0);

    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, thread_func, &ids[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&mutex);
    sem_destroy(&even_sem);
    sem_destroy(&odd_sem);

    return 0;
}
