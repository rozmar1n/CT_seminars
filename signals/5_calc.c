#define _POSIX_C_SOURCE 200809L /* Ð´Ð»Ñ sigaction, clock_gettime */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <inttypes.h>

static volatile sig_atomic_t print_status = 0;

static void sigusr1_handler(int signo)
{
  (void)signo; 
  print_status = 1;
}

static double elapsed_seconds(const struct timespec *start,
                              const struct timespec *now)
{
  time_t sec = now->tv_sec - start->tv_sec;
  long nsec = now->tv_nsec - start->tv_nsec;
  return (double)sec + (double)nsec / 1e9;
}

int main(void)
{
  struct sigaction act;
  struct timespec t_start;
  uint64_t n = 1;
  uint64_t fact = 1;

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = sigusr1_handler;

  if (sigaction(SIGUSR1, &act, NULL) == -1) {
    perror("sigaction");
    return 1;
  }

  if (clock_gettime(CLOCK_MONOTONIC, &t_start) == -1) {
    perror("clock_gettime");
    return 1;
  }

  printf("PID: %ld. Send SIGUSR1 (kill -USR1 %ld) to print status.\n",
         (long)getpid(), (long)getpid());

  while (1) {
    sleep(2);
    if (UINT64_MAX / n < fact) {
      fprintf(stderr, "Stopping: overflow risk at n=%" PRIu64 ".\n", n);
      break;
    }

    fact *= n;
    ++n;

    if (print_status) {
      struct timespec t_now;
      if (clock_gettime(CLOCK_MONOTONIC, &t_now) == -1) {
        perror("clock_gettime");
        return 1;
      }

      printf("[SIGUSR1] elapsed=%.3f s, n=%" PRIu64
             ", factorial=%" PRIu64 "\n",
             elapsed_seconds(&t_start, &t_now), n - 1, fact);
      fflush(stdout);
      print_status = 0;
    }
  }

  return 0;
}
