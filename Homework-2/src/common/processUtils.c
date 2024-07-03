#include "../../include/common/global.h"
#include "../../include/common/processUtils.h"

void Read(int fd, void* dest, size_t n_bytes) {

    long bytes_left = (long) n_bytes;
    char* dest_ptr = dest;
    ssize_t bytes_read = 0;

    // As long as we have more bytes to read to reach n_bytes
    while(bytes_left > 0) {
        
        // Try to read 'bytes_left' bytes from fd and store them where the dest_ptr points. 'bytes_read'
        // stores how many bytes read() actually read in this iteration
        bytes_read = read(fd, dest_ptr, bytes_left);

        if(bytes_read == -1) {                         // If read returns with failure
            perror("read");
            exit(EXIT_FAILURE);
        }
        bytes_left -= bytes_read;                       // Update 'bytes_left'
        dest_ptr += bytes_read;                         // Move the dest_ptr 'bytes_read' bytes forward
    }
}

void prepare_file(int* fd, char* file_name, int offset, int intention) {
    *fd = open(file_name, intention);
    if (*fd == -1)
        errExit("open");

    if (lseek(*fd, offset, SEEK_SET) == -1)
        errExit("lseek");
}

void shut_file(int fd) {
    if(close(fd) == -1)
		errExit("close");
}

int gen_num(int max_num, int min_num) {
	return rand() % (max_num + 1 - min_num) + min_num;
}

void start_timer(timer* start) {
    gettimeofday(start, NULL);
}

double elapsed_time(timer start) {
    timer current;
    gettimeofday(&current, NULL);

    return (current.tv_sec - start.tv_sec) +
           (current.tv_usec - start.tv_usec) / 1e6;
}