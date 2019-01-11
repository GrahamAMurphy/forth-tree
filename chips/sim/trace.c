#include <stdio.h>
#include "globals.h"
#include "external.h"

/*
 *	Local data
 */

static FILE *tracefp = NULL;			/* address trace */

FILE *
Traceopen(file)
char *file;
{
	return(tracefp = fopen(file,"wb"));
}

void
Trace(mode)
uint32 mode;
{
	if(tracefp){				/* traces everything */
		uint32 traceaddr = portaddr | mode;
		fwrite((char *)&traceaddr,sizeof(uint32),1,tracefp);
	}
}

void
Logliteral()
{
	/* ignore */
}

void
Traceclose()
{
	if(tracefp) fclose(tracefp);
}
