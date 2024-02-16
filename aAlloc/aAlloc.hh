#ifndef aAlloc_HH
#define aAlloc_HH 1
#include <cassert>
#include <cstdlib>
#include <cinttypes>
#include <cstdio>
#include <new>
#include <random>


/// aAlloc_malloc(sz, file, line)
///    Return a pointer to `sz` bytes of newly-allocated dynamic memory.
void* aAlloc_malloc(size_t sz, const char* file = __builtin_FILE(), int line = __builtin_LINE());

/// aAlloc_free(ptr, file, line)
///    Free the memory space pointed to by `ptr`.
void aAlloc_free(void* ptr, const char* file = __builtin_FILE(), int line = __builtin_LINE());

/// aAlloc_calloc(count, sz, file, line)
///    Return a pointer to newly-allocated dynamic memory big enough to
///    hold an array of `count` elements of `sz` bytes each. The memory
///    is initialized to zero.
void* aAlloc_calloc(size_t count, size_t sz, const char* file = __builtin_FILE(), int line = __builtin_LINE());


/// aAlloc_statistics
///    Structure tracking memory statistics.
struct aAlloc_statistics {
    unsigned long long nactive;         // # active allocations
    unsigned long long active_size;     // # bytes in active allocations
    unsigned long long ntotal;          // # total allocations
    unsigned long long total_size;      // # bytes in total allocations
    unsigned long long nfail;           // # failed allocation attempts
    unsigned long long fail_size;       // # bytes in failed alloc attempts
    uintptr_t heap_min;                 // smallest allocated addr
    uintptr_t heap_max;                 // largest allocated addr
};

/// aAlloc_get_statistics()
///    Return the current memory statistics.
aAlloc_statistics aAlloc_get_statistics();

/// aAlloc_print_statistics()
///    Print the current memory statistics.
void aAlloc_print_statistics();

/// aAlloc_print_leak_report()
///    Print a report of all currently-active allocated blocks of dynamic
///    memory.
void aAlloc_print_leak_report();

void* aAlloc_find_free_space(size_t sz);

#endif
