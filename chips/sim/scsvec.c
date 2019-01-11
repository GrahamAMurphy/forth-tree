#include <stdio.h>
#include "globals.h"
#include "external.h"

#define FUDGE 50				/* added to current time
						   to sync with Genesil
						   simulator */

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
	fprintf(vecfp,"@%d <1111\n",
		10*cycle + FUDGE);
	fprintf(vecfp,"     ................................\n");
	fprintf(vecfp,"   >100%s\n",
		Binary(portaddr));
	fprintf(vecfp,"    ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ;\n");
}

void
Vecldphib()
{
	fprintf(vecfp,"@%d <1111\n",
		10*cycle - 5 + FUDGE);
	fprintf(vecfp,"     %s\n",
		Binary(portvalue));
	fprintf(vecfp,"   >100%s\n",
		Binary(portaddr));
	fprintf(vecfp,"    ................................;\n");
}

void
Vecstphia()
{
	fprintf(vecfp,"@%d <1111\n",
		10*cycle + FUDGE);
	fprintf(vecfp,"     ................................\n");
	fprintf(vecfp,"   >000%s\n",
		Binary(portaddr));
	fprintf(vecfp,"    %s;\n",
		Binary(portvalue));
}

void
Vecstphib()
{
	fprintf(vecfp,"@%d <1111\n",
		10*cycle - 5 + FUDGE);
	fprintf(vecfp,"     ................................\n");
	fprintf(vecfp,"   >000%s\n",
		Binary(portaddr));
	fprintf(vecfp,"    %s;\n",
		Binary(portvalue));
}

void
Vecintphia()
{
	fprintf(vecfp,"@%d <1111\n",
		10*cycle + FUDGE);
	fprintf(vecfp,"     ................................\n");
	fprintf(vecfp,"   >110%s\n",
		Binary(portaddr));
	fprintf(vecfp,"    ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ;\n");
}

void
Vecintphib()
{
	fprintf(vecfp,"@%d <1111\n",
		10*cycle - 5 + FUDGE);
	fprintf(vecfp,"     %s\n",
		Binary(portvalue));
	fprintf(vecfp,"   >110%s\n",
		Binary(portaddr));
	fprintf(vecfp,"    ................................;\n");
}

void
Vecdmaphia()
{
	fprintf(vecfp,"@%d <1111\n",
		10*cycle + FUDGE);
	fprintf(vecfp,"     ................................\n");
	fprintf(vecfp,"   >Z01ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ\n");
	fprintf(vecfp,"    ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ;\n");
}

void
Vecdmaphib()
{
	fprintf(vecfp,"@%d <1111\n",
		10*cycle - 5 + FUDGE);
	fprintf(vecfp,"     ................................\n");
	fprintf(vecfp,"   >Z01ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ\n");
	fprintf(vecfp,"    ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ;\n");
}

static char *
Binary(value)
int32 value;
{
	static char buf[33];
	char *bufptr = buf;
	uint32 mask;

	for(mask=0x80000000; mask!=0; mask >>= 1){
		if((value & mask) == 0){
			*bufptr++ = '0';
		} else {
			*bufptr++ = '1';
		}
		*bufptr = '\0';
	}
	return(buf);
}
