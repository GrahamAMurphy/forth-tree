#include <stdio.h>
#include <curses.h>
#include "globals.h"
#include "external.h"
#include "intrnl3.h"
#include "frmts3.h"

#define STBASEROW	18		/* stack base */
#define PSTACKCOL	31		/* parameter stack */
#define RSTACKCOL	63		/* return stack */
#define LATCHROW	1		/* latches */
#define ALUINCOL	0		/* ALU input registers */
#define ALUOUTCOL	16		/* ALU output latch */
#define PCCOL		31		/* Program Counter phase A latch */
#define IRCOL		47		/* IR */
#define PORTADCOL	59		/* port address */
#define PORTIOCOL	67		/* port I/O operation */
#define PORTVALCOL	68		/* port value */
#define FLAGROW		4		/* flag */
#define FLAGCOL		5
#define IEROW		7		/* interrupt enable flag */
#define IECOL		1
#define CACHEENCOL	12		/* cache enable bit */
#define TBUSROW		10		/* T bus */
#define TBUSCOL		0
#define ABUSROW		10		/* A bus */
#define ABUSCOL		13
#define BBUSROW		13		/* B bus */
#define BBUSCOL		13
#define UDR0ROW		16		/* UDR0-1 */
#define UDR0COL		0
#define UDR1COL		13
#define UDR2ROW		18		/* UDR2-3 */
#define CYCLEROW	19		/* cycle and phase */
#define CYCLECOL	29
#define PHASECOL	48

#define NCMDLINES (LINES-CYCLEROW-1)	/* number of lines in command window */

#define C_ERASE	'\010'			/* erase character */
#define C_KILL  '\025'			/* kill character */ 
#define C_EOF	'\004'			/* end of file marker */

#define FWFORMAT "%08lx"		/* display format */

static WINDOW *cmdwin;			/* command window */

int Wfieldyx();
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

#ifdef FORTHPARSE
/*	Forth Interpreter I/O routines */
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
		if(c==C_ERASE){
			if(i>0){
				getyx(win,row,col);
				wmove(win,row,col-1);
				waddch(win,' ');
				wmove(win,row,col-1);
				ptr--; i--;
			}
		} else if(c==C_KILL){
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

	for(;len>0;len -= 4){
		wprintw(cmdwin,FWFORMAT,addr);
		wprintw(cmdwin,":  ");
		for(i=0;i<4;i++){
			wprintw(cmdwin,"%08lx ",Fetchexternal(addr++));
		}
		waddch(cmdwin,'\n');
		wrefresh(cmdwin);
	}
}

void
Iowrite(addr,value)
uint32 addr;
int32 value;
{
	wprintw(cmdwin,"I/O %08lx ! %08lx\n",addr,value);
	wrefresh(cmdwin);
}

void
Ioread(addr,value)
uint32 addr;
int32 value;
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
	Initcmdwin();			/* initialize command window */
	mvaddstr(0,0,
" alu in         aluresult         pc             ir         port"
);
	mvaddstr(1,0,
"xxxxxxxx        xxxxxxxx       xxxxxxxx        xxxxxxxx    xxxxxxxx@xxxxxxxx");
	mvaddstr(2,0,
"xxxxxxxx");
	mvaddstr(3,0,
"                parameter      xxxxxxxx         return         xxxxxxxx");
	mvaddstr(4,0,
"flag x             stack:      xxxxxxxx         stack:         xxxxxxxx");
	mvaddstr(5,0,
"                               xxxxxxxx                        xxxxxxxx");
	mvaddstr(6,0,
"ie         cache               xxxxxxxx                        xxxxxxxx");
	mvaddstr(7,0,
" x          x                  xxxxxxxx                        xxxxxxxx");
	mvaddstr(8,0,
"                               xxxxxxxx                        xxxxxxxx");
	mvaddstr(9,0,
"  tbus         abus            xxxxxxxx                        xxxxxxxx");
	mvaddstr(10,0,
"xxxxxxxx     xxxxxxxx          xxxxxxxx                        xxxxxxxx");
	mvaddstr(11,0,
"                               xxxxxxxx                        xxxxxxxx");
	mvaddstr(12,0,
"               bbus            xxxxxxxx                        xxxxxxxx");
	mvaddstr(13,0,
"             xxxxxxxx          xxxxxxxx                        xxxxxxxx");
	mvaddstr(14,0,
"                               xxxxxxxx                        xxxxxxxx");
	mvaddstr(15,0,
"  udr0         udr1            xxxxxxxx                        xxxxxxxx");
	mvaddstr(16,0,
"xxxxxxxx     xxxxxxxx          xxxxxxxx                        xxxxxxxx");
	mvaddstr(17,0,
"  udr2         udr3            xxxxxxxx                        xxxxxxxx");
	mvaddstr(18,0,
"xxxxxxxx     xxxxxxxx          xxxxxxxx                        xxxxxxxx");
	mvaddstr(19,0,
"-----------------------cycle -------------phase -------------------------");
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
	Mvprintword(LATCHROW,PCCOL,pca);
	Mvprintword(LATCHROW,ALUOUTCOL,aluout);
	Mvprintword(LATCHROW,ALUINCOL,alua);
	Mvprintword(LATCHROW+1,ALUINCOL,alub);
	Mvprintword(LATCHROW,IRCOL,ir.irvalue);
	Mvprintword(LATCHROW,PORTADCOL,portaddr);
	if(portio == PORT_RD)
		mvaddch(LATCHROW,PORTIOCOL,'@');
	else
		mvaddch(LATCHROW,PORTIOCOL,'!');
	Mvprintword(LATCHROW,PORTVALCOL,portvalue);
	move(IEROW,IECOL); printw("%1d",psw.fields.ie);
	move(IEROW,CACHEENCOL); printw("%1d",psw.fields.cache);
	move(FLAGROW,FLAGCOL); printw("%1d",flag);
	Mvprintword(TBUSROW,TBUSCOL,tbus);
	Mvprintword(ABUSROW,ABUSCOL,abus);
	Mvprintword(BBUSROW,BBUSCOL,bbus);
	Mvprintword(UDR0ROW,UDR0COL,udr0);
	Mvprintword(UDR0ROW,UDR1COL,udr1);
	Mvprintword(UDR2ROW,UDR0COL,udr2);
	Mvprintword(UDR2ROW,UDR1COL,udr3);
	move(CYCLEROW,CYCLECOL);
	printw("%ld",cycle);
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
int32 value;
{
	move(y,x);
	printw(FWFORMAT,value);
}
