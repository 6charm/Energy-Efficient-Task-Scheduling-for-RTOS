#include <stdlib.h>

#include "process.h" // this is a second import, but since we have used ifdef, no need to worry
#include "edf.h"

// create the new edf struct
struct edf *init_struct()
{
    struct edf *schedule = malloc(sizeof(struct edf));                    // allocate memory for the struct
    schedule->num_processes = 0;                                          // initially, there are no processes                                                                // calculate the size of the array
    schedule->processes = malloc(sizeof(struct process) * MAX_PROCESSES); // allocate memory for the processes

    return schedule;
}

// add a process to our scheduler
int add_to_scheduler(struct edf *edf, struct process *p)
{
    // check to see if we have any space for more processes
    if (edf->num_processes == MAX_PROCESSES)
        return -1;

    p->PID = edf->num_processes;             // assign the PID of the process
    edf->processes[edf->num_processes] = *p; // add the process to the array
    edf->num_processes++;                    // inc the number of processes
    return 1;                                // indicating success
}

// find and assign the priorities of each process in our scheduler
// this function is not externed and wont be visible to the main users
void find_priorities(struct edf *edf, float time)
{
    // we have to find how close the deadline of each process is to the current time
    for (int i = 0; i < edf->num_processes; i++)
    {
        // calculate the priority of the process
        edf->processes[i].priority = edf->processes[i].deadline - time;
    }
}

// declare an insertion sorting function
void insertion_sort(struct edf *edf)
{
    // we have to sort the processes in the array by their priority
    for (int i = 1; i < edf->num_processes; i++)
    {
        struct process key = edf->processes[i];
        int j = i - 1;

        while (j >= 0 && edf->processes[j].priority > key.priority)
        {
            edf->processes[j + 1] = edf->processes[j];
            j = j - 1;
        }
        edf->processes[j + 1] = key;
    }
}

// sort according to priorities set and return the process with the highest priority
void sort_processes(struct edf *edf, float time)
{
    // call the find priority function here
    find_priorities(edf, time);

    // sort the processes according to their priorities
    insertion_sort(edf);
}

int edf_test(struct edf *edf, float alpha)
{
    // check if total utilization of the CPU is less than alpha
    float c = 0.0;
    for (int i = 0; i < edf->num_processes; i++)
    {
        c += edf->processes[i].util;
    }

    return (c <= alpha) ? 1 : 0;
}

// supporting gcd function
int gcd(int n1, int n2)
{
    if (n2 != 0)
        return gcd(n2, n1 % n2);
    else
        return n1;
}

// find the hyper-period of the processes in the scheduler --> a * b == lcm(a, b) * gcd(a, b)
// lcm of the periods of all the processes
float hyper_period(struct edf *edf)
{
    // find the LCM of the periods of the processes, considering everything as an integer
    int lcm = (int)edf->processes[0].period;
    for (int i = 1; i < edf->num_processes; i++)
    {
        lcm = (int)((lcm * edf->processes[i].period) / (float)gcd(lcm, edf->processes[i].period));
    }

    return lcm;
}
