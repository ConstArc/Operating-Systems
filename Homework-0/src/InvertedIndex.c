#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/Voter.h"
#include "../include/InvertedIndex.h"
#include "../include/LinkedList.h"


// ------------------------------ STRUCTS ------------------------------ //
typedef struct zipcode_node* Zip;

typedef struct zipcode_node {
    int  postal_code;
    List voters_list;
    int  n_voters;
    Zip  next;
    Zip  back;
} zipcode_node;


typedef struct invterted_index {
    Zip zipcodes_list_head;
    Zip zipcodes_list_tail;
    int n_zipcodes;
} invterted_index;


// ------------------------------ ZIP NODE ------------------------------ //

// A constructor for the zipnode, given a postal_code
static Zip zipnode_create(int postal_code) {
    Zip Z = malloc(sizeof(zipcode_node));
    if(Z == NULL) {
        fprintf(stderr, "Error: Memory allocation failure | While allocating memory for: zipcode_node.\n");
        exit(EXIT_FAILURE);
    }

    Z->postal_code = postal_code;
    Z->voters_list = list_create(NULL, NULL, voter_print_pin);
    Z->n_voters    = 0;
    Z->next        = NULL;
    Z->back        = NULL;

    return Z;
}

// Equality between two zipnodes: Z1 and Z2, means that the postal code of 
// Z1 is the same with the postal code of Z2
static bool zipnodes_equal(const Zip Z1, const Zip Z2) {
    assert(Z1 != NULL);
    assert(Z2 != NULL);

    if(Z1->postal_code == Z2->postal_code)
        return true;
    
    return false;
}

// Based on diff:
//      Positive (+): Z1's number of voters is bigger than Z2's.   Returns  1
//      Negative (-): Z1's number of voters is smaller than Z2's.  Returns -1
//      Zero     (0): Z1's number of voters is the same with Z2's. Returns  0
static int zipnodes_cmp_n_voters(const Zip Z1, const Zip Z2) {
    assert(Z1 != NULL);
    assert(Z2 != NULL);

    int diff = (Z1->n_voters) - (Z2->n_voters);

    if(diff > 0)
        return 1;

    if(diff < 0)
        return -1;
    
    return 0;
}

