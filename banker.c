/*
 ---------------------------------------------------------
 File Name: banker.c
 Project: Assignment 5 - Question 1
 ---------------------------------------------------------
 Authors:  Raeya Sangha and Suvethan Yogathasan
 IDs:      169020312 and 169039244
 Emails:   sang0312@mylaurier.ca and yoga9244@mylaurier.ca
 Version:  2026-07-13
 ---------------------------------------------------------
 This program implements the banker algorithm for resource
 allocation and deadlock avoidance. It keeps track of the 
 available, max, allocated, and needed resources for each
 customer.
 ---------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


#define NUMBER_OF_RESOURCES 4
#define NUMBER_OF_CUSTOMERS 5


//current ammount allocated to each customer
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

// amount availble for each thread
int available[NUMBER_OF_RESOURCES];

// max demand of each customer
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

// order which is safe for customers to run
int safe_sequence[NUMBER_OF_CUSTOMERS];

// need remaining for each cutomer
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

// functions
void read_file(void);
void initialize_values(void);
void print_matrix(int matrix[][NUMBER_OF_RESOURCES]);
void print_vector(int vector[]);
void print_status(void);

int safety_algorithm(void);
int release_resources(int customer_number, int release[]);
int request_resources(int customer_number, int request[]);

void *customer_thread(void *customer_number);
void run_customers(void);


// main function
int main(int argc, char *argv[])
{
    char user_command[20];
    int customer_number;
    int resources[NUMBER_OF_RESOURCES];
    int i;

    if (argc != NUMBER_OF_RESOURCES + 1)
    {
        printf("Usage: ./banker 10 5 7 8\n");
        return 1;
    }

    // initialize all availble resources from command line
    for (i = 0; i < NUMBER_OF_RESOURCES; i++)
    {
        available[i] = atoi(argv[i + 1]);
    }

    read_file();
    initialize_values();

    // print the initial resource info
    printf("Number of Customers: %d\n", NUMBER_OF_CUSTOMERS);

    printf("Currently Available resources: ");
    print_vector(available);

    printf("Maximum resources from file:\n");
    print_matrix(maximum);

    while (1)
    {
        printf("Enter Command: ");
        scanf("%19s", user_command);

        // request resources: RQ customer resource1 resource2 resource3 resource4
        if (strcmp(user_command, "RQ") == 0)
        {
            scanf("%d", &customer_number);

            for (i = 0; i < NUMBER_OF_RESOURCES; i++)
            {
                scanf("%d", &resources[i]);
            }

            if (request_resources(customer_number, resources) == 0)
            {
                printf("State is safe, and request is satisfied\n");
            }
            else
            {
                printf("State is unsafe, and request is denied\n");
            }
        }

     // release resources: RL customer resource1 resource2 resource3 resource4
         
        else if (strcmp(user_command, "RL") == 0)
        {
            scanf("%d", &customer_number);

            for (i = 0; i < NUMBER_OF_RESOURCES; i++)
            {
                scanf("%d", &resources[i]);
            }

            if (release_resources(customer_number, resources) == 0)
            {
                printf("The resources have been released successfully\n");
            }
            else
            {
                printf("The resources could not be released\n");
            }
        }

     // display all resource arrays and matrices 

        else if (strcmp(user_command, "Status") == 0)
        {
            print_status();
        }

        // run customers as threads in a safe sequence 
        else if (strcmp(user_command, "Run") == 0)
        {
            run_customers();
        }

        // stop the porgam
        else if (strcmp(user_command, "Exit") == 0)
        {
            break;
        }

        // incorrect command handler error
        else
        {
            printf("Invalid input, use one of RQ, RL, Status, Run, Exit\n");
        }
    }

    return 0;
}


// reads the maximum resource demand for each customer from sample_in_banker.txt.
void read_file(void)
{
    FILE *file;
    int customer_index;

    file = fopen("sample_in_banker.txt", "r");

    if (file == NULL)
    {
        printf("Unable to open sample_in_banker.txt\n");
        exit(1);
    }

    for (customer_index = 0;
         customer_index < NUMBER_OF_CUSTOMERS;
         customer_index++)
    {
        fscanf(file,
               "%d,%d,%d,%d",
               &maximum[customer_index][0],
               &maximum[customer_index][1],
               &maximum[customer_index][2],
               &maximum[customer_index][3]);
    }

    fclose(file);
}


// Sets every initial allocation to zero so each initial need equals the maximum demand.
 
void initialize_values(void)
{
    int customer;
    int resource_index;

    for (customer = 0;
         customer < NUMBER_OF_CUSTOMERS;
         customer++)
    {
        for (resource_index = 0;
             resource_index < NUMBER_OF_RESOURCES;
             resource_index++)
        {
            allocation[customer][resource_index] = 0;
            need[customer][resource_index] =
                maximum[customer][resource_index];
        }
    }
}


// print one row of resource values
void print_vector(int vector[])
{
    int resource;

    for (resource = 0;
         resource < NUMBER_OF_RESOURCES;
         resource++)
    {
        printf("%d", vector[resource]);

        if (resource < NUMBER_OF_RESOURCES - 1)
        {
            printf(" ");
        }
    }

    printf("\n");
}


// prints a matrix containing one row for each customer
 
void print_matrix(int matrix[][NUMBER_OF_RESOURCES])
{
    int customer;
    int resource;

    for (customer = 0;
         customer < NUMBER_OF_CUSTOMERS;
         customer++)
    {
        for (resource = 0;
             resource < NUMBER_OF_RESOURCES;
             resource++)
        {
            printf("%d", matrix[customer][resource]);

            if (resource < NUMBER_OF_RESOURCES - 1)
            {
                printf(" ");
            }
        }

        printf("\n");
    }
}

// prints the current state of all arrays and matrices
 
void print_status(void)
{
    printf("Available Resources:\n");
    print_vector(available);

    printf("Maximum Resources:\n");
    print_matrix(maximum);

    printf("Allocated Resources:\n");
    print_matrix(allocation);

    printf("Need Resources:\n");
    print_matrix(need);
}


// check if the current resource state is safe if it is safe the order is stored in safe_sequence.
 
int safety_algorithm(void)
{
    int work[NUMBER_OF_RESOURCES];
    int finish[NUMBER_OF_CUSTOMERS] = {0};
    int count = 0;
    int customer;
    int resource;
    int can_finish;
    int customers_found;

    //work begins with the available resources 
    for (resource = 0;
         resource < NUMBER_OF_RESOURCES;
         resource++)
    {
        work[resource] = available[resource];
    }

    while (count < NUMBER_OF_CUSTOMERS)
    {
        customers_found = 0;

        for (customer = 0;
             customer < NUMBER_OF_CUSTOMERS;
             customer++)
        {
            if (finish[customer] == 0)
            {
                can_finish = 1;

                for (resource = 0;
                     resource < NUMBER_OF_RESOURCES;
                     resource++)
                {
                    if (need[customer][resource] >
                        work[resource])
                    {
                        can_finish = 0;
                        break;
                    }
                }

            // this customer can finish and return its allocated resources
                 
                if (can_finish == 1)
                {
                    for (resource = 0;
                         resource < NUMBER_OF_RESOURCES;
                         resource++)
                    {
                        work[resource] +=
                            allocation[customer][resource];
                    }

                    safe_sequence[count] = customer;
                    count++;

                    finish[customer] = 1;
                    customers_found = 1;
                }
            }
        }

        //no unfinished customer could run so the system is not in a safe state
         
        if (customers_found == 0)
        {
            return 0;
        }
    }

    return 1;
}


//attempts to grant a customers resource request
 
int request_resources(int customer_number, int request[])
{
    int resource;

    if (customer_number < 0 ||
        customer_number >= NUMBER_OF_CUSTOMERS)
    {
        return -1;
    }

    //the request cannot be greater than the customers remaining need
     
    for (resource = 0;
         resource < NUMBER_OF_RESOURCES;
         resource++)
    {
        if (request[resource] < 0 ||
            request[resource] >
            need[customer_number][resource])
        {
            return -1;
        }
    }

    //the request cannot be greater than the currently available resources
     
    for (resource = 0;
         resource < NUMBER_OF_RESOURCES;
         resource++)
    {
        if (request[resource] > available[resource])
        {
            return -1;
        }
    }

    //temporarily allocate the requested resources 
    for (resource = 0;
         resource < NUMBER_OF_RESOURCES;
         resource++)
    {
        available[resource] -= request[resource];

        allocation[customer_number][resource] +=
            request[resource];

        need[customer_number][resource] -=
            request[resource];
    }

    //keep the allocation if the new state is safe
     
    if (safety_algorithm() == 1)
    {
        return 0;
    }

    //restore the old state if the request creates an unsafe state
     
    for (resource = 0;
         resource < NUMBER_OF_RESOURCES;
         resource++)
    {
        available[resource] += request[resource];

        allocation[customer_number][resource] -=
            request[resource];

        need[customer_number][resource] +=
            request[resource];
    }

    return -1;
}


//releases resources currently held by a customer
 
int release_resources(int customer_number, int release[])
{
    int resource;

    if (customer_number < 0 ||
        customer_number >= NUMBER_OF_CUSTOMERS)
    {
        return -1;
    }

    //a customer cannot release more resources than it currently has allocated
     
    for (resource = 0;
         resource < NUMBER_OF_RESOURCES;
         resource++)
    {
        if (release[resource] < 0 ||
            release[resource] >
            allocation[customer_number][resource])
        {
            return -1;
        }
    }

    //update the available allocation and need values 
    for (resource = 0;
         resource < NUMBER_OF_RESOURCES;
         resource++)
    {
        available[resource] += release[resource];

        allocation[customer_number][resource] -=
            release[resource];

        need[customer_number][resource] +=
            release[resource];
    }

    return 0;
}


// finds the safe sequence and creates one customer thread at a time in that sequence
 
void run_customers(void)
{
    pthread_t customer_thread_id;
    int customer_numbers[NUMBER_OF_CUSTOMERS];
    int i;

    if (safety_algorithm() == 0)
    {
        printf("No safe sequence is available\n");
        return;
    }

    printf("Safe Sequence is: ");

    for (i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        printf("%d", safe_sequence[i]);

        if (i < NUMBER_OF_CUSTOMERS - 1)
        {
            printf(" ");
        }
    }

    printf("\n");

    //each thread is joined before creating the next thread so customers execute in safe-sequence order
     
    for (i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        customer_numbers[i] = safe_sequence[i];

        pthread_create(&customer_thread_id,
                       NULL,
                       customer_thread,
                       &customer_numbers[i]);

        pthread_join(customer_thread_id, NULL);
    }
}


//runs one customer and releases all resources currently allocated to that customer
 
void *customer_thread(void *customer_number)
{
    int customer;
    int resource;

    customer = *((int *)customer_number);

    printf("--> Customer/Thread %d\n", customer);

    printf("    Allocated resources: ");
    print_vector(allocation[customer]);

    printf("    Needed: ");
    print_vector(need[customer]);

    printf("    Available: ");
    print_vector(available);

    printf("    Thread has started\n");
    printf("    Thread has finished\n");
    printf("    Thread is releasing resources\n");

    //release all resources currently allocated to this customer
     
    for (resource = 0;
         resource < NUMBER_OF_RESOURCES;
         resource++)
    {
        available[resource] +=
            allocation[customer][resource];

        allocation[customer][resource] = 0;

        need[customer][resource] =
            maximum[customer][resource];
    }

    printf("    New Available: ");
    print_vector(available);

    return NULL;
}