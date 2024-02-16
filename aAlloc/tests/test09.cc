#include "../aAlloc.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Check invalid free heap pointer
// Attempt to free the allocated region using 
// a pointer that was not malloc'd

int main() {
    void* ptr = aAlloc_malloc(2001);
    fprintf(stderr, "Bad pointer %p\n", (char*) ptr + 128);
    aAlloc_free((char*) ptr + 128);
    aAlloc_print_statistics();
}
