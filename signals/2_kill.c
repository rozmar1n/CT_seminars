#define _POSIX_C_SOURCE 200809L /* нужно для объявления kill() и signal() в C99 */
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
  pid_t dpid = getpid();

  if (signal(SIGABRT, SIG_IGN) == SIG_ERR) {
    perror("signal");
    return 1;
  }

  printf("PID: %d. SIGABRT will be ignored.\n", (int)dpid);

  if (kill(dpid, SIGABRT) == -1) {
    perror("kill");
    return 1;
  }

  printf("Signal sent but ignored — program keeps running.\n");
  return 0;
}
