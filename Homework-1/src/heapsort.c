#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/times.h>             /* times () */
#include <unistd.h>                /* sysconf () */
#include <signal.h>
#include "../include/record.h"
#include "../include/logs.h"
#include "../include/processUtils.h"

#define FILE_CODE 4         // Logging purposes
#define TOTAL_N_ARGS 5      // Validation of argv

static void heapify(record* records_array, int size, int index);
static void heap_sort(record* records_array, int size);


int main(int argc, char* argv[]) {

    if(argc != TOTAL_N_ARGS) {
        perror("Wrong number of arguments passed to sorter\n");
        exit(EXIT_FAILURE);
    }

    // Begin time measurements
    double t1, t2, cpu_time;
    struct tms tb1, tb2;
    double ticspersec;
    ticspersec = (double)sysconf(_SC_CLK_TCK);
    t1 = (double)times(&tb1);

    // Saving arguments to variables
    char* data_file       = argv[1];
    int   start           = atoi(argv[2]);
    int   capacity        = atoi(argv[3]);
    pid_t coordinator_pid = atoi(argv[4]);

    // Open, read and close data_file
    int fd = open(data_file, O_RDONLY);
    if (fd == -1) {
        ERROR_OPEN(FILE_CODE);
        exit(EXIT_FAILURE);
    }
    LOG_OPEN(FILE_CODE);

    // Move fd's file position to start*sizeof(record) bytes
    // from the beginning of the file. Basically, go at the 
    // designated (from the workloader) start of the file
    lseek(fd, start * sizeof(record), SEEK_SET);

    size_t n_bytes = sizeof(record) * capacity;
    record* records = malloc(n_bytes);  
    Read(fd, records, n_bytes, FILE_CODE);

    close(fd);

    // Calling heapsort here
    heap_sort(records, capacity);

    // Send result to parent process (workloader)
    write(STDOUT_FILENO, records, n_bytes);
    if(n_bytes != 0)
        free(records);

    // Finish time measurements
    t2 = (double) times (&tb2);
    cpu_time = (double) ( (double) ((tb2.tms_utime + tb2.tms_stime) - (tb1.tms_utime + tb1.tms_stime)) / ticspersec);
    double real_time = (t2-t1)/ticspersec;

    // Write time measurements (first the cpu time then the real time)
    write(STDOUT_FILENO, &cpu_time, sizeof(double));
    write(STDOUT_FILENO, &real_time, sizeof(double));

    // Send signal to coordinator letting them know that we have finished succesfully
    kill(coordinator_pid, SIGUSR2);
    exit(EXIT_SUCCESS);
}

// ----------------- Utilities of heapsort  ----------------- //
// Maintains the heap property
void heapify(record* records_array, int size, int index) {
    assert(records_array != NULL);

    int largest = index;
    int left_child = 2 * index + 1;
    int right_child = 2 * index + 2;

    if (left_child < size && records_compare(records_array[left_child], records_array[largest]) > 0)
        largest = left_child;
    if (right_child < size && records_compare(records_array[right_child], records_array[largest]) > 0)
        largest = right_child;

    if (largest != index) {
        records_swap(&(records_array[index]), &(records_array[largest]));
        heapify(records_array, size, largest);
    }
}

void heap_sort(record* records_array, int size) {
    assert(records_array != NULL);

    // Building the max-heap
    for (int i = size/2-1; i >= 0; i--) {
        heapify(records_array, size, i);
    }

    // Sorting the array
    for (int i = size-1; i >= 0; i--) {
        records_swap(&(records_array[0]), &(records_array[i]));
        heapify(records_array, i, 0);
    }
}
