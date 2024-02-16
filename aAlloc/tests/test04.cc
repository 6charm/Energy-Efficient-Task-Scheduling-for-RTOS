#include "../aAlloc.hh"
#include <cstdio>
#include <cassert>
#include <vector>
// show memory reuse occurs.
// sum of allocs >> buffer size. Hence succesful run implies memory reuse.
//8388608 bytes = total default buffer size. 8322000 8323000
int main() {
    for (int i = 0; i != 10000; ++i) {
        void* ptr = aAlloc_malloc(1000);
        assert(ptr);
        aAlloc_free(ptr);
    }
    aAlloc_print_statistics();
}

// alloc count: active          0   total      10000   fail          0
// alloc size:  active        ???   total   10000000   fail          0
