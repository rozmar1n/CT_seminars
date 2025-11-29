#define _POSIX_C_SOURCE 200809L /* для kill, getppid */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main (int argc, char ** argv)
{
	int year;
	if (argc < 2) 
	{
		fprintf (stderr, "child: too few arguments\n");
		return 2;
	}

	year = atoi (argv[1]);

	if (year <= 0)
		return 2;

	if ( ((year%4 == 0) && (year%100 != 0)) ||
			(year%400 == 0) )
	{
		if (kill (getppid(), SIGUSR1) == -1) {
			perror("kill SIGUSR1");
			return 2;
		}
	}
	else
	{
		if (kill (getppid(), SIGUSR2) == -1) {
			perror("kill SIGUSR2");
			return 2;
		}
	}

	return 0;
}
