/**
 * FILE: aIO.c.
 * ------------
 * This is the aIO source file that 
 * implements the aIO API.
*/

#include "aIO.h"
#include <errno.h>
#include <error.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <stdint.h>  // SSIZE_MAX
#include <sys/param.h>

static const int e_FailedOpen = 1;
static const int e_FailedMalloc = 2;

#define CACHESIZE 64 << 10 // to init struct.

static const int k_CacheSize = CACHESIZE; // 64KB

struct aIO_file {
    int fd;
    int mode; // O_RDONLY, O_WRONLY

    unsigned char cache[CACHESIZE];
    
    // `tag`: File offset of first byte of cached data (0 when file is opened).
    off_t tag;

    // `end_tag`: File offset one past the last byte of cached data (0 when file is opened).
    off_t end_tag;

    // In read caches, this is the file offset of the next character to be read.
    off_t pos_tag;
};

static void *safe_malloc(size_t sz) {
    void *ptr = malloc(sz);
    if (!ptr) {
        error(e_FailedMalloc, errno, "Malloc of size %ld failed.\n", sz);
        exit(EXIT_FAILURE);
    }
    memset(ptr, 0, sz);
    return ptr;
}


aIO_file *aIO_fdopen(int fd, int mode) {
    assert(fd >= 0);
    aIO_file *f = safe_malloc(sizeof(aIO_file));
    f->fd = fd;
    f->mode = mode;
    f->tag = 0;
    f->end_tag = 0;
    f->pos_tag = 0;

    return f;
}

aIO_file* aIO_open_check(const char* filename, int mode) {
    int fd;
    if (filename)
        fd = open(filename, mode);
    else if (mode == O_RDONLY)
        fd = STDIN_FILENO;
    else
        fd = STDOUT_FILENO;

    if (fd < 0) {
        error(e_FailedOpen, errno, "%s failed to open.\n", filename);
        exit(EXIT_FAILURE);
    }
    return aIO_fdopen(fd, mode);
}

int aIO_close(aIO_file *f) {
    aIO_flush(f);
    int r = close(f->fd);
    free(f);
    return r;
}

/*
 * Return the number of bytes in `f`. Returns -1 if `f` is 
 * not a seekable file (eg: a pipe).
 */
int aIO_fseek(aIO_file *f, off_t off) {
    // Flush cache if not empty in write mode.
    if (f->pos_tag != f->tag && (f->mode == O_WRONLY || f->mode == O_RDONLY)) {
        ssize_t nw = write(f->fd, f->cache, f->pos_tag - f->tag);
        if (nw <= 0) {
            return -1;
        }
        // Reset Cache
        f->tag = 0;
        f->pos_tag = 0;
        f->end_tag = 0;        
    }

    // Update cache before reading.
    if (off >= f->tag && off < f->end_tag && (f->mode == O_RDONLY || f->mode == O_RDWR)) {
        // Seek to position in cache.
        f->pos_tag = off;
    } else {
        // Invalidate cache
        f->tag = off;
        f->end_tag = off;
        f-> pos_tag = off;
    }

    // Do seek
    off_t rs = lseek(f->fd, off, SEEK_SET);
    if (rs == 1) {
        return -1;
    }

    return 0;
}

int aIO_readc(aIO_file *f) {
    // Cache hit
    if (f->pos_tag < f->end_tag) {
        // Read char
        unsigned char ch = f->cache[f->pos_tag - f->tag];
        f->pos_tag += 1;
        return ch;
    }

    // Cache miss, add more data to cache.
    f->tag = f->pos_tag;
    ssize_t nr = read(f->fd, f->cache, k_CacheSize);
    if (nr <= 0) {
        // Either an error occurred, or we hit EOF
        if (nr == 0) {
            errno = 0; // clear `errno` to indicate EOF
        } else {
            assert(nr == -1 && errno > 0);
        }
        return -1;
    }

    // New cache tags
    f->end_tag = f->tag + nr;

    // Return next char from refilled cache
    unsigned char ch = f->cache[0];
    f->pos_tag += 1;
    return ch;
}

/*
 * Reads <= sz bytes into buf. On success, returns bytes read.
 * On short-read, bytes_read < sz.
*/
ssize_t aIO_read(aIO_file *f, char *buf, size_t sz) {
    size_t bytes_read = 0; 
    while (bytes_read < sz) {
        // check for data in cache
        if (f->pos_tag < f->end_tag) {
            // copy all bytes if sz < end - pos, else sz = sz
            size_t bytes_from_cache = MIN(sz - bytes_read, f->end_tag - f->pos_tag);
            memcpy(&buf[bytes_read],  &f->cache[f->pos_tag - f->tag], bytes_from_cache);
            bytes_read += bytes_from_cache;
            f->pos_tag += bytes_from_cache;
        }

        // If sz was > cache, need to read from file.
        if (bytes_read < sz) {
            f->tag = f->pos_tag;
            ssize_t nr = read(f->fd, f->cache, k_CacheSize);
            if (nr <= 0) {
                if (bytes_read > 0) {
                    return bytes_read;
                } else {
                    return nr;
                }
            }
            f->end_tag = f->tag + nr;
        }
    }
    return bytes_read;
}

int aIO_writec(aIO_file* f, int c) {
    unsigned char ch = c;

    if (f->pos_tag == f->end_tag) {
        if (aIO_flush(f) == -1) {
            // write error
            return -1;
        }
    }

    f->cache[f->pos_tag - f->tag] = ch;
    f->pos_tag++;

    return 0;
}


ssize_t aIO_write(aIO_file* f, const char* buf, size_t sz) {
    size_t bytes_to_write = sz;
    size_t bytes_written = 0;

    while(bytes_to_write) {
        // Flush cache if full
        if (f->pos_tag == f->end_tag) {
            if (aIO_flush(f) == -1){
                // write error
                return -1;
            }
        }
        size_t bytes_to_cache = MIN(bytes_to_write, f->end_tag - f->pos_tag);
        memcpy(&f->cache[f->pos_tag - f->tag], &buf[bytes_written], bytes_to_cache);
        bytes_written += bytes_to_cache;
        f->pos_tag += bytes_to_cache;
        bytes_to_write -= bytes_to_cache;
    }

    return bytes_written;
}


/* Forces a write of any f cache buffers that contain data. */
int aIO_flush(aIO_file* f) {
    int bytes_written = write(f->fd, f->cache, f->pos_tag - f->tag);
    if (bytes_written >= 0) {
        f->tag = f->pos_tag;
        f->end_tag += k_CacheSize;
        return 0;
    }
    return -1;
}
