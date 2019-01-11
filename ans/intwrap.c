/* (c) 1992 Johns Hopkins University / Applied Physics Laboratory */
#include <signal.h>

unsigned int
lock()
{
	return(sigblock(sigmask(SIGALRM)));
}

void
unlock(mask)
unsigned int mask;
{
	sigsetmask(mask);
}

void
startclock()
{
	void alarmhandler();

	signal(SIGALRM,alarmhandler);
	alarm(1);
}
