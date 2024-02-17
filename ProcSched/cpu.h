#ifndef __CPU_H
#define __CPU_H

#include "process.h"
#include "edf.h"

#define MAX_FREQUENCY 1

struct CPU
{
    float frequency; // Cycles Per Second
};

// run a given task
extern int run(struct CPU *cpu, float *time, struct edf *edf);

#endif