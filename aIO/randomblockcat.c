/**
 * FILE: randomblockcat.c
 * Usage: ./randomblockcat [-b MAXBLOCKSIZE] [-S SEED] [FILE]
 * Copy FILE to STDOUT block-at-a-time. Blocksize random in [1, MAXBLOCKSIZE)
*/
#include "aIO.h"
#include "profile.h"

int main(int argc, char **argv) {
    // Parse arguments
    size_t max_blocksize = 4096;
    srandom(6969);
    while (argc >= 3) {
        if (strcmp(argv[1], "-b") == 0) {
            max_blocksize = strtoul(argv[2], 0, 0);
            argc -= 2, argv += 2;
        } else if (strcmp(argv[1], "-S") == 0) {
            srandom(strtoul(argv[2], 0, 0));
            argc -= 2, argv += 2;
        } else
            break;
    }

    // Allocate buffer, open files
    assert(max_blocksize > 0);
    char *buf = malloc(max_blocksize);

    const char *in_filename = argc >= 2 ? argv[1] : NULL;
    aIO_profile_begin();
    aIO_file *inf = aIO_open_check(in_filename, O_RDONLY);
    aIO_file *outf = aIO_fdopen(STDOUT_FILENO, O_WRONLY);

    // Copy file data
    while (1) {
        size_t m = (random() % max_blocksize) + 1;
        ssize_t amount = aIO_read(inf, buf, m);
        if (amount <= 0)
            break;
        aIO_write(outf, buf, amount);
    }

    aIO_close(inf);
    aIO_close(outf);
    aIO_profile_end();
}