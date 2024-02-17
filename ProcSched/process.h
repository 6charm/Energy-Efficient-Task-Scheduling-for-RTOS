#ifndef __PROCESS_H
#define __PROCESS_H

// define the process structure
struct process
{
    int PID;            // Process ID
    int cpu_cycles;     // in the worst case, this process will require this many cycles to complete
    float period;       // the time between two consecutive activations
    float deadline;     // the time by which the process must complete its execution
    float priority;     // the priority of the process, required for scheduling
    float release_time; // the time at which the process was released
    int actual_cycles;  // the actual number of cycles the process required
    int completions;    // the number of times the process has completed successfully
    float util;         // the CPU Utilization of the process
};

/**
 * NOTE: All the absolute deadlines here are based on Maximum Frequency of the CPU
 */

// simple function that prints functions
extern void print_process(struct process *p);

// function that builds a process from the given parameters
extern struct process *create_process(int cpu_cycles, float period);

// function that randomly decides that actual running cycle from CPU Cycles
extern int actual_cycles(struct process *p);

#endif