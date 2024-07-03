#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include "../include/record.h"

// Swaps the records A and B
void records_swap(record* A, record* B) {
    assert(A != NULL);
    assert(B != NULL);

    record temp;
    memcpy(&temp, A, sizeof(record));
    memcpy(A, B, sizeof(record));
    memcpy(B, &temp, sizeof(record));

}

// Compare record A with record B (string comparisons by ascii using strcmp)
// We check the scenarios below:
//  --> If A has greater surname than B,                           return 1:  '+' number
//  --> If A has the same surname with B, but greater name,        return 1:  '+' number
//  --> If A has the same surname and name with B, but greater id, return 1:  '+' number
//  --> If none of the above worked, we consider B > A and we      return -1: '-' number
int records_compare(record A, record B) {
    assert(&A != NULL);
    assert(&B != NULL);
    assert(A.surname != NULL && A.name != NULL && A.postal_code != NULL);
    assert(B.surname != NULL && B.name != NULL && B.postal_code != NULL);

    if(strcmp(A.surname, B.surname) > 0)
        return 1;
    if((strcmp(A.surname, B.surname) == 0) && (strcmp(A.name, B.name) > 0))
        return 1;
    if((strcmp(A.surname, B.surname) == 0) && (strcmp(A.name, B.name) == 0) && (A.id > B.id))
        return 1;
    return -1;
}

// A simple record print function with some allignment
void record_print(record R) {
    assert(&R != NULL);
    assert(R.surname != NULL && R.name != NULL && R.postal_code != NULL);
    
    printf("%-12s %-12s %-6d %s\n", R.surname, R.name, R.id, R.postal_code);
}

// Testing purposes
void record_print_pid(record R) {
    assert(&R != NULL);
    
    fprintf(stderr, "%-12s %-12s %-6d %s from process: [%d]\n", R.surname, R.name, R.id, R.postal_code, getpid());
}
