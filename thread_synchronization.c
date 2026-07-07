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
#include <semaphore.h>
#include <sys/stat.h>
#include <time.h>
 
sem_t csLock;    
sem_t evenGate;  
sem_t oddGate;   
 
pthread_mutex_t stateLock = PTHREAD_MUTEX_INITIALIZER;
int evenWaiting     = 0;  
int oddWaiting      = 0;  
int lastParity      = -1; 
int threadsCreated  = 0;  
int totalThreads    = 0;  
 
void logStart(char *tID); 
void logFinish(char *tID); 
 
void startClock(); 
long getCurrentTime(); 
time_t programClock; 
 
typedef struct thread 
{
	char tid[4];
	unsigned int startTime;
	int state;
	pthread_t handle;
	int retVal;
	int parity;
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
 
	sem_init(&csLock, 0, 1);
	sem_init(&evenGate, 0, 0);
	sem_init(&oddGate, 0, 0);
 
	Thread *threads = NULL;
	int threadCount = readFile(argv[1], &threads);
	totalThreads = threadCount;
 
	for (int k = 0; k < threadCount; k++) {
		char yChar = threads[k].tid[strlen(threads[k].tid) - 1];
		int yDigit = yChar - '0';
		threads[k].parity = (yDigit % 2 == 0) ? 0 : 1;
	}
 
	startClock();
	while (threadsLeft(threads, threadCount) > 0)
	{
		int i = 0;
		while ((i = threadToStart(threads, threadCount)) > -1) {
			threads[i].state = 1;
			threads[i].retVal = pthread_create(&(threads[i].handle), NULL,
					threadRun, &threads[i]);
 
			// record that this thread has now actually been dispatched
			pthread_mutex_lock(&stateLock);
			threadsCreated++;
			pthread_mutex_unlock(&stateLock);
		}
	}
 
	// wait for every created thread to finish before exiting
	for (int k = 0; k < threadCount; k++) {
		if (threads[k].retVal == 0) {
			pthread_join(threads[k].handle, NULL);
		}
	}
 
	sem_destroy(&csLock);
	sem_destroy(&evenGate);
	sem_destroy(&oddGate);
	pthread_mutex_destroy(&stateLock);
	free(threads);
 
	return 0;
}
 
int readFile(char *fileName, Thread **threads)
{
	FILE *in = fopen(fileName, "r");
	if (!in) {
		printf(
				"Child A: Error in opening input file...exiting with error code -1\n");
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
		lines[i] = malloc(sizeof(command) * sizeof(char));
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
			if (j == 0)
				strcpy((*threads)[k].tid, token);
			if (j == 1)
				(*threads)[k].startTime = atoi(token);
			j++;
			token = strtok(NULL, ";");
		}
	}
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
		if (threads[k].state > -1)
			remainingThreads++;
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
 
void* threadRun(void *t)
{
	Thread *self = (Thread*) t;
	logStart(self->tid);
 
	int p = self->parity;
 
	pthread_mutex_lock(&stateLock);
	int proceed = 0;
 
	if (lastParity == -1) {
		proceed = 1;
	} else if (p != lastParity) {
		proceed = 1;
	} else {
		int oppositeWaiting = (p == 0) ? oddWaiting : evenWaiting;
		if (threadsCreated == totalThreads && oppositeWaiting == 0) {
			proceed = 1;
		}
	}
 
	if (proceed) {
		lastParity = p;
		pthread_mutex_unlock(&stateLock);
	} else {
		if (p == 0)
			evenWaiting++;
		else
			oddWaiting++;
		pthread_mutex_unlock(&stateLock);
		sem_wait(p == 0 ? &evenGate : &oddGate);
	}
 
	sem_wait(&csLock);
	printf("[%ld] Thread %s is in its critical section\n", getCurrentTime(),
			self->tid);
 
	logFinish(self->tid);
	self->state = -1;
	sem_post(&csLock);
 
	pthread_mutex_lock(&stateLock);
	int wantParity   = 1 - lastParity;
	int wantWaiting  = (wantParity == 0) ? evenWaiting : oddWaiting;
	int otherWaiting = (wantParity == 0) ? oddWaiting : evenWaiting;
 
	if (wantWaiting > 0) {
		if (wantParity == 0) {
			evenWaiting--;
			lastParity = 0;
			sem_post(&evenGate);
		} else {
			oddWaiting--;
			lastParity = 1;
			sem_post(&oddGate);
		}
	} else if (otherWaiting > 0 && threadsCreated == totalThreads) {
		if (wantParity == 0) {
			oddWaiting--;
			lastParity = 1;
			sem_post(&oddGate);
		} else {
			evenWaiting--;
			lastParity = 0;
			sem_post(&evenGate);
		}
	}
    
	pthread_mutex_unlock(&stateLock);
 
	pthread_exit(0);
}
 
void startClock() {
	programClock = time(NULL);
}
 
long getCurrentTime()
{
	time_t now;
	now = time(NULL);
	return now - programClock;
}