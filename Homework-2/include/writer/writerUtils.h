#pragma once

// Initializes the -f, -l, -v, -d, -s flags and makes some initial filtering
void writer_args_init(int argc, char** argv, int n_args, char** file_name, int* recid, int* value, int* time, char** shm_path);