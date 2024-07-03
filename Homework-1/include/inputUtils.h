#pragma once
#include <stdbool.h>

// ------------------------------ INPUT UTILS ------------------------------ //

// Initializes the -i, -k, -e1, -e2 flags and makes some initial filtering
bool validArgs(int argc, char** argv, char** DataFile, int* NumofChildren, char** sorting1, char** sorting2, int max_args);

// Converts a given positive number to a string (something similar to a reverse of atoi)
char* itoa(size_t num);

// Cleanup function for initial arguments
void freeInput(char* file_name, char* s1, char* s2);
