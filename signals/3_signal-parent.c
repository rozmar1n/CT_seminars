#define _POSIX_C_SOURCE 200809L /* для sigaction, execl, fork, kill */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>

volatile sig_atomic_t sig_status = 0; 

void handle_usr1 (int s_num)
{
	(void)s_num; 
	sig_status = 1;
}

void handle_usr2 (int s_num)
{
	(void)s_num;
	sig_status = 2;
}

int main (int argc, char ** argv)
{
	struct sigaction act_usr1, act_usr2;
	pid_t child_pid;

	sigemptyset (&act_usr1.sa_mask);
	sigemptyset (&act_usr2.sa_mask);
	act_usr1.sa_flags = 0;
	act_usr2.sa_flags = 0;
	act_usr1.sa_handler = &handle_usr1;
	act_usr2.sa_handler = &handle_usr2;

	if (sigaction (SIGUSR1, &act_usr1, NULL) == -1) 
	{
		perror ("sigaction (SIGUSR1)");
		return 1;
	}

	if (sigaction (SIGUSR2, &act_usr2, NULL) == -1) 
	{
		perror ("sigaction (SIGUSR2)");
		return 1;
	}

	if (argc < 2) 
	{
		fprintf (stderr, "Too few arguments\n");
		return 1;
	}	

	child_pid = fork();
	if (child_pid == -1) {
		perror("fork");
		return 1;
	}

	if (child_pid == 0) 
	{
		execl ("./3_signal-child.out", "Child", argv[1], (char *)NULL);
		perror ("execl");
		_exit (1); 
	}

	while (1) 
	{
		if (sig_status == 1) 
		{
			printf ("%s: leap year\n", argv[1]);
			return 0;
		}

		if (sig_status == 2) 
		{
			printf ("%s: not leap year\n", argv[1]);
			return 0;
		}

		pause();
	}
	
	return 0;
}
