#include <stdio.h>
#include "globals.h"
#include "external.h"
#include "intrnl3.h"
#include "frmts3.h"
#include "masks3.h"

#define TRUE (1)
#define FALSE (0)

/*************************************************************************
 * The following creates a log file of the literals seen in the simulation.
 * Of course, at this level we cannot tell what is a literal without
 * looking at succesive lah and lal/load/store instructions.  Here are the
 * cases:
 * 	*, lal/load/store:	non-lah followed by lal/load/store
 * 	lah, lal:		lal modifies preceding lah literal
 * 	lah, load/store:	load/store modifies preceding lah literal
 *					ignores literals in I/O trap space
 * 	lah, lah:		two different literals (second may be
 * 					subsequently modified)
 * The following case is not handled:
 * 	lah, *, lal/load/store:	intervening instruction
 * Also, the technique used to ignore I/O traps to simulator fails
 * if the simulated Forth I/O interface has not been optimized.
 * 
 * Zero literals are not reported.
 */

static FILE *tracefp = NULL;

FILE *
Traceopen(file)
char *file;
{
	return(tracefp = fopen(file,"w"));
}

void
Trace()
{
	/* ignore */
}

void
Logliteral(opcode)
union opstruct opcode;
{
#define LOG(y) if(y) fprintf(tracefp, "%08x\n",y)
	static int pending = FALSE;	/* last instruction was lah */
	static union opstruct popcode;	/* previous opcode */

	if(tracefp == NULL) return;	/* no log file open */
	if(!pending){
		switch(opcode.irbranch.type){
		   case B_LAL:
		   case B_LOAD:
		   case B_STORE:
			LOG(opcode.irload.offset);
			break;
		   case B_LAH:
			popcode = opcode;
			pending = TRUE;
			break;
		   default:
			break;
		}
	} else {
		switch(opcode.irbranch.type){
		   case B_LAL:
		   case B_LOAD:
		   case B_STORE:
			if(popcode.irload.reg2 == opcode.irload.reg1){
				unsigned int literal =
					(popcode.irload.offset<<16)
						+ opcode.irload.offset;
				if(opcode.irbranch.type == B_LAL){
					LOG(literal);
				} else {
					if(literal <= TRAPSPACE) LOG(literal);
				}
			} else {
				LOG(popcode.irload.offset<<16);
				LOG(opcode.irload.offset);
			}
			pending = FALSE;
			break;
		   case B_LAH:
			LOG(popcode.irload.offset<<16);
			popcode = opcode;
			break;
		   default:
			LOG(popcode.irload.offset<<16);
			pending = FALSE;
			break;
		}
	}
}

void
Traceclose()
{
	if(tracefp) fclose(tracefp);
	tracefp = NULL;
}
