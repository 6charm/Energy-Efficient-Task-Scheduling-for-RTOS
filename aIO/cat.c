/**
 * FILE: cat.c
 * -----------
 * Usage: ./cat [FILE]
 * Copy input FILE to STDOUT char-at-a-time i.e. 1B, sequential
*/

#include "aIO.h"
#include "profile.h"

int main(int argc, char** argv) {
    const char* in_filename = argc >= 2 ? argv[1] : NULL;
    aIO_profile_begin();
    aIO_file* inf = aIO_open_check(in_filename, O_RDONLY);
    aIO_file* outf = aIO_fdopen(STDOUT_FILENO, O_WRONLY);

    while (1) {
        int ch = aIO_readc(inf);
        if (ch == EOF)
            break;
        aIO_writec(outf, ch);
    }

    aIO_close(inf);
    aIO_close(outf);
    aIO_profile_end();
}