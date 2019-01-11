#include <stdio.h>
#include <curses.h>
#include "globals.h"
#include "external.h"
#include "intrnl4a.h"
#include "frmts4.h"

#define STBASEROW	18		/* stack base */
#define PSTACKCOL	47		/* parameter stack */
#define RSTACKCOL	68		/* return stack */
#define LATCHROW	1		/* latches */
#define ALUOUTCOL	0		/* ALU output latch */
#define BARRELCOL	0		/* barrel shifter output latch */
#define HOLDCOL		12		/* hold output latch */
#define DHOLDCOL	24		/* load hold latch */
#define PCCOL		36		/* Program Counter phase A latch */
#define IRCOL		47		/* IR */
#define PORTADCOL	59		/* port address */
#define PORTIOCOL	67		/* port I/O operation */
#define PORTVALCOL	68		/* port value */
#define FLAGROW		10		/* flag */
#define FLAGCOL		13
#define CACHEROW	5		/* parameter and return stacks */
#define PCACHEENCOL	37		/* parameter cache enable bit */
#define RCACHEENCOL	58		/* return cache enable bit */
#define IEROW		12		/* interrupt enable flag */
#define IECOL		1
#define CPCOL		13		/* coprocessor select */
#define PSPROW		8		/* external stack pointers */
#define PSPCOL		37		/* external parameter stack pointer */
#define RSPCOL		58		/* external return stack pointer */
#define YROW		10		/* Y shift register */
#define YCOL		0
#define ABUSROW		5		/* A bus */
#define ABUSCOL		0
#define BBUSROW		5		/* B bus */
#define BBUSCOL		13
#define UDRROW		15		/* UDRs */
#define UDR0COL		0
#define UDR1COL		11
#define UDR2COL		22
#define UDR3COL		33
#define CYCLEROW	19		/* cycle and phase */
#define CYCLECOL	29
#define PHASECOL	48
#define INTROW		18		/* interrupt controller */
#define MASKCOL		1		/* interrupt mask */
#define MODECOL		7		/* interrupt mode */
#define EDGECOL		13		/* interrupt edge */

#define NCMDLINES (LINES-CYCLEROW-1)	/* number of lines in command window */

#define C_ERASE	'\010'			/* erase character */
#define C_KILL  '\025'			/* kill character */ 
#define C_EOF	'\004'			/* end of file marker */

#define FWFORMAT "%08x"			/* display format */

static char cmdbuf[133] = "";		/* command input buffer */
static char *bufptr = cmdbuf;		/* for scanning command line */
static WINDOW *cmdwin;			/* command window */

char * Wfieldyx();
void Pstackov();
void Rstackov();
void Mvprintword();

void
Initcmdwin()
{
	cmdwin = newwin(NCMDLINES,COLS,CYCLEROW+1,0);
	scrollok(cmdwin,TRUE);		/* enable scrolling in window */
	wclear(cmdwin);			/* clear window */
}

int
Getcmdc(fp)
FILE *fp;				/* fp ignored for now */
{
	if(*bufptr == '\0'){
		if((bufptr=Wfieldyx(cmdwin,cmdbuf,132,NCMDLINES-1,0)) == NULL){
			return(EOF);
		}
	}
	return((int) (*bufptr++));
}

char *
Wfieldyx(win,str,len,y,x)
WINDOW *win;
char *str;
int len;
int y,x;
{
	int i;				/* loop counter */
	char *ptr;			/* buffer pointer */
	char c;				/* input character */
	int row, col;			/* temporary row and column number */

	wmove(win,y,x);
	wrefresh(win);
	ptr=str; i=0;
	while(i<len && (c=wgetch(win))!='\n'){
		if(c==C_ERASE){
			if(i>0){
				getyx(win,row,col);
				wmove(win,row,col-1);
				waddch(win,' ');
				wmove(win,row,col-1);
				ptr--; i--;
			}
		} else if(c==C_KILL){
			wmove(win,y,x);
			wclrtoeol(win);
			ptr=str; i=0;
		} else if(c==C_EOF){
			return(NULL);
		} else {
			waddch(win,c);
			*ptr++=c;  i++;
		}
		wrefresh(win);
	}
	waddch(win,'\n');		/* scroll window */
	wrefresh(win);
#ifdef sun
	wclrtoeol(win);			/* there is a bug in sun curses*/
#endif
	*ptr++ = '\n';
	*ptr='\0';			/* put null at end of string */
	return(str);
}

