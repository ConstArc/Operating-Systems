#pragma once
#include <stdbool.h>

typedef struct Record {
    int  id;
    char surname[20];
    char name[20];
    char postal_code[6];
} record;

// Compares record A with record B
int records_compare(record A, record B);

// Swaps record A and record B
void records_swap(record* A, record* B);

// A simple print function for record R
void record_print(record R);

// Testing purposes
void record_print_pid(record R);
