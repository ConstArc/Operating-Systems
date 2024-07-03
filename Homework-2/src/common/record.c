#include "../../include/common/global.h"
#include "../../include/common/record.h"

// A simple record print function with some formatting
void record_print(record R, pid_t pid, int option) {
    assert(&R != NULL);
    assert(R.surname != NULL && R.name != NULL);
    assert(option == 0 || option == 1);

    fflush(stdout);
    if(option == 0)
        printf("[%d] | READER | %-12s %-12s %-6d %-6d\n", pid, R.surname, R.name, R.id, R.balance);
    if(option == 1)
        printf("[%d] | WRITER | %-12s %-12s %-6d %-6d\n", pid, R.surname, R.name, R.id, R.balance);
}

void record_balance_update(record* R, int diff) {
    assert(R != NULL);
    assert(R->surname != NULL && R->name != NULL);
    
    R->balance += diff;
}

int record_get_balance(record R) {
    assert(&R != NULL);

    return R.balance;
}