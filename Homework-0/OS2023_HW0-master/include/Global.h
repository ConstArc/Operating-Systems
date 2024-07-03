#pragma once

// A global variable in which we are going to store and print, at the normal exit
// of the program, the bytes allocated dynamically that we are freeing at that moment 
// (the moment of the exit).  By normal exit we mean an exit triggered from the user.
extern size_t bytes_freed;

// For generic purpose, used in LinkedList
typedef void* Pointer;

// The basic item struct of the database, that we are storing within it
typedef struct voter* Voter;
