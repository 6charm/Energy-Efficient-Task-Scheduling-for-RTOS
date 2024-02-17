/**
 * FILE: aIO.c.
 * ------------
 * This is a non-buffered I/O library with 
 * char-at-a-time reads/writes.
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

static const int e_FailedOpen = 1;

static const int e_FailedMalloc = 2;

struct aIO_file {
    int fd;
    int mode; // O_RDONLY, O_WRONLY
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
// ssize_t aIO_filesize(aIO_file* f) {
//     struct stat s;
//     int r = fstat(f->fd, &s);
//     if (r >= 0 && S_ISREG(s.st_mode) && s.st_size <= SSIZE_MAX) {
//         return s.st_size;
//     }
//     return -1;
// }

int aIO_fseek(aIO_file *f, off_t pos) {
    off_t r = lseek(f->fd, (off_t)pos, SEEK_SET);
    if (r == (off_t)pos) {
        return 0;
    } else {
        return -1;
    }
}

int aIO_readc(aIO_file *f) {
    unsigned char ch;
    ssize_t nr = read(f->fd, &ch, 1); // char-at-a-time
    if (nr == 1) {
        return ch;
    } else if (nr == 0) {
        return EOF;
    } else {
        assert(nr == -1 && errno > 0);
        return -1;
    }
}

/*
 * Reads <= sz bytes into buf. On success, returns bytes read.
 * On short-read, bytes_read < sz.
*/
ssize_t aIO_read(aIO_file *f, char *buf, size_t sz) {
    size_t bytes_read = 0; 
    while (bytes_read != sz) {
        int ch = aIO_readc(f);  // EOF is a macro, -1
        if (ch == EOF) {
            break;
        }
        buf[bytes_read] = ch;
        ++bytes_read;
    }
    // Reached file-end or read all sz bytes
    if (bytes_read == 0 && sz != 0) {
        return -1;
    } else {
        return bytes_read;
    }
}

int aIO_writec(aIO_file* f, int c) {
    unsigned char ch = c;
    ssize_t nw = write(f->fd, &ch, 1);
    if (nw == 1) {
        return 0;
    } else {
        return -1;
    }
}

ssize_t aIO_write(aIO_file* f, const char* buf, size_t sz) {
    size_t bytes_written = 0;
    while (bytes_written != sz) {
        if (aIO_writec(f, buf[bytes_written]) == -1) {
            break;
        }
        ++bytes_written;
    }
    if (bytes_written == 0 && sz != 0) {
        return -1;
    } else {
        return bytes_written;
    }
}

/* Forces a write of any f cache buffers that contain data. */
int aIO_flush(aIO_file* f) {
    (void) f;
    return 0;
}
