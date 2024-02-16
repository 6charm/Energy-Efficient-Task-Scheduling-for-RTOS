#include "../aAlloc.hh"
#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>
#include <random>
// show that small blocks of memory can be coalesced into larger pieces.
// check free-list coalescing

int main() {
    const size_t nmax = 7168;
    void* ptrs[nmax];
    size_t n = 0;
    while (n != nmax) {
        ptrs[n] = aAlloc_malloc(850);
        assert(ptrs[n]);
        ++n;
    }
    // ensure smallest address is in `ptrs[0]`
    std::sort(ptrs, ptrs + n);

    std::random_device rd;
    std::mt19937 gen(rd());

    while (n != 1) {
        std::uniform_int_distribution<> size_distr(1, n-1);
        size_t i = size_distr(gen);
        aAlloc_free(ptrs[i]);
        ptrs[i] = ptrs[n - 1];
        --n;
    }

    void* bigptr = aAlloc_malloc(6091950);
    assert(bigptr);
    aAlloc_free(bigptr);

    aAlloc_statistics stat = aAlloc_get_statistics();
    assert(reinterpret_cast<uintptr_t>(bigptr) >= stat.heap_min);
    assert(reinterpret_cast<uintptr_t>(bigptr) + 6091949 <= stat.heap_max);

    aAlloc_print_statistics();
}

// alloc count: active          1   total       7169   fail          0
// alloc size:  active        ???   total   12184750   fail          0
