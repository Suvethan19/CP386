/*
 ---------------------------------------------------------
 File: process_management.c
 Project: Assignment 2
 -------------------------------------
 Author:  Raeya Sangha and Suvethan Yogathasan
 ID:      169020312 and 169039244
 Email:   sang0312@mylaurier.ca and yoga9244@mylaurier.ca
 Version  2026-06-05
 ---------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SHM_SIZE 4096

// Provided output function
void writeOuput(char *command, char *output) {
    FILE *fp = fopen("output.txt", "a");
    if (fp == NULL) return;
    fprintf(fp, "The output of: %s is\n", command);
    fprintf(fp, ">>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<\n", output);
    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // 1. Setup Shared Memory
    int shm_fd = shm_open("OS_SHM", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHM_SIZE);
    char *ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // 2. Fork to Read File
    pid_t pid = fork();
    if (pid == 0) {
        FILE *f = fopen(argv[1], "r");
        char buffer[SHM_SIZE];
        fread(buffer, 1, SHM_SIZE, f);
        strcpy(ptr, buffer);
        fclose(f);
        exit(0);
    }
    wait(NULL);

    // 3. Parent reads from shared memory to dynamic array
    char commands[SHM_SIZE];
    strcpy(commands, ptr);
    munmap(ptr, SHM_SIZE);
    shm_unlink("OS_SHM");

    // 4. Process commands one by one
    char *line = strtok(commands, "\n");
    while (line != NULL) {
        int pipefd[2];
        pipe(pipefd);
        
        if (fork() == 0) {
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            
            char *args[] = {"/bin/sh", "-c", line, NULL};
            execvp(args[0], args);
            exit(0);
        } else {
            close(pipefd[1]);
            char output[SHM_SIZE] = {0};
            read(pipefd[0], output, SHM_SIZE);
            writeOuput(line, output);
            close(pipefd[0]);
            wait(NULL);
        }
        line = strtok(NULL, "\n");
    }

    return 0;
}
