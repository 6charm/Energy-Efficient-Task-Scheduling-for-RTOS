#include "aAlloc.hh"
#include <iostream>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#include <cassert>
#include <sys/mman.h>
#include <vector>
#include <algorithm>
#include <cstddef>

struct aAlloc_memory_buffer {
    char* buffer;
    size_t pos = 0;
    size_t size = 8 << 20; /* 8 MiB */

    aAlloc_memory_buffer();
    ~aAlloc_memory_buffer();
};

struct allocation {
    char* start_ptr;
    char* end_ptr;
    const char* file;
    int line;
}allocation;

std::vector<struct allocation>active_allocations {0};
std::vector<struct allocation>freed_allocations {0};

bool compareByAddress(const struct allocation &a, const struct allocation &b)
{
    return a.start_ptr < b.start_ptr;
}

static aAlloc_memory_buffer default_buffer;

aAlloc_memory_buffer::aAlloc_memory_buffer() {
    void* buf = mmap(nullptr,    // Place the buffer at a random address
        this->size,              // Buffer should be 8 MiB big
        PROT_WRITE,              // We want to read and write the buffer
        MAP_ANON | MAP_PRIVATE, -1, 0);
                                 // want memory freshly allocated by the OS
    assert(buf != MAP_FAILED);
    this->buffer = (char*) buf;
}

aAlloc_memory_buffer::~aAlloc_memory_buffer() {
    munmap(this->buffer, this->size);
}

aAlloc_statistics stats = {0,0,0,0,0,0,0,0};

//called when default buffer full.
void* aAlloc_find_free_space(size_t sz){
    //check in freed list
    for (auto it = freed_allocations.begin(); it != freed_allocations.end(); ++it){
        size_t freed_sz = (size_t)((uintptr_t)it->end_ptr - (uintptr_t)it->start_ptr) +1;

        if(freed_sz >= sz){
            //can allocate. Empty space from a previous allocation. Default buffer not changed.
            void *ptr = (void *)(it->start_ptr); 
            size_t remaining_size = freed_sz - sz;

            if(remaining_size){//create a new freed block with remaining space, 
                struct allocation new_freed_alloc {};
                new_freed_alloc.start_ptr =it->start_ptr + sz;
                new_freed_alloc.end_ptr  = new_freed_alloc.start_ptr + remaining_size - 1;
                new_freed_alloc.file = it->file;
                new_freed_alloc.line = it->line;

                freed_allocations.insert(it + 1, new_freed_alloc); //add it to the freed list so it is reused in next iter.

            }
            else{//block fully used
                freed_allocations.erase(it);
            }
            return ptr;
        }
        else{
            //current freed alloc not enough for malloc
            //check if freed block can merge with default buffer (t25)
            if(freed_allocations.back().end_ptr == ((char *)&default_buffer.buffer[default_buffer.pos] -1)){
                struct allocation joined_alloc = freed_allocations.back();
                joined_alloc.end_ptr += (default_buffer.size - default_buffer.pos);
                freed_allocations.insert(it + 1, joined_alloc);
            }
        }
    }
    //no space in freed allocs.
    (&stats)->nfail++;
    (&stats)->fail_size+=sz;
    return nullptr;
}

