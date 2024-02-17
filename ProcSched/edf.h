#ifndef __EDF_H
#define __EDF_H

#include "process.h"

#define MAX_PROCESSES 5

struct edf
{
    int num_processes;         // number of processes
    struct process *processes; // array of processes
};

// now we have several functions

// init_struct --> num_processes = 0; processes = NULL;
extern struct edf *init_struct();

// adds a process to the array of processes
extern int add_to_scheduler(struct edf *edf, struct process *process);

// finds the next process to be executed, to be used only after finding priorities
extern void sort_processes(struct edf *edf, float time);

// Impose the EDF Schedulability Test
extern int edf_test(struct edf *edf, float alpha);

// finds the hyper-period of the processes in the scheduler
extern float hyper_period(struct edf *edf);

#endif