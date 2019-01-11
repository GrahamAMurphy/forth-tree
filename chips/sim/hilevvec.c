#include <stdio.h>
#include "globals.h"
#include "external.h"

/*
 *	Global data
 */

int genvectors = FALSE;				/* no vectors by default */

static FILE *vecfp = NULL;

static char *Binary();

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
	fprintf(vecfp," 11111");
	fprintf(vecfp," 100 %s",
		Binary(portaddr,'0','1'));
	fprintf(vecfp," ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ\n");
}

void
Vecldphib()
{
	fprintf(vecfp," 01111");
	fprintf(vecfp," 100 %s",
		Binary(portaddr,'0','1'));
	fprintf(vecfp," %s\n",
		Binary(portvalue,'0','1'));
}

void
Vecstphia()
{
	fprintf(vecfp," 11111");
	fprintf(vecfp," 000 %s",
		Binary(portaddr,'0','1'));
	fprintf(vecfp," ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ\n");
}

void
Vecstphib()
{
	fprintf(vecfp," 01111");
	fprintf(vecfp," 000 %s",
		Binary(portaddr,'0','1'));
	fprintf(vecfp," %s\n",
		Binary(portvalue,'L','H'));
}

void
Vecintphia()
{
	fprintf(vecfp," 11111");
	fprintf(vecfp," 110 %s",
		Binary(portaddr,'0','1'));
	fprintf(vecfp," ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ\n");
}

void
Vecintphib()
{
	fprintf(vecfp," 01111");
	fprintf(vecfp," 110 %s",
		Binary(portaddr,'0','1'));
	fprintf(vecfp," %s\n",
		Binary(portvalue,'0','1'));
}

void
Vecdmaphia()
{
	fprintf(vecfp," 11111");
	fprintf(vecfp," Z01 ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
	fprintf(vecfp," ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ\n");
}

void
Vecdmaphib()
{
	fprintf(vecfp," 01111");
	fprintf(vecfp," Z01 ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
	fprintf(vecfp," ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ\n");
}

static char *
Binary(value,zero,one)
int32 value;
char zero, one;
{
	static char buf[33];
	char *bufptr = buf;
	uint32 mask;

	for(mask=0x80000000; mask!=0; mask >>= 1){
		if((value & mask) == 0){
			*bufptr++ = zero;
		} else {
			*bufptr++ = one;
		}
		*bufptr = '\0';
	}
	return(buf);
}
