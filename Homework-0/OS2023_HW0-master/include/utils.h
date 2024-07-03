#pragma once
#include <stdbool.h>

// ------------------------------ UTILS ------------------------------ //

// Initializes the -b, -m, -f flags and makes some initial filtering
bool validArgs(int argc, char** argv, char** file_name, int* bucket_size, int* m, int max_args);

// Trims the \n character of a string
void trimInput(char* input);

// Counts the number of words within a string (delimeter is whitespace only)
int tokenCount(const char* token);

// Clears the buffer
void clearInput(void);

// Checks if a string is a positive integer number
bool isPositiveIntegerNumber(const char* str);
