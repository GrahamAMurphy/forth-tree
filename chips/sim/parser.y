%{

#include "globals.h"

#define yyerror(s) Cmdmsg(s)

typedef union {
	long num;
	int mode;
} YYSTYPE;

%}

%token		RESET
%token		DUMP
%token		SAVE
%token		STARTAT
%token		WAIT
%token		INTON
%token		INTOFF
%token		DMASTART
%token		DMAEND
%token		PHASE
%token		CYCLE
%token		SIM
%token		CYCLES
%token		TIMES
%token		CLEAR
%token		TRACE
%token		VECTORS
%token		VISUAL
%token <mode>	MODE
%token <num>	NUM
%token		TERM

%%

program	:	/* null program */

	|	program statement 

	;

statement:	RESET TERM {
			Resetregs();		/* reset machine state */
		}

	|	DUMP NUM NUM TERM {		/* dump memory */
			Dump((uint32)$2,(int)$3);
		}

	|	SAVE TERM {			/* save core image */
			Resetobj();		/* rewind core file */
			Saveobj();
		}

	|	STARTAT NUM TERM {		/* start simulation at X */
			Startat((uint32)$2);
		}

	|	WAIT TERM {
			Waitcycle();		/* wait cycle */
		}

	|	INTON TERM {			/* activate interrupt */
			Interrupton(0);		/* default to int 0 */
		}

	|	INTON NUM TERM {		/* activate interrupt */
			Interrupton((int)$2);
		}

	|	INTOFF TERM {			/* deactivate interrupt */
			Interruptoff(0);	/* default to int 0 */
		}

	|	INTOFF NUM TERM {		/* deactivate interrupt */
			Interruptoff((int)$2);
		}

	|	DMASTART TERM {			/* start DMA */
			Dmastart();
		}

	|	DMAEND TERM {			/* end DMA */
			Dmaend();
		}

	| 	PHASE TERM {			/* single phase mode */
			Phase();
		}

	|	CYCLE TERM {			/* single cycle mode */
			Cycle();
		}

	|	SIM NUM TERM {			/* non-interactive sim. */
			Sim((uint32)$2,(int)1);
		}

	|	SIM NUM NUM TIMES TERM {	/* non-interactive sim. */
			Sim((uint32)$2,(int)$3);
		}

	|	SIM NUM CYCLES TERM {		/* non-interactive sim. */
			Simcycles((int)$2);
		}

	|	CLEAR TERM {			/* clear statistics */
			Clearstats();
		}

	|	TRACE MODE TERM {		/* control address tracing */
			Tracemode($2);
		}

	|	VECTORS MODE TERM {		/* control vector generation */
			Vectormode($2);
		}

	|	VISUAL MODE TERM {		/* control screen update */
			Visualmode($2);
		}

	|	NUM TERM {			/* stack a number */
			Stack((int32)$1);
		}

	|	TERM {				/* single step */
			Singlestep();
		}

	|	error TERM {
			yyerrok;
			yyclearin;
		}

	;

%%

#include "scan.c"
