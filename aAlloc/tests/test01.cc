#include "../aAlloc.hh"
#include <cstdio>
// show that allocator tracks active allocation count
// accurately

int main() {
    //puts("here");
    void* ptrs[10];
    for (int i = 0; i != 10; ++i) {
        //puts("here1");
        ptrs[i] = aAlloc_malloc(i + 1);
    }
    for (int i = 0; i != 5; ++i) {
        //puts("here2");
        aAlloc_free(ptrs[i]);
    }
    aAlloc_print_statistics();
}

// alloc count: active          5   total         10   fail        ???
// alloc size:  active        ???   total        ???   fail        ???