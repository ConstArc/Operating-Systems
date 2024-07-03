#pragma once


// A wrapper function for read(). It iteratively stores bytes from 
// [fd] into [dest], until all [n_bytes] are succesfully read
void Read(int fd, void* dest, size_t n_bytes);

// Generates a random number within the interval [max_num, min_num].
// It is the caller's responsibility to seed the random number
// generator (srand), before calling this function
int gen_num(int max_num, int min_num);

// Opens the file with name [file_name] and with the flag of [intention]. 
// The return value of open is assigned to [*fd]. Lastly, it uses lseek to
// reposition the file offset by [offset]
void prepare_file(int* fd, char* file_name, int offset, int intention);

// Closes the file with [fd]
void shut_file(int fd);

// Starts the timer by initializing [start] with current time
void start_timer(timer* start);

// Returns the elapsed time from [start] to current time
double elapsed_time(timer start);