// Return pointer to newly reserved memory
void* aAlloc_malloc(size_t sz, const char* file, int line) {
    void *ptr = nullptr;

    if(sz == 0) return ptr;

    //integer overflow if sz = SIZE_MAX
    if (sz > SIZE_MAX - (&stats)->total_size) {
        // Not enough space left in default buffer for allocation
        (&stats)->nfail++;
        (&stats)->fail_size += sz;
        return ptr;
    }
    //space in main buffer?
    if(sz <= default_buffer.size - default_buffer.pos){
        ptr = &default_buffer.buffer[default_buffer.pos];
        (&stats)->active_size += sz;
    }
    else{
        //coalesce freed alloc list if no un-alloced bytes available.
        std::sort(freed_allocations.begin(), freed_allocations.end(), compareByAddress); //sorted freed list.
        auto it1 = freed_allocations.begin();
        auto it2 = freed_allocations.begin() + 1;

        //2 ptr coalesce algo
        while(it1 != freed_allocations.end() && it2 != freed_allocations.end()){
            if((char *)it1->end_ptr == (char *)it2->start_ptr - 15){ //idk why 15 works here.
                it1->end_ptr = it2->end_ptr;
                freed_allocations.erase(it2); // Erase the 2nd alloc
            }
            else{
                it1++;
                it2++;
            }
        }
        //find free space
        ptr = aAlloc_find_free_space(sz);

    }
    
    // Somehow, the allocator has found the address of a contiguous chunk of `sz` size.
    if(ptr){
        // check alignment.
        int rem = (uintptr_t) ptr % alignof(std::max_align_t);
        int align = alignof(std::max_align_t);
        if(rem){
            default_buffer.pos += (align - rem); //aligned.
            ptr = &default_buffer.buffer[default_buffer.pos];
        }
        default_buffer.pos += (sz); // +1 for the 0xA5 boundary error check. Allocs themselves will be of the correct size tho.

        if(!(&stats)->ntotal) 
            (&stats)->heap_min = (uintptr_t)ptr;

        //highest allocated address is always the most recently allocated addr. FALSE after re use.
        if((&stats)->heap_max < (uintptr_t)&default_buffer.buffer[default_buffer.pos]) 
            (&stats)->heap_max = (uintptr_t)&default_buffer.buffer[default_buffer.pos];

        //printf("active num: %lld\n", (&stats)->nactive);
        (&stats)->nactive++;
        (&stats)->total_size += sz;
        (&stats)->ntotal++;

        //tag for boundary write
        if(default_buffer.pos < (8<<20) && default_buffer.pos > 0){
            *((char *)ptr + sz) = 0xA5;//if this byte not A5 on free, boundary write.
            //*((char *)ptr -1) = 0xA5;   
        }

        //add ptr alloc to active list.
        struct allocation this_active_alloc {};

        this_active_alloc.start_ptr = (char *)ptr;
        this_active_alloc.end_ptr = (char *)ptr + sz -1;//excludes 0xA5 byte. 
        this_active_alloc.file = file;
        this_active_alloc.line = line;

        active_allocations.push_back(this_active_alloc);

    }
    return ptr;
}


