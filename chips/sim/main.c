#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include "globals.h"
#include "external.h"

#ifdef FORTHPARSE
static void Installcalls();
static void Install();
static void Save();
#endif

static jmp_buf interpenv;		/* save environment for longjmp */

int summarize = FALSE;			/* set true for statistics */

main(argc,argv)
int argc;
char *argv[];
{
	Startup(argc,argv);
	signal(SIGINT,Quit);		/* catch SIGINT */
	Visopen();			/* init. curses and display skeleton*/
	Resetregs();			/* initialize machine state */
	Updatescr();
#ifdef FORTHPARSE
	initforth();
	Installcalls();
#endif
	setjmp(interpenv);
#ifdef FORTHPARSE
	evalforth("abort");
#else
	yyparse();
#endif
	Vecclose();
	Traceclose();
	Saveclose();
	Visclose();
	if(summarize) Summary();
	return(0);
}

void
Quit()
{
	Updatescr();
	longjmp(interpenv,0);
}

#ifdef FORTHPARSE
static void
Installcalls()
{
	Install("0 %x 0 foreign: reset", Resetregs);
	Install("2 %x 0 foreign: tdump", Dump);
	Install("0 %x 0 foreign: save", Save);
	Install("1 %x 0 foreign: startat", Startat);
	Install("0 %x 0 foreign: wait", Waitcycle);
	Install("1 %x 0 foreign: inton", Interrupton);
	Install("1 %x 0 foreign: intoff", Interruptoff);
	Install("0 %x 0 foreign: dmastart", Dmastart);
	Install("0 %x 0 foreign: dmaend", Dmaend);
	Install("0 %x 0 foreign: phase", Phase);
	Install("0 %x 0 foreign: cycle", Cycle);
	Install("2 %x 0 foreign: sim-times", Sim);
	evalforth(": sim 1 sim-times ;");
	Install("1 %x 0 foreign: cycles", Simcycles);
	Install("0 %x 0 foreign: clear", Clearstats);
	Install("1 %x 0 foreign: trace", Tracemode);
	Install("1 %x 0 foreign: vectors", Vectormode);
	Install("1 %x 0 foreign: visual", Visualmode);
	Install("1 %x 0 foreign: ->s", Stack);
	Install("0 %x 0 foreign: s", Singlestep);
}

static void
Install(s,f)
char *s;
void (*f)();
{
	char buf[256];

	sprintf(buf,s,f);
	evalforth(buf);
}

static void
Save()
{
	Resetobj();		/* rewind core file */
	Saveobj();
}
#endif
