#pragma once
// ------------------------------ INPUT UTILS ------------------------------ //

// Helper functions for parsing arguments in command line
void str_init(int argc, char** argv, char** file_name, char flag, int* i);
void num_init(int argc, char** argv, int* num, char flag, int* i, int option);

// Converts a given positive number: [num] to a string and returns this string 
// (works like a reversed atoi)
char* itoa(size_t num);

bool validNumberOfArgs(int argc, int total_args);

// Cleanup function for initial arguments
void free_args(char* file_name1, char* file_name2);