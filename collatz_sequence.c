/*
 ---------------------------------------------------------
 File: collatz_sequence.c
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE 4096
#define SHM_NAME "/collatz_memory"

int main(int argc, char *argv[])
{
    int fd;
    char buffer[1024];
    ssize_t bytes_read;
    ssize_t k;

    int start_numbers[100];
    int num_count = 0;
    int current_number = 0;
    int building_number = 0;
    int index;

    if (argc != 2)
    {
        printf("Usage: ./collatz_sequence start_numbers.txt\n");
        return 1;
    }

    fd = open(argv[1], O_RDONLY);

    if (fd == -1)
    {
        printf("Error opening file.\n");
        return 1;
    }

    bytes_read = read(fd, buffer, sizeof(buffer));

    if (bytes_read == -1)
    {
        printf("Error reading file.\n");
        close(fd);
        return 1;
    }

    close(fd);

    for (k = 0; k <= bytes_read; k++)
    {
        char c;

        if (k == bytes_read)
            c = '\n';
        else
            c = buffer[k];

        if (c >= '0' && c <= '9')
        {
            current_number = current_number * 10 + (c - '0');
            building_number = 1;
        }
        else
        {
            if (building_number == 1)
            {
                start_numbers[num_count] = current_number;
                num_count++;

                current_number = 0;
                building_number = 0;
            }
        }
    }

    for (index = 0; index < num_count; index++)
    {
        int shm_fd;
        int *shared_mem;
        int sequence_index;
        int collatz_num;
        pid_t pid;

        collatz_num = start_numbers[index];

        printf("Parent Process: The positive integer read from file is %d\n", collatz_num);

        shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

        if (shm_fd == -1)
        {
            printf("Shared memory error.\n");
            return 1;
        }

        if (ftruncate(shm_fd, SIZE) == -1)
        {
            printf("ftruncate failed.\n");
            close(shm_fd);
            shm_unlink(SHM_NAME);
            return 1;
        }

        shared_mem = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

        if (shared_mem == (int *) MAP_FAILED)
        {
            printf("Map failed.\n");
            close(shm_fd);
            shm_unlink(SHM_NAME);
            return 1;
        }

        sequence_index = 0;
        shared_mem[sequence_index] = collatz_num;
        sequence_index++;

        while (collatz_num != 1)
        {
            if (collatz_num % 2 == 0)
                collatz_num = collatz_num / 2;
            else
                collatz_num = 3 * collatz_num + 1;

            shared_mem[sequence_index] = collatz_num;
            sequence_index++;
        }

        shared_mem[sequence_index] = -1;

        pid = fork();

        if (pid < 0)
        {
            printf("Fork failed.\n");
            munmap(shared_mem, SIZE);
            close(shm_fd);
            shm_unlink(SHM_NAME);
            return 1;
        }

        if (pid == 0)
        {
            int child_fd;
            int *child_ptr;
            int print_index = 0;

            child_fd = shm_open(SHM_NAME, O_RDONLY, 0666);

            if (child_fd == -1)
            {
                printf("Child shared memory error.\n");
                return 1;
            }

            child_ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, child_fd, 0);

            if (child_ptr == (int *) MAP_FAILED)
            {
                printf("Child map failed.\n");
                close(child_fd);
                return 1;
            }

            printf("Child Process: The generated collatz sequence is ");

            while (child_ptr[print_index] != -1)
            {
                printf("%d", child_ptr[print_index]);

                if (child_ptr[print_index + 1] != -1)
                    printf(" ");

                print_index++;
            }

            printf("\n");

            munmap(child_ptr, SIZE);
            close(child_fd);
            shm_unlink(SHM_NAME);

            return 0;
        }
        else
        {
            wait(NULL);

            munmap(shared_mem, SIZE);
            close(shm_fd);
        }
    }

    return 0;
}