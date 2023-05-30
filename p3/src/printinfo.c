#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int main(int argc, char *argv[])
{
    if(argc != 6)exit();

    int sliceA = atoi(argv[1]);
    char* sleepA = argv[2];
    int sliceB = atoi(argv[3]);
    char* sleepB = argv[4];
    int sleepParent = atoi(argv[5]);

    int pid = fork2(sliceA);
    if(pid == 0){
        char *argv1[3];
        argv1[0] = "loop";
        argv1[1] = sleepA;
        argv1[2] = 0;
        exec("loop", argv1);
    }

    int pid2 = fork2(sliceB);
    if(pid2 == 0){
        char *argv2[3];
        argv2[0] = "loop";
        argv2[1] = sleepB;
        argv2[2] = 0;
        exec("loop", argv2);
    }

    sleep(sleepParent);
    struct pstat pstat_t;
    getpinfo(&pstat_t);

    //printf(1, "Compticks: %d %d\n", pstat_t.compticks[0], pstat_t.compticks[1]);

    printf(1, "PID, Timeslice, Compticks, Schedticks, Sleepticks, Switches\n");
    for(int i=0; i<NPROC; i++){
        printf(1, "%d ", pstat_t.pid[i]);
        printf(1, "%d ", pstat_t.timeslice[i]);
        printf(1, "%d ", pstat_t.compticks[i]);
        printf(1, "%d ", pstat_t.schedticks[i]);
        printf(1, "%d ", pstat_t.sleepticks[i]);
        printf(1, "%d\n", pstat_t.switches[i]);
    }

    wait();
    wait();
    exit();
}