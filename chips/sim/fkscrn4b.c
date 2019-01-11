/* Fake out screen update functions for a non-interactive simulator */
#include <stdio.h>
#include "globals.h"
#include "external.h"

#define FWFORMAT "%08x"

#ifdef FORTHPARSE
/*	Forth Interpreter I/O routines */
FILE *OUTPUTFP; /* not used */

int cmd_getc()
{
	return(getc(stdin));
}

int cmd_getline(buf, len)
char *buf;
int len;
{
	return(read_line(buf, len, stdin));
}

void cmd_putc(c)
char c;
{
	putc(c,stdout);
}

void cmd_putline(buf, length)
char *buf;
int length;
{
/* TBD: weirdness! this doesn't add \n, but cmd_getline removes \n */
	fwrite(buf, 1, length, stdout);
}

#else /* lex/yacc command interpreter */

int
Getcmdc(fp)
FILE *fp;
{
	return(getc(fp));
}

#endif

void
Dump(addr,len)
uint32 addr;
int len;
{
	int i;				/* loop counter */

	for(;len>0;len -= 8){
		printf(FWFORMAT,addr);
		printf(":  ");
		for(i=0;i<8;i++){
			printf("%02x ",Fetchexternalbyte(addr++));
		}
		putchar('\n');
	}
}

void
Cmdcharput(c)
int c;
{
	putchar(c);
}

int
Cmdcharget()
{
	return(getc(stdin));
}

void
Cmdmsg(string)
char *string;
{
	fputs(string,stdout);
}

void
Iowrite(addr,value)
uint32 addr;
uint32 value;
{
	printf("I/O %08lx ! %08lx\n",addr,value);
}

void
Ioread(addr,value)
uint32 addr;
uint32 value;
{
	printf("I/O %08lx @ %08lx\n",addr,value);
}

void
Visopen()
{
}

void
Visclose()
{
}

void
Updatescr()
{
}
