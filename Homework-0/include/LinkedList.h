#pragma once
#include <stdbool.h>
#include "Global.h"

typedef struct list* List;

typedef int (*CompareFunc)(const Pointer Item_A, const Pointer Item_B);
typedef void (*DestroyFunc)(const Pointer Item);
typedef void (*PrintFunc)(const Pointer Item);

// ------------------------------ LINKED LIST ------------------------------ //

// Creates a list
List list_create(CompareFunc compare_foo, DestroyFunc destroy_foo, PrintFunc print_foo);

// Adds an Item at the end of the list
void list_append(const List list, const Pointer Item);

// Searches for Item in the list
Pointer list_search(const List list, const Pointer Item);

// Removes the first item of the list
Pointer list_remove(const List list);

// Returns the size of the list
int list_size(const List list);

// Returns true if list is empty, else it returns false
bool is_list_empty(const List list);

// Destroys and frees allocated memory for the list and its nodes,
// BUT not of the items within it. Use this function with great
// care to avoid memory leaks.
void list_destroy(const List list, bool count_bytes);

// A simple print function
void list_print(const List list);

// If we treat the list as an indexed list, add an Item at exactly
// the next position of index
void list_add(const List list, const Pointer Item, int index);
