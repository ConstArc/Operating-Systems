#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/LinkedList.h"

// ------------------------------ STRUCTS ------------------------------ //
typedef struct ListNode{
    Pointer value;
    struct ListNode* next;
} ListNode;

typedef struct list {
    ListNode* head;
    ListNode* tail;
    int n_elements;
    CompareFunc cmp_func;
    DestroyFunc destroy_func;
    PrintFunc print_func;
} list;


// ------------------------------ LIST ------------------------------ //
List list_create(CompareFunc cmp_func, DestroyFunc destroy_func, PrintFunc print_func) {
    List L = malloc(sizeof(list));
    if(L == NULL) {
        fprintf(stderr, "Error: Memory allocation failure | While allocating memory for: list.\n");
        exit(EXIT_FAILURE);
    }

    L->head = NULL;
    L->tail = NULL;
    L->n_elements = 0;

    L->cmp_func = cmp_func;
    L->destroy_func = destroy_func;
    L->print_func = print_func;

    return L;
}

int list_size(const List L) {
    assert(L != NULL);

    return L->n_elements;
}

bool is_list_empty(const List L) {
    assert(L != NULL);

    return (L->head == NULL);
}

void list_append(const List L, const Pointer value) {
    assert(L != NULL);
    
    ListNode* new_node = malloc(sizeof(ListNode));
    assert(new_node != NULL);

    new_node->value = value;
    new_node->next = NULL;

    if(!is_list_empty(L)) {
        L->tail->next = new_node;
        L->tail = new_node;
    }
    else {
        L->head = new_node;
        L->tail = L->head;
    }

    L->n_elements++;
}

Pointer list_search(const List L, const Pointer P) {
    assert(L != NULL);

    ListNode* temp = L->head;
    while(temp != NULL) {
        if(L->cmp_func(temp->value, P) == 0)
            return temp;
    }
    return NULL;
}

Pointer list_remove(const List L) {
    assert(L != NULL);

    ListNode* removed = L->head;
    Pointer removed_value = removed->value;
    L->head = L->head->next;
    free(removed);

    L->n_elements--;
    return removed_value;
}

void list_add(const List L, const Pointer value, const int index) {
    assert(L != NULL);

    if(index >= list_size(L) - 1) {
        list_append(L, value);
    }
    else {
        ListNode* new_node = malloc(sizeof(ListNode));
        assert(new_node != NULL);

        new_node->value = value;
        new_node->next = NULL;
        ListNode* temp = L->head;
        int counter = 0;
        while(counter < index) {
            temp = temp->next;
            counter++;
        }
        new_node->next = temp->next;
        temp->next = new_node;
    }
    L->n_elements++;
}


// This is a function that only destroys the list and frees allocated space for
// the head, ListNodes, NOT for the items within them. Usage of this function
// should be very precise and with great care, in order to not create any memory
// leaks.
void list_destroy(List L, bool count_bytes) {
    if(L == NULL)
        return;

    ListNode* temp = L->head;
    ListNode* temp_before = L->head;

    while(temp != NULL) {
        temp = temp->next;
        free(temp_before);
        if(count_bytes)
            bytes_freed += sizeof(ListNode);
        temp_before = temp;
    }

    free(temp_before);
    if(count_bytes)
        bytes_freed += sizeof(ListNode);

    free(L);
    L = NULL;
    if(count_bytes)
        bytes_freed += sizeof(list);
}

void list_print(const List L) {
    assert(L != NULL);
    
    ListNode* temp = L->head;
    while(temp != NULL) {
        L->print_func(temp->value);
        temp = temp->next;
    }
}
