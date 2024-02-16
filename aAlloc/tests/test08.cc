#include "../aAlloc.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Check double free detection.

int main() {
    void* ptr = aAlloc_malloc(2001);
    fprintf(stderr, "Will free %p\n", ptr);
    aAlloc_free(ptr);
    aAlloc_free(ptr);
    aAlloc_print_statistics();
}