#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../include/logs.h"
#include "../include/inputUtils.h"
#include "../include/record.h"
#include "../include/processUtils.h"

// Helper function for merge_result
// Given two sorted arrays of records: res1, res2 of size cap1 and cap2 respectively, it merges 
// those two arrays and returns the merged, sorted array of records of size cap1 + cap2 
static record* merge_pair(record* res1, int cap1, record* res2, int cap2, int file_code);

// Helper function for RunMerger
// Given an array of records arrays of uneven sizes, accompanied with their sizes in the
// capacities_array, it merges the records arrays by pairs and returns the resulting, 
// merged and sorted array of records, of size:
//
//                  Sum (from i = 0 to n_children) { capacities_array[i] }
//
static record* merge_results(record** records_array, int* capacities_array, int n_children, int file_code);


void pipes_init(int*** pipes, int n_children, int file_code) {
    *pipes = malloc(sizeof(int *) * n_children);
    if(*pipes == NULL) {
        ERROR_MALLOC(file_code);
        free(*pipes);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n_children; i++) {

        (*pipes)[i] = malloc(sizeof(int) * 2);
        if( ( (*pipes)[i] ) == NULL ) {
            ERROR_MALLOC(file_code);
            pipes_destroy(*pipes, i);
            exit(EXIT_FAILURE);
        }

        if (pipe((*pipes)[i]) < 0) {
            perror("Error in pipe cretion\n");
            pipes_destroy(*pipes, i);
            exit(EXIT_FAILURE);
        }
    }
}

void pipes_destroy(int** pipes, int n_children) {
    for (int i = 0; i < n_children; i++)
        free(pipes[i]);
    free(pipes);
}

void prepare_pipe(int** pipe, int child) {
    close(pipe[child][READ]);                    // Close valve
    dup2(pipe[child][WRITE], STDOUT_FILENO);     // Duplicate STDOUT of child
    close(pipe[child][WRITE]);                   // Close the original write end
}

void children_args_init(child_args** children_args_array, int n_children, int file_code) {
    (*children_args_array) = malloc(sizeof(child_args) * n_children);
    if( (*children_args_array) == NULL) {
        ERROR_MALLOC(file_code);
        exit(EXIT_FAILURE);
    }
}

void children_args_destroy(child_args* children_args) {
    free(children_args);
}

void wait_children(child_args* children_args, int n_children, int file_code) {
    assert(children_args != NULL);
    assert(n_children > 0);

    for(int child = 0; child < n_children; child++) {
        int status;
        // wait for child
        wait(&status);
        if(status == -1) {
            ERROR_WAIT(file_code, status, child, children_args[child].pid);
            exit(EXIT_FAILURE);
        }

        LOG_WAIT(file_code, status, child, children_args[child].pid);
    }
}

void set_child_record_bounds(int child, int* child_start, int start, int* child_capacity, int parent_capacity, int n_children, int sum_n_records) {
    *child_start =  start;
    if(child != n_children - 1)
        *child_capacity = parent_capacity / n_children;
    else
        *child_capacity = parent_capacity - sum_n_records;
}

// A wrapper function for read(), it iteratively reads bytes from 
// fd into dest, until all n_bytes are succesfully read
void Read(int fd, void* dest, size_t n_bytes, int file_code) {

    long bytes_left = (long) n_bytes;
    char* dest_ptr = dest;
    ssize_t bytes_read = 0;

    // As long as we have more bytes to read to reach n_bytes
    while(bytes_left > 0) {
        
        // Try to read 'bytes_left' bytes from fd and store them where the dest_ptr points. 'bytes_read'
        // stores how many bytes read() actually read in this iteration
        bytes_read = read(fd, dest_ptr, bytes_left);

        if(bytes_read == -1) {                          // If read returns with failure
            ERROR_READ(file_code);
            exit(EXIT_FAILURE);
        }
        bytes_left -= bytes_read;                       // Update 'bytes_left'
        dest_ptr += bytes_read;                         // Move the dest_ptr 'bytes_read' bytes forward
    }
}

