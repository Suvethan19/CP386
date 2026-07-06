
/*
 ---------------------------------------------------------
 File: Stack.c
 Project: Assignment 4
 ---------------------------------------------------------
 Author:  Raeya Sangha and Suvethan Yogathasan
 ID:      169020312 and 169039244
 Email:   sang0312@mylaurier.ca and yoga9244@mylaurier.ca
 Version  2026-07-03
 ---------------------------------------------------------
 This prohgram demonstates thread synchronzation using a stack.
 Multiple push and pop thrrads access the same shared stack. A
 pthread mutex lock is used to protect the critical sections in 
 push() function and pop() function so only thread can modify 
 the stack at a time.
 ---------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define NUM_THREADS 	10
typedef struct Node
{
    int data;
    struct Node *next;
} StackNode;
StackNode *top;
//Mutex lock used to ensure that only one thread can access or modify the shared stack at one a time
pthread_mutex_t stack_lock; //added line for m utex lock
// function prototypes
void push(int v, StackNode **top);
int pop(StackNode **top);
int is_empty(StackNode *top);
// push function
void push(int v, StackNode **top)
{
    StackNode *new_node;
    //Lock the mutex before entering the critical section of the cide section
    pthread_mutex_lock(&stack_lock); //added line fot the code
    printf("Thread is running Push() operation for value: %d\n", v);
    new_node = (StackNode *)malloc(sizeof(StackNode));
    new_node->data = v;
    new_node->next = *top;
    *top = new_node;
    //unclock the mutex after the stack update is complete and finished
    pthread_mutex_unlock(&stack_lock); //added line for the code
}

// pop function
int pop(StackNode **top)
{
	StackNode *temp;
    // lockk the mutex before accessing the shared stack
    pthread_mutex_lock(&stack_lock); //added for teh code
   if (is_empty(*top)) {
    printf("Stack empty \n");
    //unlock the mutex before leaving the function
    pthread_mutex_unlock(&stack_lock); //added for the code
    return 0;
}
    else {
        int data = (*top)->data;
        printf("Thread is running Pop() operation and value is: %d\n",data);
		temp = *top;
        *top = (*top)->next;
		free(temp);
        //unlock the mutex aftwr removing the top element
        pthread_mutex_unlock(&stack_lock); //added 
        return data;
       
    }
}
//Check if top is NULL
int is_empty(StackNode *top) {
    if (top == NULL)
        return 1;
    else
        return 0;
}
// Thread's push function
void* thread_push(void *args)
{
	int i;
    int *threadId = (int *)args;
    push(*threadId + 1,&top);
    return NULL;
}
// Thread's pop function 
void* thread_pop(){
    pop(&top);
    return NULL;
}
// main function 
int main(void)
{
    //set global top pointer to NULL to indicATE that the stack is initially empty
    top = NULL; //changed
    pthread_t threads_push[NUM_THREADS];
    pthread_t threads_pop[NUM_THREADS];
    int thread_args[NUM_THREADS];
    int i, j;
    //initialize the mutex before creating any thread
    pthread_mutex_init(&stack_lock, NULL); //added line of code
    // Creating push threads
    for(i=0;i<NUM_THREADS;i++){
	thread_args[i] = i;
	    pthread_create(&threads_push[i],NULL,thread_push, (void *)&thread_args[i]);
	}
	// Create Pop Threads
	for(i=0;i<NUM_THREADS;i++){
        thread_args[i] = i;
	    pthread_create(&threads_pop[i],NULL,thread_pop, NULL);
	}
	// Join Push Threads
	for(j = 0; j < NUM_THREADS; j++)
	{
	pthread_join(threads_push[j],NULL);
	}
    // Join Pop Threads
	for(j = 0; j < NUM_THREADS; j++)
	{
	pthread_join(threads_pop[j],NULL);
	}
    //destoy the mutex after all threads have finishded and done being used
    pthread_mutex_destroy(&stack_lock); //added
    pthread_exit(NULL);
	return 0;
}
