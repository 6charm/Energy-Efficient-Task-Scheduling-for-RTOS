#include "profile.h"

static struct timeval tv_begin;

void aIO_profile_begin(void) {
    int r = gettimeofday(&tv_begin, 0);
    assert(r >= 0);
}

void aIO_profile_end(void) {
    struct timeval tv_end;
    struct rusage usage, cusage;

    int r = gettimeofday(&tv_end, 0);
    assert(r >= 0);
    r = getrusage(RUSAGE_SELF, &usage); // calling process
    assert(r >= 0);
    r = getrusage(RUSAGE_CHILDREN, &cusage); // All terminated children (not yet reaped)
    assert(r >= 0);

    timersub(&tv_end, &tv_begin, &tv_end);
    timeradd(&usage.ru_utime, &cusage.ru_utime, &usage.ru_utime);
    timeradd(&usage.ru_stime, &cusage.ru_stime, &usage.ru_stime);

    char buf[1000];
    int len = sprintf(buf, "{\"time\":%ld.%06ld, \"utime\":%ld.%06ld, \"stime\":%ld.%06ld, \"maxrss\":%ld}\n",
                      tv_end.tv_sec, (long) tv_end.tv_usec,
                      usage.ru_utime.tv_sec, (long) usage.ru_utime.tv_usec,
                      usage.ru_stime.tv_sec, (long) usage.ru_stime.tv_usec,
                      usage.ru_maxrss + cusage.ru_maxrss);

    // Print the report to file descriptor 100 if it's available.
    // Read from fd 100 to generate report on STDOUT
    off_t off = lseek(100, 0, SEEK_CUR);

    // Very unlikely that fd 100 will be non-seekable and in use, but just in case
    int fd = (off != (off_t)(-1) || errno == ESPIPE ? 100 : STDERR_FILENO);
    
    if (fd == STDERR_FILENO)
        fflush(stderr);

    // write to 100 or stderr.
    ssize_t bytes_written = write(fd, buf, len);
    
    assert(bytes_written == len);
}