void RunSplitter(int child, int** pipe_r, child_args* children_args, int file_code, pid_t child_pid) {
    LOG_FORK(file_code, child_pid);
    children_args[child].pid = child_pid;
    close(pipe_r[child][WRITE]);                   // Close valve
}

record* merge_results(record** records_array, int* capacities_array, int n_children, int file_code) {

    // We can safely just use memcpy for the first records array to initialize our result
    record* final_result = malloc(sizeof(record) * capacities_array[0]);
    if(final_result == NULL) {
        ERROR_MALLOC(file_code);
        exit(EXIT_FAILURE);
    }
    memcpy(final_result, records_array[0], sizeof(record) * capacities_array[0]);

    int capacity_so_far = capacities_array[0];                  // Number of records in result

    record* temp_result;                                        // In order to store result of merge_pair temporarily
    for (int child = 1; child < n_children; child++) {          // Starting from index 1

        if(capacities_array[child] == 0) continue;

        temp_result = merge_pair(final_result, capacity_so_far, records_array[child], capacities_array[child], file_code);
        free(final_result);                             // Free old result
        final_result = temp_result;                     // Take new result

        capacity_so_far += capacities_array[child];     // Increase number of records in result
    }

    return final_result;
}

// The logic in this function is pretty simple
record* merge_pair(record* res1, int cap1, record* res2, int cap2, int file_code) {
    assert(res1 != NULL);
    assert(res2 != NULL);

    int i = 0;
    int j = 0;
    int k = 0;
    record* result = malloc((cap1 + cap2) * sizeof(record));
    if(result == NULL) {
        ERROR_MALLOC(file_code);
        exit(EXIT_FAILURE);
    }

    // Whichever index i, j reaches its array's size first
    while (i < cap1 && j < cap2) {
        if (records_compare(res2[j], res1[i]) > 0)
            result[k++] = res1[i++];
        else
            result[k++] = res2[j++];
    }

    // If res1 array has still got more records
    while (i < cap1)
        result[k++] = res1[i++];

    // Else if res2 array has still got more records
    while (j < cap2)
        result[k++] = res2[j++];

    return result;    
}

record* RunMerger(int n_children, child_args* children_args, int** pipe_r, int file_code) {

    // Create the array that shall hold the number of records of each result array of children processes
    int* capacities_array = malloc(sizeof(int) * n_children);
    if(capacities_array == NULL) {
        ERROR_MALLOC(file_code);
        exit(EXIT_FAILURE);
    }

    // Create the array that shall hold pointers to the result arrays of children processes
    record** records_array = calloc(n_children, sizeof(record*));
    if(records_array == NULL) {
        ERROR_MALLOC(file_code);
        exit(EXIT_FAILURE);
    }

    // Read results from children, one by one
    for(int child = 0; child < n_children; child++) {

        children_args[child].result = NULL;
        if(children_args[child].capacity != 0) {
            
            children_args[child].result = calloc(children_args[child].capacity, sizeof(record));
            if(children_args[child].result == NULL) {
                ERROR_MALLOC(file_code);
                exit(EXIT_FAILURE);
            }
        }

        if(children_args[child].capacity != 0)
            // Read results from children processes
            Read(pipe_r[child][READ], children_args[child].result,  children_args[child].capacity * sizeof(record), file_code);
        
        records_array[child]    = children_args[child].result;
        capacities_array[child] = children_args[child].capacity;
    }

    // Merge and form the final result from the children processes
    record* result = merge_results(records_array, capacities_array, n_children, file_code);

    // Cleanup
    for(int child = 0; child < n_children; child++) {
        if(children_args[child].capacity != 0)
            free(children_args[child].result);
    }
    free(records_array);
    free(capacities_array);

    return result;
}
