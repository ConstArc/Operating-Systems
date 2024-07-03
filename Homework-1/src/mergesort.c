#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <sys/times.h>             /* times () */
#include <unistd.h>                /* sysconf () */
#include <fcntl.h>
#include <signal.h>
#include "../include/record.h"
#include "../include/processUtils.h"
#include "../include/logs.h"

#define FILE_CODE 3         // Logging purposes
#define TOTAL_N_ARGS 5      // Validation of argv

static void merge_sort(record* records_array, int start, int finish);
static void merge(record* array_A, int left, int middle, int right);


int main(int argc, char* argv[]) {

    if(argc != TOTAL_N_ARGS) {
        perror("Wrong number of arguments passed to sorter\n");
        exit(EXIT_FAILURE);
    }

    // Begin time measurements
    double t1 , t2 , cpu_time ;
    struct tms tb1 , tb2 ;
    double ticspersec ;
    ticspersec = (double)sysconf(_SC_CLK_TCK);
    t1 = (double)times(&tb1) ;

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

    // Calling mergesort here
    merge_sort(records, 0, capacity);

    // Send result to parent process (workloader)
    write(STDOUT_FILENO, records, n_bytes);
    if(n_bytes != 0)
        free(records);

    // Finish time measurements
    t2 = (double)times(&tb2);
    cpu_time = (double) ( (double) ((tb2.tms_utime + tb2.tms_stime) - (tb1.tms_utime + tb1.tms_stime)) / ticspersec);
    double real_time = (t2-t1)/ticspersec;

    // Write time measurements (first the cpu time then the real time)
    write(STDOUT_FILENO, &cpu_time, sizeof(double));
    write(STDOUT_FILENO, &real_time, sizeof(double));

    // Send signal to coordinator letting them know that we have finished succesfully
    kill(coordinator_pid, SIGUSR2);
    exit(EXIT_SUCCESS);
}

// ------------------ Utilities of mergesort  ------------------ //
void merge_sort(record* records_array, int start, int finish) {
    assert(records_array != NULL);

    // Base case: records_array has 0 or 1 elements, no sorting needed
    if ( finish - start <= 1) {
        return;
    }

    int mid = (start + finish)/2;

    merge_sort(records_array, start, mid);      // Sort the left  subarray ( records_array[start - mid] )
    merge_sort(records_array, mid, finish);     // Sort the right subarray ( records_array[mid - finish] )

    merge(records_array, start, mid, finish);   // Merge the two subarrays
}

void merge(record* array_A, int left, int middle, int right) {
    assert(array_A != NULL);

    int i = left;
    int j = middle;

    record* array_B = malloc(sizeof(record)*(right - left));
    if(array_B == NULL) {
        ERROR_MALLOC(FILE_CODE);
        exit(EXIT_FAILURE);
    }

    // While there are elements in the left or right subarray
    for (int k = left; k < right; k++) {
        if (i < middle && (j >= right || records_compare(array_A[i], array_A[j]) < 0)) {
            array_B[k - left] = array_A[i];
            i++;
        } 
        else {
            array_B[k - left] = array_A[j];
            j++;
        }
    }

    // Copy the merged elements from array_B to the original array, array_A
    for (int k = left; k < right; k++) {
        array_A[k] = array_B[k - left];
    }
    
    free(array_B);          // Cleanup
}
