#include <stdio.h>
#include "globals.h"
#include "external.h"

static void Unixstring();
static int Forthstring();

static int32 fakeioargs[10];
static int argindex = 0;

int32
Fakeio(trapnumber)
int32 trapnumber;
{
	char buf1[256], buf2[256];	/* I/O buffers */
	int result;

	argindex = 0;			/* reset index on each trap */
	switch(trapnumber){
	   case 1:			/* emit ( c -- 0 ) */
		Cmdcharput((int)fakeioargs[0]);
		return(0);
	   case 2:			/* key ( -- c )*/
		return(Cmdcharget());
	   case 3:			/* open ( "file" "mode" -- fp ) */
		Unixstring((uint32)fakeioargs[3],(int)fakeioargs[2],buf1);
		Unixstring((uint32)fakeioargs[1],(int)fakeioargs[0],buf2);
		return((int32)fopen(buf1,buf2));
	   case 4:			/* close ( fp -- ior ) */
		return((int32)fclose((FILE*)fakeioargs[0]));
	   case 5:			/* seek ( addr fp -- ior ) */
		return((int32)fseek((FILE *)fakeioargs[0],
				(long)fakeioargs[1],0));
	   case 6:			/* read ( addr count fp -- actcoun)*/
		result = fread(buf1,sizeof(char),
				(int)fakeioargs[1],(FILE *)fakeioargs[0]);
		buf1[result] = '\0';
		Forthstring(buf1,(uint32)fakeioargs[2]);
		return(result);
	   case 7:			/* fgetc ( fp -- byte ) */
		return(fgetc((FILE *)fakeioargs[0]));
	   case 8:			/* fputc ( byte fp -- ior ) */
		return(fputc((int)fakeioargs[1],(FILE *)fakeioargs[0]));
	   case 9:			/* fexpect ( addr len fp -- len' )*/
		if(fgets(buf1,(int)fakeioargs[1],(FILE *)fakeioargs[0])
				 == 0) return(0);
		return(Forthstring(buf1,(uint32)fakeioargs[2]));
	   case 10:			/* chdir ( "dir" -- ior ) */
		Unixstring((uint32)fakeioargs[1],(int)fakeioargs[0],buf1);
		return(chdir(buf1));
	   case 11:			/* bye ( -- 0 ) */
		quitflag = TRUE;
		return(0);
	   case 12:			/* trace ( mode -- 0 ) */
		Tracemode((int)fakeioargs[0]);
		return(0);
	   case 13:			/* visual ( mode -- 0 ) */
		Visualmode((int)fakeioargs[0]);
		return(0);
	   case 14:			/* set-trap ( addr count -- 0 ) */
		Settrap((uint32)fakeioargs[1],(int)fakeioargs[0]);
		return(0);
	   case 15:			/* cycle-count ( -- count ) */
		return(cycle);
	   case 16:			/* clear-stats ( -- 0 ) */
		Clearstats();
		return(0);
	   default:
		Cmdmsg("Unknown trap number\n"); break;
	}
	return(0);
}

void
Fakeioarg(arg)
int32 arg;
{
	fakeioargs[argindex++] = arg;
}

static void
Unixstring(forthaddr,count,targetstring)
uint32 forthaddr;
int count;
char *targetstring;
{
	for( ; count; count--){
		*targetstring++ = (char)Fetchexternalbyte(forthaddr++);
	}
	*targetstring = '\0';
}

static int
Forthstring(string,forthaddr)
char *string;
uint32 forthaddr;
{
	int length = 0;
	while(*string){
		Storeexternalbyte(forthaddr++, (int32)*string++);
		length++;
	}
	return(length);
}
