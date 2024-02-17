#include "aIO.h"
#include "profile.h"
#include <sys/stat.h> // fstat

/**
 * Usage: ./reversecat [FILE]
 * Copy FILE to STDOUT char-at-a-time, char order reversed.
*/ 


int main(int argc, char **argv) {
    const char *in_filename = argc >= 2 ? argv[1] : NULL;
    aIO_profile_begin();
    aIO_file *inf = aIO_open_check(in_filename, O_RDONLY);
    aIO_file *outf = aIO_fdopen(STDOUT_FILENO, O_WRONLY);

    ssize_t in_size = -1;
    
    struct stat s;
    int r = fstat(inf->fd, &s);
    if (r >= 0 && S_ISREG(s.st_mode)) {
        in_size = s.st_size;
    }

    if (in_size == -1) {
        fprintf(stderr, "reversecat: can't get size of input file\n");
        exit(1);
    }

    while (in_size != 0) {
        --in_size;
        aIO_fseek(inf, in_size);
        int ch = aIO_readc(inf);
        aIO_writec(outf, ch);
    }

    aIO_close(inf);
    aIO_close(outf);
    aIO_profile_end();
}