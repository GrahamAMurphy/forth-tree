#include <stdio.h>
#include <curses.h>
#include <varargs.h>
#include "globals.h"
#include "external.h"
#include "intrnl4c.h"
#include "frmts4c.h"

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
#define PORTADCOL	58		/* port address */
#define PORTIOCOL	66		/* port I/O operation */
#define PORTVALCOL	68		/* port value */
#define FLAGROW		10		/* flag */
#define FLAGCOL		13
#define STCACHEROW	5		/* parameter and return stacks */
#define PCACHEENCOL	37		/* parameter cache enable bit */
#define RCACHEENCOL	58		/* return cache enable bit */
#define IEROW		12		/* interrupt enable flag */
#define IECOL		1
#define CACHEROW	12		/* cache enable flag */
#define CACHECOL	13
#define CPCOL		13		/* coprocessor select */
#define PSPROW		8		/* external stack pointers */
#define PSPCOL		37		/* external parameter stack pointer */
#define RSPCOL		58		/* external return stack pointer */
#define YROW		10		/* Y shift register */
#define YCOL		0
#define STALLROW	10		/* stall indicator */
#define STALLCOL	24
#define ABUSROW		5		/* A bus */
#define ABUSCOL		24
#define DBUSROW		6		/* D bus */
#define DBUSCOL		24
#define TBUSROW		5		/* T bus */
#define TBUSCOL		0
#define BBUSROW		6		/* B bus */
#define BBUSCOL		0
#define WBUSROW		5		/* W bus */
#define WBUSCOL		12
#define UDRROW		15		/* UDRs */
#define UDR0COL		0
#define UDR1COL		11
#define UDR2COL		22
#define UDR3COL		33
#define CYCLEROW	19		/* execution and stall cycles */
#define CYCLECOL	29
#define STALLCYCCOL	48
#define INTROW		18		/* interrupt controller */
#define MASKCOL		1		/* interrupt mask */
#define MODECOL		7		/* interrupt mode */
#define EDGECOL		13		/* interrupt edge */
#define LEVLCOL		19		/* interrupt level */

#define NCMDLINES (LINES-CYCLEROW-1)	/* number of lines in command window */

#define C_EOF	'\004'			/* end of file marker */

#define FWFORMAT "%08lx"		/* display format */

static WINDOW *cmdwin;			/* command window */

static int c_erase;			/* character erase character */
static int c_kill;			/* line kill character */

static int Wfieldyx();
static void Pstackov();
static void Rstackov();
static void Mvprintword();

void
Initcmdwin()
{
	cmdwin = newwin(NCMDLINES,COLS,CYCLEROW+1,0);
	scrollok(cmdwin,TRUE);		/* enable scrolling in window */
	wclear(cmdwin);			/* clear window */
}

#ifdef FORTHPARSE
/*	Forth Interpreter I/O routines */
FILE *OUTPUTFP; /* not used */

int cmd_getc()
{
	return(Cmdcharget(stdin));
}

int cmd_getline(buf, len)
char *buf;
int len;
{
	return(Wfieldyx(cmdwin,buf,len));
}

void cmd_putc(c)
char c;
{
	Cmdcharput(c);
}

void cmd_putline(buf, length)
char *buf;
int length;
{
	while(length-- >0 ) Cmdcharput(*buf++);
}

#else /* lex/yacc command interpreter */

int
Getcmdc(fp)
FILE *fp;				/* fp ignored for now */
{
	static char cmdbuf[133] = "";	/* command input buffer */
	static char *bufptr = cmdbuf;	/* for scanning command line */
	if(*bufptr == '\0'){
		wmove(cmdwin,NCMDLINES-1,0);/* position cursor at bottom left */
		wrefresh(cmdwin);
		if(Wfieldyx(cmdwin,cmdbuf,132) == 0){
			return(EOF);
		}
		bufptr = cmdbuf;
	}
	return((int) (*bufptr++));
}
#endif

