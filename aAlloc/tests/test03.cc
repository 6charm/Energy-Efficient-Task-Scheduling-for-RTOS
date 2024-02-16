#include "../aAlloc.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
#include <random>
// show taht aAlloc_calloc works when called ~500 times.

int main() {

    // Ref: https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
    std::random_device rd;
    std::mt19937 gen(rd());
    
    constexpr int nptrs = 5;
    char* ptrs[nptrs] = {nullptr, nullptr, nullptr, nullptr, nullptr};

    // do ~10000 allocations and aAlloc_frees, checking that each allocation
    // has zeroed contents.
    for (int round = 0; round != 1000; ++round) {
        std::uniform_int_distribution<> index_distr(0, nptrs - 1);
        int index = index_distr(gen);
        if (!ptrs[index]) {
            // Allocate a new randomly-sized block of memory
            std::uniform_int_distribution<> size_distr(1, 2000);
            size_t size = size_distr(gen);
            char* p = (char*) aAlloc_calloc(size, 1);
            assert(p != nullptr);
            // check contents
            size_t i = 0;
            while (i != size && p[i] == 0) {
                ++i;
            }
            assert(i == size);
            // set to non-zero contents and save
            memset(p, 'A', size);
            ptrs[index] = p;

        } else {
            // Free previously-allocated block
            aAlloc_free(ptrs[index]);
            ptrs[index] = nullptr;
        }
    }

    for (int i = 0; i != nptrs; ++i) {
        aAlloc_free(ptrs[i]);
    }

    aAlloc_print_statistics();
}

// alloc count: active          0   total  ??>=500??   fail          0
// alloc size:  active        ???   total        ???   fail          0
