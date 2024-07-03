#pragma once
#include "Global.h"

// The function that initializes the database and runs it, for as long as the program runs
void RunDB(char* file_path, int m, int bucket_size, float LOAD_THRESHOLD);
