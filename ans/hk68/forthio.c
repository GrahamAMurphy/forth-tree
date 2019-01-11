#include "io.h"

/*
 *	forth io routines: expect and type
 */

#define CNTLH	'\010'
#define CNTLU	'\025'

int
forthread(addr,len,ttyno)
register unsigned char *addr;
register unsigned len;
int ttyno;
{
	register unsigned i=0;
	register unsigned char c;
	register struct channel *chan = chans[ttyno];

	while(i<len-1){
		switch(c=input(chan)){
		   case '\r':	output(chan,' ');
			     	return(i);
		   case CNTLH: if(i){
					output(chan,CNTLH);
					output(chan,' ');
					output(chan,CNTLH);
					i--;
				} break;
		   case CNTLU:	output(chan,'\r');
				output(chan,'\n');
				i=0; break;
		   default:	output(chan,c);
				*(addr+(i++))=c; break;
		}
	}
	return(i);
}

void
forthwrite(addr,count,ttyno)
register unsigned char *addr;
register unsigned count;
int ttyno;
{
	register struct channel *chan = chans[ttyno];

	while(count-- > 0){
		output(chan,*addr++);
	}
}

void
forthemit(c,ttyno)
char c;
int ttyno;
{
	output(chans[ttyno],c);
}

#define  FEOF  '\004'

void
forthload(addr,ttyno)
register unsigned char *addr;
int ttyno;
{
	register struct channel *chan =  chans[ttyno];

	while((*addr++ = input(chan)) != FEOF)
		;
}

int
forthfkey(ttyno)
int ttyno;
{
	register struct channel *chan = chans[ttyno];

	return(input(chan));
}

int
forthqfkey(ttyno)
int ttyno;
{
	return(chans[ttyno]->infifo.count);
}
