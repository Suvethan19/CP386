/*
 ---------------------------------------------------------
 File: process_management.c
 Project: Assignment 3
 ---------------------------------------------------------
 Author:  Raeya Sangha and Suvethan Yogathasan
 ID:      169020312 and 169039244
 Email:   sang0312@mylaurier.ca and yoga9244@mylaurier.ca
 Version  2026-06-20
 ---------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 9
#define NUM_THREADS 11

int sudoku[SIZE][SIZE];
int results[NUM_THREADS];

typedef struct {
    int row;
    int column;
    int index;
} parameters;

/* Check all rows */
void *check_rows(void *param) {
    for (int i = 0; i < SIZE; i++) {
        int seen[10] = {0};

        for (int j = 0; j < SIZE; j++) {
            int num = sudoku[i][j];

            if (num < 1 || num > 9 || seen[num]) {
                results[0] = 0;
                pthread_exit(NULL);
            }

            seen[num] = 1;
        }
    }

    results[0] = 1;
    pthread_exit(NULL);
}

/* Check all columns */
void *check_columns(void *param) {
    for (int j = 0; j < SIZE; j++) {
        int seen[10] = {0};

        for (int i = 0; i < SIZE; i++) {
            int num = sudoku[i][j];

            if (num < 1 || num > 9 || seen[num]) {
                results[1] = 0;
                pthread_exit(NULL);
            }

            seen[num] = 1;
        }
    }

    results[1] = 1;
    pthread_exit(NULL);
}

/* Check one 3x3 subgrid */
void *check_subgrid(void *param) {
    parameters *data = (parameters *)param;

    int row = data->row;
    int col = data->column;
    int index = data->index;

    int seen[10] = {0};

    for (int i = row; i < row + 3; i++) {
        for (int j = col; j < col + 3; j++) {

            int num = sudoku[i][j];

            if (num < 1 || num > 9 || seen[num]) {
                results[index] = 0;
                free(data);
                pthread_exit(NULL);
            }

            seen[num] = 1;
        }
    }

    results[index] = 1;

    free(data);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: ./sudoku <sudoku grid>\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");

    if (fp == NULL) {
        printf("Unable to open the file\n");
        return 1;
    }

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {

            if (fscanf(fp, "%d", &sudoku[i][j]) != 1) {
                fclose(fp);
                printf("Invalid Sudoku file\n");
                return 1;
            }
        }
    }

    fclose(fp);

    printf("Sudoku Puzzle Solution is:\n");

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d ", sudoku[i][j]);
        }
        printf("\n");
    }

    pthread_t tid[NUM_THREADS];

    pthread_create(&tid[0], NULL, check_rows, NULL);
    pthread_create(&tid[1], NULL, check_columns, NULL);

    int threadIndex = 2;

    for (int row = 0; row < SIZE; row += 3) {
        for (int col = 0; col < SIZE; col += 3) {

            parameters *data =
                (parameters *)malloc(sizeof(parameters));

            data->row = row;
            data->column = col;
            data->index = threadIndex;

            pthread_create(
                &tid[threadIndex],
                NULL,
                check_subgrid,
                data
            );

            threadIndex++;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(tid[i], NULL);
    }

    int valid = 1;

    for (int i = 0; i < NUM_THREADS; i++) {
        if (results[i] == 0) {
            valid = 0;
            break;
        }
    }

    if (valid)
        printf("Sudoku puzzle is valid\n");
    else
        printf("Sudoku puzzle is invalid\n");

    return 0;
}
