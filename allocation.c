 /*
 ---------------------------------------------------------
 File Name: allocation.c
 Project: Assignment 5 - Question 2
 ---------------------------------------------------------
 Authors:  Raeya Sangha and Suvethan Yogathasan
 IDs:      169020312 and 169039244
 Emails:   sang0312@mylaurier.ca and yoga9244@mylaurier.ca
 Version:  2026-07-13
 ---------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#define MAX_PROC_NAME 32
#define MAX_LINE 256
 
typedef struct Block {
    int start;
    int size;
    int is_free;
    char process[MAX_PROC_NAME];
    struct Block *next;
} Block;
 
static Block *head = NULL;
static int MAX_MEM = 0;
 
 
static Block *make_block(int start, int size, int is_free, const char *proc) {
    Block *b = malloc(sizeof(Block));
    if (!b) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    b->start = start;
    b->size = size;
    b->is_free = is_free;
    if (proc)
        snprintf(b->process, MAX_PROC_NAME, "%s", proc);
    else
        b->process[0] = '\0';
    b->next = NULL;
    return b;
}
 
/* Merge any adjacent free blocks in the list. */
static void coalesce(void) {
    Block *b = head;
    while (b != NULL && b->next != NULL) {
        if (b->is_free && b->next->is_free) {
            Block *dead = b->next;
            b->size += dead->size;
            b->next = dead->next;
            free(dead);
            /* stay on b in case a third free block also needs merging */
        } else {
            b = b->next;
        }
    }
}
 
/* Request memory for a process using Best Fit. */
static void request_memory(const char *process, int size) {
    Block *best = NULL;
    int best_delta = MAX_MEM + 1; /* larger than any possible delta */
    int index = 0;
 
    for (Block *b = head; b != NULL; b = b->next) {
        if (!b->is_free)
            continue;
        if (b->size >= size) {
            int delta = b->size - size;
            if (delta < best_delta) {
                best_delta = delta;
                best = b;
            }
        }
        index++;
    }
 
    if (best == NULL) {
        printf("No hole of sufficient size\n");
        return;
    }
 
    if (best->size == size) {
        best->is_free = 0;
        snprintf(best->process, MAX_PROC_NAME, "%s", process);
    } else {
        Block *hole = make_block(best->start + size, best->size - size, 1, NULL);
        hole->next = best->next;
        best->next = hole;
        best->size = size;
        best->is_free = 0;
        snprintf(best->process, MAX_PROC_NAME, "%s", process);
    }
 
    printf("Successfully allocated %d to process %s\n", size, process);
}
 
/* Release memory held by a process. */
static void release_memory(const char *process) {
    Block *b = head;
    while (b != NULL && (b->is_free || strcmp(b->process, process) != 0))
        b = b->next;
 
    if (b == NULL) {
        printf("No memory allocated to process %s\n", process);
        return;
    }
 
    b->is_free = 1;
    b->process[0] = '\0';
    coalesce();
    printf("Successfully released memory for process %s\n", process);
}
 
/* Print current partitions and holes. */
static void status(void) {
    int allocated_total = 0, free_total = 0;
    for (Block *b = head; b != NULL; b = b->next) {
        if (b->is_free)
            free_total += b->size;
        else
            allocated_total += b->size;
    }
 
    printf("Partitions [Allocated memory = %d]:\n", allocated_total);
    for (Block *b = head; b != NULL; b = b->next) {
        if (!b->is_free)
            printf("Address [%d:%d] Process %s\n", b->start, b->start + b->size - 1, b->process);
    }
 
    printf("\n");
    printf("Holes [Free memory = %d]:\n", free_total);
    for (Block *b = head; b != NULL; b = b->next) {
        if (b->is_free)
            printf("Address [%d:%d] len = %d\n", b->start, b->start + b->size - 1, b->size);
    }
}
 
/* Compact all allocated partitions to the front of memory, leaving
 * a single hole covering the remaining space. Updates each process's
 * starting address to reflect its new location. */
static void compact(void) {
    int addr = 0;
    for (Block *b = head; b != NULL; b = b->next) {
        if (!b->is_free) {
            b->start = addr;
            addr += b->size;
        }
    }
 
    Block *new_list = NULL, *tail = NULL;
    Block *b = head;
    while (b != NULL) {
        Block *next = b->next;
        if (!b->is_free) {
            b->next = NULL;
            if (tail == NULL) {
                new_list = b;
                tail = b;
            } else {
                tail->next = b;
                tail = b;
            }
        } else {
            free(b);
        }
        b = next;
    }
 
    int free_size = MAX_MEM - addr;
    if (free_size > 0) {
        Block *hole = make_block(addr, free_size, 1, NULL);
        if (tail == NULL)
            new_list = hole;
        else
            tail->next = hole;
    }
 
    head = new_list;
    printf("Compaction process is successful\n");
}
 
static void free_all(void) {
    Block *b = head;
    while (b != NULL) {
        Block *next = b->next;
        free(b);
        b = next;
    }
    head = NULL;
}
 
 
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <memory size in bytes>\n", argv[0]);
        return 1;
    }
 
    MAX_MEM = atoi(argv[1]);
    if (MAX_MEM <= 0) {
        fprintf(stderr, "Memory size must be a positive integer\n");
        return 1;
    }
 
    head = make_block(0, MAX_MEM, 1, NULL);
 
    printf("Here, the Best Fit approach has been implemented and the allocated %d bytes of memory.\n", MAX_MEM);
 
    char line[MAX_LINE];
    while (1) {
        printf("allocator> ");
        fflush(stdout);
 
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;
 
        char cmd[16];
        if (sscanf(line, "%15s", cmd) != 1)
            continue; /* blank line */
 
        if (strcmp(cmd, "RQ") == 0) {
            char proc[MAX_PROC_NAME];
            int size;
            char algo;
            if (sscanf(line, "%*s %31s %d %c", proc, &size, &algo) == 3) {
                (void)algo; /* algorithm is fixed at Best Fit for this program */
                if (size <= 0)
                    printf("Invalid input, use one of RQ, RL, C, Status, Exit\n");
                else
                    request_memory(proc, size);
            } else {
                printf("Invalid input, use one of RQ, RL, C, Status, Exit\n");
            }
        } else if (strcmp(cmd, "RL") == 0) {
            char proc[MAX_PROC_NAME];
            if (sscanf(line, "%*s %31s", proc) == 1)
                release_memory(proc);
            else
                printf("Invalid input, use one of RQ, RL, C, Status, Exit\n");
        } else if (strcmp(cmd, "C") == 0) {
            compact();
        } else if (strcmp(cmd, "Status") == 0) {
            status();
        } else if (strcmp(cmd, "Exit") == 0) {
            break;
        } else {
            printf("Invalid input, use one of RQ, RL, C, Status, Exit\n");
        }
    }
 
    free_all();
    return 0;
}
