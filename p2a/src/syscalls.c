#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  if(argc < 3){
    printf(2, "Invalid Input");
    exit();
  }
  int N = atoi(argv[1]);
  int g = atoi(argv[2]);
  int r = N-g;
  int pid = getpid();

  for(int i=1; i<g; i++){
    getpid();
  }

  for(int i=0; i<r; i++){
    kill(-1);
  }

  int totalCalls = getnumsyscalls(pid);
  int goodCalls = getnumsyscallsgood(pid);
  printf(2, "%d %d\n", totalCalls, goodCalls);
  exit();
}
