#define _POSIX_C_SOURCE 200809L /* для sigaction, pause */
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

volatile sig_atomic_t sig_count = 0;

void sig_handler (int snum)
{
	(void)snum; 
    if (sig_count < 5)
		sig_count++;
}

int main (void)
{
	struct sigaction act;
	struct sigaction default_act;

	sigemptyset (&act.sa_mask);
	act.sa_handler = &sig_handler;
	act.sa_flags = 0;

	if (sigaction (SIGINT, &act, NULL) == -1) 
	{
		perror ("sigaction (install handler)");
		return 1;
	}

	while (1) 
	{
		pause();
		
        if (sig_count) 
		{
			fprintf (stderr, "SIGINT received (%d/5)\n", (int)sig_count);

			if (sig_count >= 5) 
			{
				sigemptyset (&default_act.sa_mask);
				default_act.sa_flags = 0;
				default_act.sa_handler = SIG_DFL;

				if (sigaction (SIGINT, &default_act, NULL) == -1) 
				{
					perror ("sigaction (restore default)");
					return 1;
				}

				fprintf (stderr, "Handler removed, default action restored.\n");
			}
		}
	}
	return 0;
}
