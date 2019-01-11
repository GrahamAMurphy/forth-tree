/* (c) 1992 Johns Hopkins University / Applied Physics Laboratory */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef sun
#include <inttypes.h>
#include <sys/termios.h>
#endif
#include "forth.h"
#include "io.h"				/* implement this interface here */

static void my_handler();

FILE *OUTPUTFP;				/* fp for interpreter output */

int
main(int argc,char *argv[])
{
	(void)signal(SIGBUS,my_handler);
	(void)signal(SIGSEGV,my_handler);
	init_forth();
	do_forth(argc,argv);
	return(0);
}

static void
my_handler(void)
{
	printf("ouch!\n");
	exit(1);
}

/*	Forth Interpreter I/O routines */
int
cmd_getc(void)
/* Special case version of fkey (read from standard input).  BUG: this
   assumes that the standard input is never closed then reopened.
 */
{
#ifdef sun
	static int firstcall = 1;	/* true only on first call */
	static int tty;			/* set true if stdin is a tty */
	static struct termios ttyorig;
	static struct termios ttymod;
	int c;

	if(firstcall){
		if(tty = isatty(fileno(stdin))){
			ioctl(fileno(stdin), TCGETS, &ttyorig);
			ttymod = ttyorig;
			ttymod.c_lflag &= ~(ISIG | ICANON | ECHO);
			ttymod.c_cc[VMIN] = 1;
			ttymod.c_cc[VTIME] = 0;
		}
		firstcall = 0;
	}

	if(tty) ioctl(fileno(stdin), TCSETS, &ttymod);
	c = getc(stdin);
	if(tty) ioctl(fileno(stdin), TCSETS, &ttyorig);

	return(c);
#else
	return(getchar());
#endif
}

int
cmd_getline(char *buf, int len)
{
	return(read_line(buf, len, stdin));
}

void
cmd_putc(unsigned char c)
{
	putc(c,OUTPUTFP);
}

void
cmd_putline(char *buf, int length)
{
/* TBD: weirdness! this doesn't add \n, but cmd_getline removes \n */
	fwrite(buf, 1, length, OUTPUTFP);
}

void
cmd_ioinit(void)
{
	OUTPUTFP = stdout;
}

