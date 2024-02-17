#include "aIO.h"
#include <errno.h>
#include <error.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <stdint.h>  // SSIZE_MAX

// stdio-aIO.c
//    This is a version of aIO.c that uses stdio internally.

static const int e_FailedOpen = 1;

struct aIO_file {
    FILE* f;
};

aIO_file* aIO_fdopen(int fd, int mode) {
    assert(fd >= 0);
    aIO_file* f = (aIO_file*) malloc(sizeof(aIO_file));
    f->f = fdopen(fd, mode == O_RDONLY ? "r" : "w");
    return f;
}

int aIO_close(aIO_file* f) {
    aIO_flush(f);
    int r = fclose(f->f);
    free(f);
    return r;
}


int aIO_readc(aIO_file* f) {
    return fgetc(f->f);
}

int aIO_writec(aIO_file* f, int ch) {
    return fputc(ch, f->f);
}


int aIO_flush(aIO_file* f) {
    return fflush(f->f);
}


ssize_t aIO_read(aIO_file* f, char* buf, size_t sz) {
    size_t n = fread(buf, 1, sz, f->f);
    if (n)
        return (ssize_t) n;
    else if (feof(f->f))
        return 0;
    else
        return ferror(f->f);
}

ssize_t aIO_write(aIO_file* f, const char* buf, size_t sz) {
    size_t n = fwrite(buf, 1, sz, f->f);
    if (n)
        return (ssize_t) n;
    else if (feof(f->f))
        return 0;
    else
        return ferror(f->f);
}


int aIO_fseek(aIO_file* f, off_t pos) {
    return fseek(f->f, pos, SEEK_SET);
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