void
Dump(addr,len)
int addr, len;
{
	int i;				/* loop counter */

	for(;len>0;len -= 4){
		wprintw(cmdwin,FWFORMAT,addr);
		wprintw(cmdwin,":  ");
		for(i=0;i<4;i++){
			wprintw(cmdwin,"%08x ", Fetchexternal(addr++));
		}
		waddch(cmdwin,'\n');
		wrefresh(cmdwin);
	}
}

void
Iowrite(addr,value)
unsigned int addr;
int value;
{
	wprintw(cmdwin,"I/O %08x ! %08x\n",addr,value);
	wrefresh(cmdwin);
}

void
Ioread(addr,value)
unsigned int addr;
int value;
{
	wprintw(cmdwin,"I/O %08x @ %08x\n",addr,value);
	wrefresh(cmdwin);
}

void
Cmdcharput(c)
int c;
{
	waddch(cmdwin,c);
#ifdef sun
	if(c=='\n') wclrtoeol(cmdwin);	/* there is a bug in sun curses*/
#endif
	wrefresh(cmdwin);
}

int
Cmdcharget()
{
	return(wgetch(cmdwin));
}

void
Cmdmsg(string)
char *string;
{
	wprintw(cmdwin,"%s",string);
	wrefresh(cmdwin);
}

void
Visopen()
{
	initscr();			/* initialize curses */
	crmode();			/* cbreak mode */
	noecho();			/* don't let driver echo */
	clear();			/* clear screen */
	Initcmdwin();			/* initialize command window */
	mvaddstr(0,0,
"alu/barrel    hold      loaddata       pc         ir          port"
);
	mvaddstr(1,0,
"xxxxxxxx    xxxxxxxx    xxxxxxxx    xxxxxxxx   xxxxxxxx    xxxxxxxx@xxxxxxxx");
	mvaddstr(2,0,
"xxxxxxxx");
	mvaddstr(3,0,
"                                    parameter  xxxxxxxx     return  xxxxxxxx");
	mvaddstr(4,0,
"  abus         bbus                    stack:  xxxxxxxx     stack:  xxxxxxxx");
	mvaddstr(5,0,
"xxxxxxxx     xxxxxxxx                          xxxxxxxx             xxxxxxxx");
	mvaddstr(6,0,
"                                               xxxxxxxx             xxxxxxxx");
	mvaddstr(7,0,
"                                     overflow  xxxxxxxx   overflow  xxxxxxxx");
	mvaddstr(8,0,
"                                               xxxxxxxx             xxxxxxxx");
	mvaddstr(9,0,
"    y       flag                               xxxxxxxx             xxxxxxxx");
	mvaddstr(10,0,
"xxxxxxxx     x                                 xxxxxxxx             xxxxxxxx");
	mvaddstr(11,0,
"ie          cp                                 xxxxxxxx             xxxxxxxx");
	mvaddstr(12,0,
" x           x                                 xxxxxxxx             xxxxxxxx");
	mvaddstr(13,0,
"                                               xxxxxxxx             xxxxxxxx");
	mvaddstr(14,0,
"  udr0       udr1       udr2       udr3        xxxxxxxx             xxxxxxxx");
	mvaddstr(15,0,
"xxxxxxxx   xxxxxxxx   xxxxxxxx   xxxxxxxx      xxxxxxxx             xxxxxxxx");
	mvaddstr(16,0,
"                                               xxxxxxxx             xxxxxxxx");
	mvaddstr(17,0,
"mask  mode  edge                               xxxxxxxx             xxxxxxxx");
	mvaddstr(18,0,
" xx    xx    xx                                xxxxxxxx             xxxxxxxx");
	mvaddstr(19,0,
"-----------------------cycle -------------phase ----------------------------");
}

void
Visclose()
{
	endwin();			/* shut down curses */
}

