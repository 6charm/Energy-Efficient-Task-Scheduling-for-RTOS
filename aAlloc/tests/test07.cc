#include "../aAlloc.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Check invalid free of stack pointer.

int main() {
    int x;
    aAlloc_free(&x);
    aAlloc_print_statistics();
}