static int
Wfieldyx(win,str,len)
WINDOW *win;
char *str;
int len;
{
	int i;				/* loop counter */
	char *ptr;			/* buffer pointer */
	char c;				/* input character */
	int orow, ocol;			/* original row and column number */
	int row, col;			/* temporary row and column number */

	getyx(win,orow,ocol);
	ptr=str; i=0;
	while(i<len && (c=wgetch(win))!=C_EOF){
		if(c==c_erase){
			if(i>0){
				getyx(win,row,col);
				wmove(win,row,col-1);
				waddch(win,' ');
				wmove(win,row,col-1);
				ptr--; i--;
			}
		} else if(c==c_kill){
			wmove(win,orow,ocol);
			wclrtoeol(win);
			ptr=str; i=0;
		} else if(c=='\n'){
			*ptr++=c; i++; break;
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
	*ptr = '\0';			/* null terminated */
	return(i);
}

void
Dump(addr,len)
uint32 addr;
int len;
{
	int i;				/* loop counter */

	for(;len>0;len -= 8){
		wprintw(cmdwin,FWFORMAT,addr);
		wprintw(cmdwin,":  ");
		for(i=0;i<8;i++){
			wprintw(cmdwin,"%02x ", Fetchexternalbyte(addr++));
		}
		waddch(cmdwin,'\n');
		wrefresh(cmdwin);
	}
}

void
Iowrite(addr,value)
uint32 addr;
uint32 value;
{
	wprintw(cmdwin,"I/O %08lx ! %08lx\n",addr,value);
	wrefresh(cmdwin);
}

void
Ioread(addr,value)
uint32 addr;
uint32 value;
{
	wprintw(cmdwin,"I/O %08lx @ %08lx\n",addr,value);
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
	c_erase = erasechar();		/* character erase character */
	c_kill = killchar();		/* line kill character */
	Initcmdwin();			/* initialize command window */
	mvaddstr(0,0,
"alu/barrel    hold      loaddata       pc         ir             port"
);
	mvaddstr(1,0,
"xxxxxxxx    xxxxxxxx    xxxxxxxx    xxxxxxxx   xxxxxxxx   xxxxxxxx@ xxxxxxxx");
	mvaddstr(2,0,
"xxxxxxxx");
	mvaddstr(3,0,
"                                    parameter  xxxxxxxx     return  xxxxxxxx");
	mvaddstr(4,0,
"tbus/bbus     wbus      abus/dbus      stack:  xxxxxxxx     stack:  xxxxxxxx");
	mvaddstr(5,0,
"xxxxxxxx    xxxxxxxx    xxxxxxxx               xxxxxxxx             xxxxxxxx");
	mvaddstr(6,0,
"xxxxxxxx                xxxxxxxx               xxxxxxxx             xxxxxxxx");
	mvaddstr(7,0,
"                                     overflow  xxxxxxxx   overflow  xxxxxxxx");
	mvaddstr(8,0,
"                                               xxxxxxxx             xxxxxxxx");
	mvaddstr(9,0,
"    y       flag                               xxxxxxxx             xxxxxxxx");
	mvaddstr(10,0,
"xxxxxxxx     x                                 xxxxxxxx             xxxxxxxx");
	mvaddstr(11,0,
"ie          cache                              xxxxxxxx             xxxxxxxx");
	mvaddstr(12,0,
" x           x                                 xxxxxxxx             xxxxxxxx");
	mvaddstr(13,0,
"                                               xxxxxxxx             xxxxxxxx");
	mvaddstr(14,0,
"   u0         u1         u2         u3         xxxxxxxx             xxxxxxxx");
	mvaddstr(15,0,
"xxxxxxxx   xxxxxxxx   xxxxxxxx   xxxxxxxx      xxxxxxxx             xxxxxxxx");
	mvaddstr(16,0,
"                                               xxxxxxxx             xxxxxxxx");
	mvaddstr(17,0,
"mask  mode  edge  level                        xxxxxxxx             xxxxxxxx");
	mvaddstr(18,0,
" xx    xx    xx    xx                          xxxxxxxx             xxxxxxxx");
	mvaddstr(19,0,
"-----------------------cycle -------------stall ----------------------------");
	wmove(cmdwin,NCMDLINES-1,0);	/* position cursor at bottom left */
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
	Mvprintword(LATCHROW,PCCOL,pc);
	Mvprintword(LATCHROW,IRCOL,ir.irvalue);
	Mvprintword(LATCHROW,PORTADCOL,portaddr);
	move(LATCHROW,PORTIOCOL);
	if(busidle){
		addstr("ZZ");
	} else {
		switch(portio){
		   case PORT_RD:	addstr("@ "); break;
		   case PORT_WR:	addstr("! "); break;
		   case PORT_RDBYTE:	addstr("@C"); break;
		   case PORT_WRBYTE:	addstr("!C"); break;
		   default:		addstr("??"); break;
		}
	}
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
	move(STCACHEROW,PCACHEENCOL);
		printw(ssw.fields.pcache ? "enabled " : "disabled");
	move(STCACHEROW,RCACHEENCOL);
		printw(ssw.fields.rcache ? "enabled " : "disabled");
	Mvprintword(YROW,YCOL,y);
	move(FLAGROW,FLAGCOL); printw("%1d",flag);
	mvaddstr(STALLROW,STALLCOL,(biustall||cachestall)?"STALL":"     "); 
	move(IEROW,IECOL); printw("%1d",psw.fields.ie);
	move(CACHEROW,CACHECOL); printw("%1d",psw.fields.cacheenabled);
	Mvprintword(PSPROW,PSPCOL,externalpsp);
	Mvprintword(PSPROW,RSPCOL,externalrsp);
	Mvprintword(ABUSROW,ABUSCOL,abus);
	Mvprintword(DBUSROW,DBUSCOL,dbus);
	Mvprintword(BBUSROW,BBUSCOL,bbus);
	Mvprintword(TBUSROW,TBUSCOL,tbus);
	Mvprintword(WBUSROW,WBUSCOL,wbus);
	Mvprintword(UDRROW,UDR0COL,udr0);
	Mvprintword(UDRROW,UDR1COL,udr1);
	Mvprintword(UDRROW,UDR2COL,udr2);
	Mvprintword(UDRROW,UDR3COL,udr3);
	move(INTROW,MASKCOL); printw("%02x",intmask);
	move(INTROW,MODECOL); printw("%02x",intmode);
	move(INTROW,EDGECOL); printw("%02x",intedgedetected);
	move(INTROW,LEVLCOL); printw("%02x",intlevel);
	move(CYCLEROW,CYCLECOL);
	printw("%ld",cycle);
	move(CYCLEROW,STALLCYCCOL);
	printw("%ld",stallcycle);
	refresh();
}

static void
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

static void
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

static void
Mvprintword(y,x,value)
int y, x;
int32 value;
{
	move(y,x);
	printw(FWFORMAT,value);
}

#ifdef PCCURSES
int
printw(va_alist)
va_dcl
{
	va_list args;
	char *fmt;
	char buf[100];

	va_start(args);
	fmt = va_arg(args, char *);
	vsprintf(buf, fmt, args);
	va_end(args);
	return(addstr(buf));
}

int
wprintw(va_alist)
va_dcl
{
	va_list args;
	WINDOW *win;
	char *fmt;
	char buf[100];

	va_start(args);
	win = va_arg(args, WINDOW *);
	fmt = va_arg(args, char *);
	vsprintf(buf, fmt, args);
	va_end(args);
	return(waddstr(win,buf));
}
#endif