// A simple search of zipnode with postal_code = zipcode
static Zip zipcodes_list_search_zipnode(const Zip Z, int zipcode) {
    Zip temp = Z;
    while(temp != NULL) {
        if(temp->postal_code == zipcode)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

// This is a function that implements sorted insert of Z, it works like this:
// Starting from start (which will be Z->back), iterate backwards (towards the head).
// At the first time a zipnode is found with greater or equal number of voters than Z,
// say zipnode temp, perform the insertion of Z in-between temp and temp->next. If the
// iteration ends without performing the insertion that means we have reached the head
// and in this case once inserting Z at the back of the head, we shall update the list's
// head as well.
static void zipcodes_list_sorted_insert_left(Zip* head, Zip start, Zip Z) {
    assert(start != NULL);
    assert(Z != NULL);

    Zip temp = start;
    Z->back = NULL;                         // we no longer need the Z->back pointer since we have temp pointer
    while(temp->back != NULL) {
        if(zipnodes_cmp_n_voters(temp, Z) >= 0) {           // At the first time a zipnode with greater or equal
            Z->next = temp->next;                           // number of voters is found
            Z->next->back = Z;                              // Place Z in front of it (the temp node), whilst
            temp->next = Z;                                 // maintaining the general structure
            Z->back = temp;
            return;
        }
        temp = temp->back;
    }

    // We have reached the head node
    if(zipnodes_cmp_n_voters(temp, Z) >= 0) {
        temp->next->back = Z;
        Z->next = temp->next;
        Z->back = temp;
        temp->next = Z;
    }
    else {
        temp->back = Z;
        Z->next = temp;
        *head = Z;                                              // we have to update the head
    }
}


// This function simply inserts a voter into the voters_list of
// a particular zipnode Z
static void zipnode_insert(const Zip Z, const Voter V) {
    assert(Z != NULL);

    list_append(Z->voters_list, V);
    Z->n_voters++;
}

// Destroys the voters_list of Z (without the voters in it),
// frees allocated space and adds that space into bytes_freed
// global variable
static void zipnode_destroy(const Zip Z) {
    if(Z == NULL)
        return;

    list_destroy(Z->voters_list, true);

    free(Z);
    bytes_freed += sizeof(zipcode_node);
}

// ------------------------------ INVERTED INDEX ------------------------------ //
InvertedIndex inv_index_create(void) {
    InvertedIndex INV_INDEX = malloc(sizeof(invterted_index));
    if(INV_INDEX == NULL) {
        fprintf(stderr, "Error: Memory allocation failure | While allocating memory for: zipcode_node.\n");
        exit(EXIT_FAILURE);
    }

    INV_INDEX->n_zipcodes         = 0;
    INV_INDEX->zipcodes_list_head = NULL;
    INV_INDEX->zipcodes_list_tail = NULL;
    
    return INV_INDEX;
}

void inv_index_insert(const InvertedIndex INV_INDEX, const Voter V) {
    assert(INV_INDEX != NULL);
    assert(V != NULL);

    // If the voter V we are trying to insert hasn't voted yet,
    // then the insertion should not take place.
    if(!voter_has_voted(V))
        return;

    int zip = voter_get_zip(V);

    // In this case, zipcodes_list is empty
    if(INV_INDEX->zipcodes_list_head == NULL) {
        INV_INDEX->zipcodes_list_head = zipnode_create(zip);
        zipnode_insert(INV_INDEX->zipcodes_list_head, V);
        INV_INDEX->zipcodes_list_tail = INV_INDEX->zipcodes_list_head;
        return;
    }
    
    // Search for the zipnode
    Zip zipnode = zipcodes_list_search_zipnode(INV_INDEX->zipcodes_list_head, zip);

    // In this case, zipnode wasn't found, therefore
    // the new zipnode we are going to insert will 
    // have number of voters <= than the number of voters in head
    if(zipnode == NULL) {
        Zip new_zipnode = zipnode_create(zip);
        zipnode_insert(new_zipnode, V);
        INV_INDEX->zipcodes_list_tail->next = new_zipnode;
        new_zipnode->back = INV_INDEX->zipcodes_list_tail;
        INV_INDEX->zipcodes_list_tail = new_zipnode;
        return;
    }

    // This is an easy case, as we only have to insert the voter V
    // into the head node (the reason is pretty trivial)
    if(zipnodes_equal(INV_INDEX->zipcodes_list_head, zipnode)) {
        zipnode_insert(INV_INDEX->zipcodes_list_head, V);
        return;
    }

    // In this case, the zipnode is the same with the tail zipnode
    if(zipnodes_equal(INV_INDEX->zipcodes_list_tail, zipnode)) {
        zipnode_insert(INV_INDEX->zipcodes_list_tail, V);

        Zip before_tail = INV_INDEX->zipcodes_list_tail->back;
        // Before-tail node has fewer voters than the updated n_voters of tail
        if(zipnodes_cmp_n_voters(INV_INDEX->zipcodes_list_tail, before_tail) > 0) {
            before_tail->next = NULL;
            zipcodes_list_sorted_insert_left(&INV_INDEX->zipcodes_list_head, before_tail, INV_INDEX->zipcodes_list_tail);
            // Update the tail
            INV_INDEX->zipcodes_list_tail = before_tail;
        }
        return;
    }

    zipnode_insert(zipnode, V);
    if(zipnodes_cmp_n_voters(zipnode, zipnode->back) > 0) {
        // "removing" zipnode safely, maintaining the structure
        zipnode->back->next = zipnode->next;
        zipnode->next->back = zipnode->back;
        zipnode->next = NULL;

        zipcodes_list_sorted_insert_left(&INV_INDEX->zipcodes_list_head, zipnode->back, zipnode);
        return;
    }
}

void inv_index_n_voters_zipcode(const InvertedIndex INV_INDEX, int zipcode) {
    assert(INV_INDEX != NULL);

    Zip z = zipcodes_list_search_zipnode(INV_INDEX->zipcodes_list_head, zipcode);
    if(z != NULL) {
        printf("%d voted in %d\n", z->n_voters, zipcode);
        list_print(z->voters_list);
    }
}

void inv_index_zipcodes_n_voters(const InvertedIndex INV_INDEX) {
    assert(INV_INDEX != NULL);

    Zip temp = INV_INDEX->zipcodes_list_head;
    while(temp != NULL) {
        printf("%d %d\n", temp->postal_code, temp->n_voters);
        temp = temp->next;
    }
}

void inv_index_destroy(const InvertedIndex INV_INDEX) {
    if(INV_INDEX == NULL)
        return;
    
    Zip temp = INV_INDEX->zipcodes_list_head;
    Zip temp_before = INV_INDEX->zipcodes_list_head;

    while (temp != NULL) {
        temp = temp->next;
        zipnode_destroy(temp_before);
        temp_before = temp;
    }
    zipnode_destroy(temp_before);
    
    free(INV_INDEX);
    bytes_freed += sizeof(invterted_index);
}
