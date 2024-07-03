#pragma once
#include "../common/record.h"

// Initializes the -f, -l, -d, -s flags and makes some initial filtering
void reader_args_init(int argc, char** argv, int max_n_args, char** file_name, int* rec_start, int* rec_finish, int* time, char** shm_path);

// Prints the [records] array of records and initializes the [sum_balance]
void reader_print_records(int* sum_balance, int capacity, record* records);

// Prints the average of balances of the records read, with the
// help of [sum_balance] and [capacity]
void reader_print_avg_balance(int sum_balance, int capacity);

// Initializes [capacity], that is the number of records within the [wanted_range]
void reader_capacity_init(int* capacity, int finish, int start);
