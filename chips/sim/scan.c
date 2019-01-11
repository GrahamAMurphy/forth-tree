#include <stdio.h>
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
#ifndef __cplusplus
# define output(c) (void)putc(c,yyout)
#else
# define lex_output(c) (void)putc(c,yyout)
#endif

#if defined(__cplusplus) || defined(__STDC__)

#if defined(__cplusplus) && defined(__EXTERN_C__)
extern "C" {
#endif
	int yyback(int *, int);
	int yyinput(void);
	int yylook(void);
	void yyoutput(int);
	int yyracc(int);
	int yyreject(void);
	void yyunput(int);
	int yylex(void);
#ifdef YYLEX_E
	void yywoutput(wchar_t);
	wchar_t yywinput(void);
#endif
#ifndef yyless
	void yyless(int);
#endif
#ifndef yywrap
	int yywrap(void);
#endif
#ifdef LEXDEBUG
	void allprint(char);
	void sprint(char *);
#endif
#if defined(__cplusplus) && defined(__EXTERN_C__)
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
	void exit(int);
#ifdef __cplusplus
}
#endif

#endif
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
#ifndef __cplusplus
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#else
# define lex_input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#endif
#define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;

#undef getc
#define getc(x)	Getcmdc(x)		/* use getc through curses */

# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
#ifdef __cplusplus
/* to avoid CC and lint complaining yyfussy not being used ...*/
static int __lex_hack = 0;
if (__lex_hack) goto yyfussy;
#endif
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:

# line 10 "scan.l"
		{ return(RESET); }
break;
case 2:

# line 12 "scan.l"
		{ return(DUMP); }
break;
case 3:

# line 14 "scan.l"
		{ return(SAVE); }
break;
case 4:

# line 16 "scan.l"
		{ return(STARTAT); }
break;
case 5:

# line 18 "scan.l"
		{ return(WAIT); }
break;
case 6:

# line 20 "scan.l"
		{ return(INTON); }
break;
case 7:

# line 22 "scan.l"
		{ return(INTOFF); }
break;
case 8:

# line 24 "scan.l"
	{ return(DMASTART); }
break;
case 9:

# line 25 "scan.l"
		{ return(DMAEND); }
break;
case 10:

# line 27 "scan.l"
		{ return(PHASE); }
break;
case 11:

# line 29 "scan.l"
		{ return(CYCLE); }
break;
case 12:

# line 31 "scan.l"
		{ return(SIM); }
break;
case 13:

# line 33 "scan.l"
		{ return(CYCLES); }
break;
case 14:

# line 35 "scan.l"
		{ return(TIMES); }
break;
case 15:

# line 37 "scan.l"
		{ return(CLEAR); }
break;
case 16:

# line 39 "scan.l"
		{ return(TRACE); }
break;
case 17:

# line 41 "scan.l"
		{ return(VECTORS); }
break;
case 18:

# line 43 "scan.l"
		{ return(VISUAL); }
break;
case 19:

# line 45 "scan.l"
		{ return(EOF); }
break;
case 20:

# line 47 "scan.l"
		{ yylval.mode = 0; return(MODE); }
break;
case 21:

# line 49 "scan.l"
		{ yylval.mode = 1; return(MODE); }
break;
case 22:

# line 51 "scan.l"
		{ yylval.num = atoi(yytext);
			  return(NUM); }
break;
case 23:

# line 54 "scan.l"
		{ int value = 0;
			  char *cptr = yytext+1;

			  while(*cptr){
				value *= 8;
				value += *cptr++ - '0';
			  }
			  yylval.num = value;
			  return(NUM); }
break;
case 24:

# line 64 "scan.l"
{ int value = 0;
			  char *cptr = yytext+2;

			  while(*cptr){
				value *= 16;
				if(*cptr <= '9')
					value += *cptr++ - '0';
				else if(*cptr <= 'F')
					value += *cptr++ - 'A' + 10;
				else
					value += *cptr++ - 'a' + 10;
			  }
			  yylval.num = value;
			  return(NUM); }
break;
case 25:

# line 79 "scan.l"
		;
break;
case 26:

# line 81 "scan.l"
		{ return(TERM); }
break;
case 27:

# line 83 "scan.l"
		;
break;
case -1:
break;
default:
(void)fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
yywrap()
{
	return(1);
}

