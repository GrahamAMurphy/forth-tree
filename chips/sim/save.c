#include <stdio.h>
#include "globals.h"
#include "external.h"

/*
 *	Global data
 */

static FILE *savefp = NULL;

FILE *
Saveopen(file)
char *file;
{
	savefp = fopen(file,"wb");
	return(savefp);
}

void
Saveclose()
{
	if(savefp) fclose(savefp);
}

void
Resetobj()
{
	rewind(savefp);
}

void
Savecore(src,len)
unsigned char *src;
uint32 len;
{
	if(savefp) fwrite((char *)src,(int)len,1,savefp);
}
