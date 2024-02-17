#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>


void aIO_profile_begin(void);
void aIO_profile_end(void);