int yyvstop[] = {
0,

27,
0,

25,
27,
0,

26,
0,

22,
27,
0,

22,
27,
0,

27,
0,

27,
0,

27,
0,

27,
0,

27,
0,

27,
0,

27,
0,

27,
0,

27,
0,

27,
0,

27,
0,

25,
0,

22,
23,
0,

22,
0,

21,
0,

24,
0,

19,
0,

20,
0,

12,
0,

2,
0,

3,
0,

5,
0,

15,
0,

11,
0,

6,
0,

10,
0,

1,
0,

14,
0,

16,
0,

13,
0,

9,
0,

7,
0,

18,
0,

4,
0,

17,
0,

8,
0,
0};
# define YYTYPE unsigned char
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,3,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,4,	1,5,	
4,19,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	4,19,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	1,6,	1,7,	1,7,	
1,7,	1,7,	1,7,	1,7,	
1,7,	1,7,	20,20,	20,20,	
20,20,	20,20,	20,20,	20,20,	
20,20,	20,20,	1,3,	0,0,	
0,0,	0,0,	0,0,	0,0,	
2,7,	2,7,	2,7,	2,7,	
2,7,	2,7,	2,7,	7,21,	
7,21,	7,21,	7,21,	7,21,	
7,21,	7,21,	7,21,	7,21,	
7,21,	1,3,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	18,40,	1,8,	
1,9,	1,10,	14,32,	17,38,	
23,42,	13,31,	1,11,	17,39,	
24,43,	25,44,	10,26,	11,28,	
1,12,	1,13,	9,24,	1,14,	
1,15,	1,16,	10,27,	1,17,	
1,18,	2,8,	2,9,	2,10,	
8,23,	12,29,	26,45,	9,25,	
2,11,	27,46,	28,47,	16,36,	
29,48,	12,30,	2,12,	2,13,	
31,49,	2,14,	2,15,	2,16,	
16,37,	2,17,	2,18,	6,20,	
6,20,	6,20,	6,20,	6,20,	
6,20,	6,20,	6,20,	6,21,	
6,21,	15,33,	32,50,	33,51,	
34,52,	35,53,	36,54,	37,55,	
38,56,	15,34,	39,57,	40,58,	
43,59,	44,60,	46,63,	47,64,	
49,65,	50,66,	45,61,	51,67,	
15,35,	22,41,	22,41,	22,41,	
22,41,	22,41,	22,41,	22,41,	
22,41,	22,41,	22,41,	6,22,	
45,62,	53,68,	54,69,	55,70,	
56,71,	57,72,	22,41,	22,41,	
22,41,	22,41,	22,41,	22,41,	
58,73,	59,74,	60,75,	61,76,	
62,77,	64,78,	65,80,	66,81,	
68,82,	69,83,	70,84,	71,85,	
72,86,	64,79,	75,87,	76,88,	
77,89,	78,90,	82,91,	6,22,	
85,92,	86,93,	89,94,	91,95,	
92,96,	94,97,	22,41,	22,41,	
22,41,	22,41,	22,41,	22,41,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-23,	yysvec+1,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+3,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+95,	0,		yyvstop+8,
yycrank+31,	0,		yyvstop+11,
yycrank+3,	0,		yyvstop+14,
yycrank+6,	0,		yyvstop+16,
yycrank+1,	0,		yyvstop+18,
yycrank+1,	0,		yyvstop+20,
yycrank+23,	0,		yyvstop+22,
yycrank+1,	0,		yyvstop+24,
yycrank+1,	0,		yyvstop+26,
yycrank+56,	0,		yyvstop+28,
yycrank+26,	0,		yyvstop+30,
yycrank+2,	0,		yyvstop+32,
yycrank+1,	0,		yyvstop+34,
yycrank+0,	yysvec+4,	yyvstop+36,
yycrank+10,	yysvec+7,	yyvstop+38,
yycrank+0,	yysvec+7,	yyvstop+41,
yycrank+125,	0,		0,	
yycrank+3,	0,		0,	
yycrank+7,	0,		0,	
yycrank+10,	0,		0,	
yycrank+29,	0,		0,	
yycrank+20,	0,		0,	
yycrank+14,	0,		0,	
yycrank+30,	0,		0,	
yycrank+0,	0,		yyvstop+43,
yycrank+39,	0,		0,	
yycrank+39,	0,		0,	
yycrank+37,	0,		0,	
yycrank+47,	0,		0,	
yycrank+60,	0,		0,	
yycrank+49,	0,		0,	
yycrank+62,	0,		0,	
yycrank+61,	0,		0,	
yycrank+47,	0,		0,	
yycrank+58,	0,		0,	
yycrank+0,	yysvec+22,	yyvstop+45,
yycrank+0,	0,		yyvstop+47,
yycrank+67,	0,		0,	
yycrank+57,	0,		0,	
yycrank+69,	0,		0,	
yycrank+54,	0,		0,	
yycrank+56,	0,		0,	
yycrank+0,	0,		yyvstop+49,
yycrank+53,	0,		0,	
yycrank+68,	0,		0,	
yycrank+70,	0,		0,	
yycrank+0,	0,		yyvstop+51,
yycrank+71,	0,		0,	
yycrank+85,	0,		0,	
yycrank+88,	0,		0,	
yycrank+72,	0,		0,	
yycrank+72,	0,		0,	
yycrank+80,	0,		0,	
yycrank+83,	0,		0,	
yycrank+97,	0,		0,	
yycrank+89,	0,		0,	
yycrank+84,	0,		0,	
yycrank+0,	0,		yyvstop+53,
yycrank+99,	0,		0,	
yycrank+101,	0,		0,	
yycrank+87,	0,		0,	
yycrank+0,	0,		yyvstop+55,
yycrank+88,	0,		0,	
yycrank+90,	0,		0,	
yycrank+105,	0,		0,	
yycrank+96,	0,		0,	
yycrank+111,	0,		0,	
yycrank+0,	0,		yyvstop+57,
yycrank+0,	0,		yyvstop+59,
yycrank+95,	0,		yyvstop+61,
yycrank+111,	0,		0,	
yycrank+115,	0,		0,	
yycrank+111,	0,		0,	
yycrank+0,	0,		yyvstop+63,
yycrank+0,	0,		yyvstop+65,
yycrank+0,	0,		yyvstop+67,
yycrank+117,	0,		0,	
yycrank+0,	0,		yyvstop+69,
yycrank+0,	0,		yyvstop+71,
yycrank+102,	0,		0,	
yycrank+109,	0,		0,	
yycrank+0,	0,		yyvstop+73,
yycrank+0,	0,		yyvstop+75,
yycrank+104,	0,		0,	
yycrank+0,	0,		yyvstop+77,
yycrank+103,	0,		0,	
yycrank+105,	0,		0,	
yycrank+0,	0,		yyvstop+79,
yycrank+105,	0,		0,	
yycrank+0,	0,		yyvstop+81,
yycrank+0,	0,		yyvstop+83,
yycrank+0,	0,		yyvstop+85,
0,	0,	0};
struct yywork *yytop = yycrank+227;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
  0,   1,   1,   1,   1,   1,   1,   1, 
  1,   9,  10,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  9,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
 48,  48,  48,  48,  48,  48,  48,  48, 
 56,  56,   1,   1,   1,   1,   1,   1, 
  1,  65,  65,  65,  65,  65,  65,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
 88,   1,   1,   1,   1,   1,   1,   1, 
  1,  65,  65,  65,  65,  65,  65,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
 88,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	Copyright (c) 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#pragma ident	"@(#)ncform	6.7	93/06/07 SMI"

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
#if defined(__cplusplus) || defined(__STDC__)
int yylook(void)
#else
yylook()
#endif
{
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
#ifndef __cplusplus
			*yylastch++ = yych = input();
#else
			*yylastch++ = yych = lex_input();
#endif
			if(yylastch > &yytext[YYLMAX]) {
				fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
				exit(1);
			}
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
#ifndef __cplusplus
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
#else
		yyprevious = yytext[0] = lex_input();
		if (yyprevious>0)
			lex_output(yyprevious);
#endif
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
#if defined(__cplusplus) || defined(__STDC__)
int yyback(int *p, int m)
#else
yyback(p, m)
	int *p;
#endif
{
	if (p==0) return(0);
	while (*p) {
		if (*p++ == m)
			return(1);
	}
	return(0);
}
	/* the following are only used in the lex library */
#if defined(__cplusplus) || defined(__STDC__)
int yyinput(void)
#else
yyinput()
#endif
{
#ifndef __cplusplus
	return(input());
#else
	return(lex_input());
#endif
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyoutput(int c)
#else
yyoutput(c)
  int c; 
#endif
{
#ifndef __cplusplus
	output(c);
#else
	lex_output(c);
#endif
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyunput(int c)
#else
yyunput(c)
   int c; 
#endif
{
	unput(c);
	}
