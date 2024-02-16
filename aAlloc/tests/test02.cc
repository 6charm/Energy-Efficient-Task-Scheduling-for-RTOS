#include "../aAlloc.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cinttypes>
#include <cstddef>
// Series of assertions that check alignment of 
// allocd pointer. Based on standard type alignments..

int main() {
    double* ptr = (double*) aAlloc_malloc(sizeof(double));
    assert((uintptr_t) ptr % alignof(double) == 0);
    assert((uintptr_t) ptr % alignof(unsigned long long) == 0);
    assert((uintptr_t) ptr % alignof(std::max_align_t) == 0);

    char* ptr2 = (char*) aAlloc_malloc(1);
    assert((uintptr_t) ptr2 % alignof(double) == 0); //8 byte
    assert((uintptr_t) ptr2 % alignof(unsigned long long) == 0);
    assert((uintptr_t) ptr2 % alignof(std::max_align_t) == 0);

    aAlloc_free(ptr);
    aAlloc_free(ptr2);
}
