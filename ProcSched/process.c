#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "process.h"

// This file will have all the process related functions

// print some info about the given process
void print_process(struct process *p)
{
    printf("P%d: CPU Cycles:%d, Period:%.2f.\n", p->PID, p->cpu_cycles, p->period);
}

// create a new process from given parameters, (much like the constructor for a class)
struct process *create_process(int cpu_cycles, float period)
{
    struct process *p = malloc(sizeof(struct process)); // empty container

    // PID will be assigned by the Scheduler

    // assign values
    p->cpu_cycles = cpu_cycles;
    p->period = period;

    p->deadline = period;
    p->release_time = 0.0;
    p->priority = 10.0;
    p->completions = 0;
    p->actual_cycles = 0; // this value has to be randomly generated on every release of a process
    p->util = p->cpu_cycles / p->period;

    return p; // return
}

int actual_cycles(struct process *p)
{
    // set the actual cycles --> [1, cpu_cycles]
    srand(time(0));
    p->actual_cycles = rand() % p->cpu_cycles + 1;

    return p->actual_cycles; // return for ease of use
}