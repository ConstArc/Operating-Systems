#pragma once

typedef struct {
    int  id;
    char surname[20];
    char name[20];
    int  balance;
} record;


// A simple print function for record R
void record_print(record R, pid_t pid, int option);

// Returns the balance of record R
int record_get_balance(record R);

// Updates the balance of record R by diff
void record_balance_update(record* R, int diff);