// Updates aactive, free lists
void aAlloc_free(void* ptr, const char* file, int line) {
    if(ptr == nullptr) return;

    //invalid free via min heap comparison
    if((uintptr_t)ptr < (&stats)->heap_min || (uintptr_t)ptr >(&stats)->heap_max){
       std::cerr<<"MEMORY BUG "<<file<<":"<<line<<": invalid free of pointer "<<ptr<<","<<" not in heap"<<std::endl;
       
       abort();
    }

    struct allocation this_freed_alloc {};

    for (auto it = active_allocations.begin(); it != active_allocations.end(); ++it) {  //O(N) free function.
        //this_freed_alloc.isfreed = it->isfreed;
        this_freed_alloc.start_ptr = it->start_ptr;
        this_freed_alloc.end_ptr = it->end_ptr;
        size_t b_size = this_freed_alloc.end_ptr - this_freed_alloc.start_ptr + 1;

        if ((uintptr_t)ptr == (uintptr_t)it->start_ptr) { //if freed ptr is active alloced.
            //found = true
                
            // if(*((char *)ptr + b_size) != 0xA5){
            // //bwe
            //     //std::cout<<(uint8_t)*((char *)ptr + b_size)<<std::endl;
            //     std::cerr<<"MEMORY BUG "<<file<<":"<<line<<": detected wild write during free of pointer "<<ptr<<std::endl;
            // //MEMORY BUG???: detected wild write during free of pointer ??ptr??
            //     //abort();
            // }

            uintptr_t h_min = (&stats)->heap_min;
            
            if((uintptr_t)ptr < h_min)
                (&stats)->heap_min = (uintptr_t)it->start_ptr;
            
            // add alloc to freed list
            freed_allocations.push_back(this_freed_alloc); //if coalesce not possible.
        
            // remove alloc from active list
            active_allocations.erase(it);

            size_t freed_sz = (size_t)((uintptr_t)this_freed_alloc.end_ptr- (uintptr_t)this_freed_alloc.start_ptr) + 1;
            
            (&stats)->active_size -= freed_sz;
            (&stats)->nactive--;
        
            return;
        }
        else if((uintptr_t)ptr < (uintptr_t)it->end_ptr && (uintptr_t)ptr > (uintptr_t)it->start_ptr){
            int rem = (uintptr_t)ptr - (uintptr_t)it->start_ptr;
            // size_t b_size = (uintptr_t)it->end_ptr - (uintptr_t)it->start_ptr + 1;
            int myline = it->line;
            const char *myfile = it->file;

            std::cerr<<"MEMORY BUG: "<<file<<":"<<line<<": invalid free of pointer "<<ptr<<","<<" not allocated"<<std::endl;
            std::cerr<<myfile<<":"<<myline<<": "<<ptr<<" is "<<rem<<" bytes inside a "<<b_size<<" byte region allocated here"<<std::endl;
        }
        // exit the loop since the element was found and removed
    }
    //ptr not active.
    for(auto it2 = freed_allocations.begin(); it2 != freed_allocations.end();++it2){
        if ((uintptr_t)ptr == (uintptr_t)it2->start_ptr) {
            //block not currently active but was freed earlier.
            std::cerr<<"MEMORY BUG "<<file<<":"<<line<<": invalid free of pointer "<<ptr<<","<<" double free"<<std::endl;
            abort();
        }

    }
    // if(this_freed_alloc.isfreed && cnt == active_allocations.size()){
    //     std::cerr<<"MEMORY BUG "<<file<<":"<<line<<": invalid free of pointer "<<ptr<<","<<" double free"<<std::endl;
    //     abort();
    // }
    return;
}





void* aAlloc_calloc(size_t count, size_t sz, const char* file, int line) {
    //check size_t overflow also
    if(count * sz > (default_buffer.size - (&stats)->total_size) || sz > (SIZE_MAX - (&stats)->total_size)/ count || count> (SIZE_MAX - (&stats)->total_size)/ sz) {
        
        (&stats)->nfail++;
        return nullptr;
    }
    void* ptr = aAlloc_malloc(count * sz, file, line);
    if (ptr) {
        memset(ptr, 0, count * sz);
    }
    return ptr;
}


/// aAlloc_get_statistics()
///    Return the current memory statistics.
aAlloc_statistics aAlloc_get_statistics() {
    return stats;
}


/// aAlloc_print_statistics()
///    Prints the current memory statistics.

void aAlloc_print_statistics() {
    stats = aAlloc_get_statistics();
    printf("alloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("alloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}


/// aAlloc_print_leak_report()
///    Prints a report of all currently-active allocated blocks of dynamic
///    memory.
void aAlloc_print_leak_report() {
    if(!active_allocations.size())
        return;

    for(auto a: active_allocations){
        //if alloc active->malloced but not freed yet.
        void *ptr = (void *)a.start_ptr;
        const char *file = a.file;
        int line = a.line;
        size_t size = (uintptr_t)a.end_ptr - (uintptr_t)a.start_ptr + 1;

        std::cout<<"LEAK CHECK: "<<file<<":"<<line<<":"<<" allocated object "<<ptr<<" with size "<<size<<std::endl;
    }    
}

static void check_contents(unsigned char* p, unsigned char ch) {
    unsigned char buf[10];
    memset(buf, ch, 10);
    assert(memcmp(p, buf, 10) == 0);
}