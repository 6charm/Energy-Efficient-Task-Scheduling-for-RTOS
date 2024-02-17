#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "process.h"
#include "edf.h"
#include "cpu.h"

int main()
{
    // create our EDF Scheduler
    struct edf *scheduler = init_struct();

    // add the three sample process from the paper
    add_to_scheduler(scheduler, create_process(3, 8.0));
    add_to_scheduler(scheduler, create_process(3, 10.0));
    add_to_scheduler(scheduler, create_process(1, 14.0));

    // Confirm we have everything in order
    printf("\n");
    printf("Number of processes: %d\n", scheduler->num_processes);
    for (int i = 0; i < scheduler->num_processes; i++)
        print_process(&scheduler->processes[i]);

    // Find and set some global params
    float time = 1.0;
    float hp = hyper_period(scheduler);

    printf("Hyper Period: %.2f\n", hp); // print the hyper period

    // create our CPU and set the frequency
    struct CPU *cpu = (struct CPU *)malloc(sizeof(struct CPU));
    cpu->frequency = 1.00; // run at 1 cycle per second

    // Start the simulation
    printf("\nEDF Starting ... \n");
    while (time < hp) // change time here to hyper_period for final demonstration
    {
        printf("\n");

        // Run the Task
        int ret = run(cpu, &time, scheduler);
        if (ret == 0)
        {
            // means no task is released yet
            printf("T%.0f:  CPU IDLING.\n", time);
            time += 1.00; // inc 1s
        }
        else if (ret == -1)
        {
            // means the task missed its deadline
            break;
        }

        // rearrange all the processes in the edf
        sort_processes(scheduler, time);

        sleep(1); // wait so we can see the output
    }

    printf("T%.0f: One Hyper Period is Over, Successful Execution.!\n");

    return 0;
}
