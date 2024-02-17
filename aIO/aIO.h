/**
 * FILE: aIO.h
 * -----------
 * Defines the aIO buffered I/O library
 * API. Standard File I/O methods similar 
 * to stdio.h
*/

#ifndef __AIO_H__
#define __AIO_H__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>

/* aIO File struct */
typedef struct aIO_file aIO_file;

aIO_file *aIO_fdopen(int fd, int mode);                       // open a file via fd
aIO_file *aIO_open_check(const char *filename, int mode);   // Verify open
int aIO_close(aIO_file *file);                              // close a file

ssize_t aIO_filesize(aIO_file *f);

int aIO_fseek(aIO_file *f, off_t pos);

int aIO_readc(aIO_file *f);
ssize_t aIO_read(aIO_file *f, char *buf, size_t nbytes);

int aIO_writec(aIO_file *f, int ch);
ssize_t aIO_write(aIO_file *f, const char *buf, size_t nbytes);

int aIO_flush(aIO_file *f);

#endif



