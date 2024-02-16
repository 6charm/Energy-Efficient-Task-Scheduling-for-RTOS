#include "../aAlloc.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Check that leak detector can detect one leaked allocation.

struct node {
    node* next;
};

int main() {
    node* list = nullptr;

    // create a list
    for (int i = 0; i != 400; ++i) {
        node* n = (node*) aAlloc_malloc(sizeof(node));
        n->next = list;
        list = n;
    }

    // free everything in it but one
    while (list && list->next) {
        node** pprev = &list;
        while (node* n = *pprev) {
            *pprev = n->next;
            aAlloc_free(n);
            if (*pprev) {
                pprev = &(*pprev)->next;
            }
        }
    }

    printf("EXPECTED LEAK: %p with size %zu\n", list, sizeof(node));
    aAlloc_print_leak_report();
}