void
Updatescr()
{
	int i;				/* loop counter */
	static int torrow = LATCHROW, torcol = HOLDCOL+8;

	for(i=0;i<ptop;i++){
		Mvprintword(STBASEROW-i,PSTACKCOL,pstack[i]);
	}
	standout();
	Mvprintword(STBASEROW-ptop,PSTACKCOL,pstack[ptop]);
	standend();
	for(i=ptop+1;i<PSTACKSIZE;i++){
		Mvprintword(STBASEROW-i,PSTACKCOL,pstack[i]);
	}
	Pstackov();
	for(i=0;i<rtop;i++){
		Mvprintword(STBASEROW-i,RSTACKCOL,rstack[i]);
	}
	standout();
	Mvprintword(STBASEROW-rtop,RSTACKCOL,rstack[rtop]);
	standend();
	for(i=rtop+1;i<RSTACKSIZE;i++){
		Mvprintword(STBASEROW-i,RSTACKCOL,rstack[i]);
	}
	Rstackov();
	Mvprintword(LATCHROW,ALUOUTCOL,aluout);
	Mvprintword(LATCHROW+1,BARRELCOL,barrelout);
	Mvprintword(LATCHROW,HOLDCOL,hold);
	Mvprintword(LATCHROW,DHOLDCOL,dhold);
	Mvprintword(LATCHROW,PCCOL,pca);
	Mvprintword(LATCHROW,IRCOL,irb.irvalue);
	Mvprintword(LATCHROW,PORTADCOL,portaddr);
	if(portio == PORT_RD)
		mvaddch(LATCHROW,PORTIOCOL,'@');
	else
		mvaddch(LATCHROW,PORTIOCOL,'!');
	Mvprintword(LATCHROW,PORTVALCOL,portvalue);
	mvaddch(torrow,torcol,' ');	/* erase old tor position */
	if(torptr == &hold){
		torrow = LATCHROW; torcol = HOLDCOL+8;
	} else if(torptr == &aluout){
		torrow = LATCHROW; torcol = ALUOUTCOL+8;
	} else if(torptr == &barrelout){
		torrow = LATCHROW+1; torcol = BARRELCOL+8;
	} else {
		torrow = LATCHROW; torcol = DHOLDCOL+8;
	}
	mvaddch(torrow,torcol,'*');	/* indicate new tor position */
	move(CACHEROW,PCACHEENCOL);
		printw(ssw.fields.pcache ? "enabled " : "disabled");
	move(CACHEROW,RCACHEENCOL);
		printw(ssw.fields.rcache ? "enabled " : "disabled");
	move(FLAGROW,FLAGCOL); printw("%1d",flag);
	Mvprintword(YROW,YCOL,y);
	move(IEROW,IECOL); printw("%1d",psw.fields.ie);
	move(IEROW,CPCOL); printw("%1d",cpselect);
	Mvprintword(PSPROW,PSPCOL,externalpsp);
	Mvprintword(PSPROW,RSPCOL,externalrsp);
	Mvprintword(ABUSROW,ABUSCOL,abus);
	Mvprintword(BBUSROW,BBUSCOL,bbus);
	Mvprintword(UDRROW,UDR0COL,udr0);
	Mvprintword(UDRROW,UDR1COL,udr1);
	Mvprintword(UDRROW,UDR2COL,udr2);
	Mvprintword(UDRROW,UDR3COL,udr3);
	move(INTROW,MASKCOL); printw("%02x",intmask);
	move(INTROW,MODECOL); printw("%02x",intmode);
	move(INTROW,EDGECOL); printw("%02x",intedgedetected);
	move(CYCLEROW,CYCLECOL);
	printw("%d",cycle);
	move(CYCLEROW,PHASECOL);
	printw("%d",phase);
	refresh();
}

void
Pstackov()
{
	static int lastpover=0, lastpunder=0;

	if(pover!=lastpover){
		mvaddch(STBASEROW-lastpover,PSTACKCOL-1,' ');
		mvaddch(STBASEROW-lastpover,PSTACKCOL+8,' ');
		mvaddch(STBASEROW-lastpunder,PSTACKCOL-1,' ');
		mvaddch(STBASEROW-lastpunder,PSTACKCOL+8,' ');
		mvaddch(STBASEROW-pover,PSTACKCOL-1,'<');
		mvaddch(STBASEROW-pover,PSTACKCOL+8,'>');
		mvaddch(STBASEROW-punder,PSTACKCOL-1,'>');
		mvaddch(STBASEROW-punder,PSTACKCOL+8,'<');
		lastpover=pover; lastpunder=punder;
	}
}

void
Rstackov()
{
	static int lastrover=0, lastrunder=0;

	if(rover!=lastrover){
		mvaddch(STBASEROW-lastrover,RSTACKCOL-1,' ');
		mvaddch(STBASEROW-lastrover,RSTACKCOL+8,' ');
		mvaddch(STBASEROW-lastrunder,RSTACKCOL-1,' ');
		mvaddch(STBASEROW-lastrunder,RSTACKCOL+8,' ');
		mvaddch(STBASEROW-rover,RSTACKCOL-1,'<');
		mvaddch(STBASEROW-rover,RSTACKCOL+8,'>');
		mvaddch(STBASEROW-runder,RSTACKCOL-1,'>');
		mvaddch(STBASEROW-runder,RSTACKCOL+8,'<');
		lastrover=rover; lastrunder=runder;
	}
}

void
Mvprintword(y,x,value)
int y, x;
forthword value;
{
	move(y,x);
	printw(FWFORMAT,value);
}
