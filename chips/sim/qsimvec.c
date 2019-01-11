#include <stdio.h>
#include "globals.h"
#include "external.h"

/*
 *	Global data
 */

int genvectors = FALSE;				/* no vectors by default */

static FILE *vecfp = NULL;

FILE *
Vecopen(file)
char *file;
{
	vecfp = fopen(file,"w");
	return(vecfp);
}

void
Vecclose()
{
	if(vecfp) fclose(vecfp);
}

void
Vecldphia()
{
}

void
Vecldphib()
{
	fprintf(vecfp, "\
run 100\n\
if ^($signal_value('busidle',$sim_time) <> \"%d\") then\n\
\twrite line \"busidle should be %d at \" ^$sim_time\n\
\tpause\n\
end if\n",		busidle, busidle);
	if(!busidle){
		fprintf(vecfp, "\
if ^($signal_value('extabus',$sim_time) <> \"%08x\") then\n\
\twrite line \"extabus should be %08x at \" ^$sim_time\n\
\tpause\n\
end if\n\
force datain %08x\n",
			portaddr, portaddr, portvalue);
	}
}

void
Vecstphia()
{
}

void
Vecstphib()
{
	fprintf(vecfp, "\
run 100\n\
if ^($signal_value('busidle',$sim_time) <> \"%d\") then\n\
\twrite line \"busidle should be %d at \" ^$sim_time\n\
\tpause\n\
end if\n",		busidle, busidle);
	if(!busidle){
		fprintf(vecfp, "\
if ^($signal_value('extabus',$sim_time) <> \"%08x\") then\n\
\twrite line \"extabus should be %08x at \" ^$sim_time\n\
\tpause\n\
end if\n\
if ^($signal_value('dataout',$sim_time) <> \"%08x\") then\n\
\twrite line \"dataout should be %08x at \" ^$sim_time\n\
\tpause\n\
end if\n",
			portaddr, portaddr,
			portvalue, portvalue);
	}
}

void
Vecintphia()
{
}

void
Vecintphib()
{
	fprintf(vecfp, "\
run 100\n\
if ^($signal_value('extabus',$sim_time) <> \"%08x\") then\n\
\twrite line \"extabus should be %08x at \" ^$sim_time\n\
\tpause\n\
end if\n\
force datain %08x\n",
			portaddr, portaddr, portvalue);
}

void
Vecdmaphia()
{
}

void
Vecdmaphib()
{
}

void
Vecinton(n)
int n;
{
	fprintf(vecfp, "\
force interrupt 1\n\
force vec %x\n",
	n);		/* BUG: assumes no higher priority int */
}

void
Vecintoff(n)
int n;
{
	fprintf(vecfp, "\
force interrupt 0\n"
	);
}
