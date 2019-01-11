#include "globals.h"

#define MSBSET(x) ((x)&0x80000000)

int32
Addop(tinput,binput,cin,flagptr)
int32 tinput, binput;
int cin;
short *flagptr;
{
	int32 result;

	if(cin) result = tinput + binput + 1;
	else	result = tinput + binput;
	if(MSBSET( (tinput&binput&~result) | (~tinput&~binput&result) ))
		*flagptr |= 0x2;
	if(MSBSET( (tinput&binput) | (~result&(tinput|binput)) ))
		*flagptr |= 0x1;
	return(result);
}
