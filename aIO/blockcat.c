/**
 * FILE: blockcat.c
 * ----------------
 * Usage: ./blockcat [-b BLOCKSIZE] [FILE]
 * Copy FILE to STDOUT block-at-a-time i.e. sequential, xKB
 * Default: 4KB
*/

#include "aIO.h"
#include "profile.h"

int main(int argc, char** argv) {
    // Parse arguments
    size_t blocksize = 4096;
    if (argc >= 3 && strcmp(argv[1], "-b") == 0) {
        blocksize = strtoul(argv[2], 0, 0);
        argc -= 2, argv += 2;
    }

    // Allocate buffer, open files
    assert(blocksize > 0);
    char* buf = malloc(blocksize);

    const char* in_filename = argc >= 2 ? argv[1] : NULL;
    aIO_profile_begin();
    aIO_file* inf = aIO_open_check(in_filename, O_RDONLY);
    aIO_file* outf = aIO_fdopen(STDOUT_FILENO, O_WRONLY);

    // Copy file data
    while (1) {
        ssize_t amount = aIO_read(inf, buf, blocksize);
        if (amount <= 0)
            break;
        aIO_write(outf, buf, amount);
    }

    aIO_close(inf);
    aIO_close(outf);
    aIO_profile_end();
}