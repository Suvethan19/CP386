 /*
 ---------------------------------------------------------
 File Name: thread_synchronization.c
 Project: Assignment 4 - Question 2
 ---------------------------------------------------------
 Authors:  Raeya Sangha and Suvethan Yogathasan
 IDs:      169020312 and 169039244
 Emails:   sang0312@mylaurier.ca and yoga9244@mylaurier.ca
 Version:  2026-07-03
 ---------------------------------------------------------
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>
#include <semaphore.h>

sem_t running;
sem_t even;
sem_t odd;

void logStart(char *tID); 
void logFinish(char *tID); 
void startClock(); 
long getCurrentTime(); 
time_t programClock; 

typedef struct thread {
	char tid[4]; 
	unsigned int startTime;
	int state;
	pthread_t handle;
	int retVal;
} Thread;

int threadsLeft(Thread *threads, int threadCount);
int threadToStart(Thread *threads, int threadCount);
void* threadRun(void *t); 
int readFile(char *fileName, Thread **threads); 

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Input file name missing...exiting with error code -1\n");
		return -1;
	}

    // Initialize semaphores
    sem_init(&running, 0, 1);
    sem_init(&even, 0, 0);
    sem_init(&odd, 0, 0);

	Thread *threads = NULL;
	int threadCount = readFile(argv[1], &threads);

	startClock();
	while (threadsLeft(threads, threadCount) > 0) {
		int i = 0;
		while ((i = threadToStart(threads, threadCount)) > -1) {
			threads[i].state = 1;
			threads[i].retVal = pthread_create(&(threads[i].handle), NULL,
					threadRun, &threads[i]);
		}
	}

    // Wait for all threads to complete
    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i].handle, NULL);
    }

    // Cleanup
    sem_destroy(&running);
    sem_destroy(&even);
    sem_destroy(&odd);
    free(threads);

	return 0;
}

int readFile(char *fileName, Thread **threads) {
	FILE *in = fopen(fileName, "r");
	if (!in) {
		printf("Child A: Error in opening input file...exiting with error code -1\n");
		return -1;
	}

	struct stat st;
	fstat(fileno(in), &st);
	char *fileContent = (char*) malloc(((int) st.st_size + 1) * sizeof(char));
	fileContent[0] = '\0';
	while (!feof(in)) {
		char line[100];
		if (fgets(line, 100, in) != NULL) {
			strncat(fileContent, line, strlen(line));
		}
	}
	fclose(in);

	char *command = NULL;
	int threadCount = 0;
	char *fileCopy = (char*) malloc((strlen(fileContent) + 1) * sizeof(char));
	strcpy(fileCopy, fileContent);
	command = strtok(fileCopy, "\r\n");
	while (command != NULL) {
		threadCount++;
		command = strtok(NULL, "\r\n");
	}
	*threads = (Thread*) malloc(sizeof(Thread) * threadCount);

	char *lines[threadCount];
	command = NULL;
	int i = 0;
	command = strtok(fileContent, "\r\n");
	while (command != NULL) {
		lines[i] = malloc(strlen(command) + 1);
		strcpy(lines[i], command);
		i++;
		command = strtok(NULL, "\r\n");
	}

	for (int k = 0; k < threadCount; k++) {
		char *token = NULL;
		int j = 0;
		token = strtok(lines[k], ";");
		while (token != NULL) {
			(*threads)[k].state = 0;
			if (j == 0) strcpy((*threads)[k].tid, token);
			if (j == 1) (*threads)[k].startTime = atoi(token);
			j++;
			token = strtok(NULL, ";");
		}
        free(lines[k]);
	}
    free(fileContent);
    free(fileCopy);
	return threadCount;
}

void logStart(char *tID) {
	printf("[%ld] New Thread with ID %s is started.\n", getCurrentTime(), tID);
}

void logFinish(char *tID) {
	printf("[%ld] Thread with ID %s is finished.\n", getCurrentTime(), tID);
}

int threadsLeft(Thread *threads, int threadCount) {
	int remainingThreads = 0;
	for (int k = 0; k < threadCount; k++) {
		if (threads[k].state > -1) remainingThreads++;
	}
	return remainingThreads;
}

int threadToStart(Thread *threads, int threadCount) {
	for (int k = 0; k < threadCount; k++) {
		if (threads[k].state == 0 && threads[k].startTime == getCurrentTime())
			return k;
	}
	return -1;
}

void* threadRun(void *t) {
    Thread *threadPtr = (Thread*) t;
	logStart(threadPtr->tid);

    // Critical section synchronization
    sem_wait(&running);

	printf("[%ld] Thread %s is in its critical section\n", getCurrentTime(), threadPtr->tid);
    
    // Simulating work
    sleep(1); 

    sem_post(&running);

	logFinish(threadPtr->tid);
	threadPtr->state = -1;
	pthread_exit(0);
}

void startClock() {
	programClock = time(NULL);
}

long getCurrentTime() {
	time_t now = time(NULL);
	return now - programClock;
}