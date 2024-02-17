#include "cpu.h"
#include <stdio.h>
#include "cc.h"
#include "process.h"

int run(struct CPU *cpu, float *time, struct edf *edf)
{

    struct process *p = &edf->processes[0];

    // check if our task is actually released
    // if (*time < p->release_time)
    //     return 0;

    printf("T%.0f: P%d Completions: %d.\n", *time, p->PID, p->completions);

    // p = sort_processes(scheduler, time);

    // UPON TASK RELEASE
    p->util = p->cpu_cycles / p->period;
    cpu->frequency = factor_frequency(edf);

    // now we are ready to run the task
    // first find how many actual cycles we have to run
    int ac = actual_cycles(p);

    // then find the actual running time --> no. of cycles / frequency
    float art = (float)ac / cpu->frequency;

    // check if we can complete this run of the process before its absolute deadline
    if (*time + art > p->deadline)
    {
        printf("P%d missed its deadline.\n", p->PID);
        return -1;
    }

    // print the fact which process ran for how long
    printf("P%d ran for %.2f seconds (%d cycle(s), f:%.2f).\n", p->PID, art, ac, cpu->frequency);

    p->completions++;             // inc completions
    p->deadline += p->period;     // set the next deadline
    p->release_time += p->period; // set the next release time for the process

    // UPON TASK COMPLETION
    p->util = (float)ac / p->period; // update the utilization of the process
    cpu->frequency = factor_frequency(edf);

    // increment and set the new time
    *time += art;

    // return 1 for success
    return 1;
}