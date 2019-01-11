
# line 2 "parser.y"

#include "globals.h"

#define yyerror(s) Cmdmsg(s)

typedef union {
	long num;
	int mode;
} YYSTYPE;

# define RESET 257
# define DUMP 258
# define SAVE 259
# define STARTAT 260
# define WAIT 261
# define INTON 262
# define INTOFF 263
# define DMASTART 264
# define DMAEND 265
# define PHASE 266
# define CYCLE 267
# define SIM 268
# define CYCLES 269
# define TIMES 270
# define CLEAR 271
# define TRACE 272
# define VECTORS 273
# define VISUAL 274
# define MODE 275
# define NUM 276
# define TERM 277

#include <malloc.h>
#include <memory.h>
#include <values.h>

#ifdef __cplusplus

#ifndef yyerror
	void yyerror(const char *);
#endif

#ifndef yylex
#ifdef __EXTERN_C__
	extern "C" { int yylex(void); }
#else
	int yylex(void);
#endif
#endif
	int yyparse(void);

#endif
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
YYSTYPE yylval;
YYSTYPE yyval;
typedef int yytabelem;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#if YYMAXDEPTH > 0
int yy_yys[YYMAXDEPTH], *yys = yy_yys;
YYSTYPE yy_yyv[YYMAXDEPTH], *yyv = yy_yyv;
#else	/* user does initial allocation */
int *yys;
YYSTYPE *yyv;
#endif
static int yymaxdepth = YYMAXDEPTH;
# define YYERRCODE 256

# line 140 "parser.y"


#include "scan.c"
yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 26
# define YYLAST 55
yytabelem yyact[]={

    21,     3,     4,     5,     6,     7,     8,     9,    10,    11,
    12,    13,    14,    30,    29,    15,    16,    17,    18,    48,
    19,    20,    28,    27,    55,    54,    47,    46,    52,    51,
    50,    49,    45,    44,    43,    41,    40,    36,    34,    33,
    32,    31,    26,    24,    22,    42,    35,    25,    23,    39,
    38,    37,    53,     2,     1 };
yytabelem yypact[]={

-10000000,  -256,-10000000,  -233,  -228,  -234,  -229,  -235,  -254,  -263,
  -236,  -237,  -238,  -239,  -230,  -240,  -224,  -225,  -226,  -241,
-10000000,  -242,-10000000,  -231,-10000000,  -243,-10000000,-10000000,  -244,-10000000,
  -245,-10000000,-10000000,-10000000,-10000000,  -250,-10000000,  -246,  -247,  -248,
-10000000,-10000000,  -249,-10000000,-10000000,-10000000,-10000000,  -218,  -252,-10000000,
-10000000,-10000000,-10000000,  -253,-10000000,-10000000 };
yytabelem yypgo[]={

     0,    54,    53 };
yytabelem yyr1[]={

     0,     1,     1,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2 };
yytabelem yyr2[]={

     0,     0,     4,     5,     9,     5,     7,     5,     5,     7,
     5,     7,     5,     5,     5,     5,     7,    11,     9,     5,
     7,     7,     7,     5,     3,     5 };
yytabelem yychk[]={

-10000000,    -1,    -2,   257,   258,   259,   260,   261,   262,   263,
   264,   265,   266,   267,   268,   271,   272,   273,   274,   276,
   277,   256,   277,   276,   277,   276,   277,   277,   276,   277,
   276,   277,   277,   277,   277,   276,   277,   275,   275,   275,
   277,   277,   276,   277,   277,   277,   277,   276,   269,   277,
   277,   277,   277,   270,   277,   277 };
yytabelem yydef[]={

     1,    -2,     2,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    24,     0,     3,     0,     5,     0,     7,     8,     0,    10,
     0,    12,    13,    14,    15,     0,    19,     0,     0,     0,
    23,    25,     0,     6,     9,    11,    16,     0,     0,    20,
    21,    22,     4,     0,    18,    17 };
typedef struct
#ifdef __cplusplus
	yytoktype
#endif
{ char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"RESET",	257,
	"DUMP",	258,
	"SAVE",	259,
	"STARTAT",	260,
	"WAIT",	261,
	"INTON",	262,
	"INTOFF",	263,
	"DMASTART",	264,
	"DMAEND",	265,
	"PHASE",	266,
	"CYCLE",	267,
	"SIM",	268,
	"CYCLES",	269,
	"TIMES",	270,
	"CLEAR",	271,
	"TRACE",	272,
	"VECTORS",	273,
	"VISUAL",	274,
	"MODE",	275,
	"NUM",	276,
	"TERM",	277,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"program : /* empty */",
	"program : program statement",
	"statement : RESET TERM",
	"statement : DUMP NUM NUM TERM",
	"statement : SAVE TERM",
	"statement : STARTAT NUM TERM",
	"statement : WAIT TERM",
	"statement : INTON TERM",
	"statement : INTON NUM TERM",
	"statement : INTOFF TERM",
	"statement : INTOFF NUM TERM",
	"statement : DMASTART TERM",
	"statement : DMAEND TERM",
	"statement : PHASE TERM",
	"statement : CYCLE TERM",
	"statement : SIM NUM TERM",
	"statement : SIM NUM NUM TIMES TERM",
	"statement : SIM NUM CYCLES TERM",
	"statement : CLEAR TERM",
	"statement : TRACE MODE TERM",
	"statement : VECTORS MODE TERM",
	"statement : VISUAL MODE TERM",
	"statement : NUM TERM",
	"statement : TERM",
	"statement : error TERM",
};
#endif /* YYDEBUG */
/*
 * Copyright (c) 1993 by Sun Microsystems, Inc.
 */

#pragma ident	"@(#)yaccpar	6.12	93/06/07 SMI"

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#define YYNEW(type)	malloc(sizeof(type) * yynewmax)
#define YYCOPY(to, from, type) \
	(type *) memcpy(to, (char *) from, yynewmax * sizeof(type))
#define YYENLARGE( from, type) \
	(type *) realloc((char *) from, yynewmax * sizeof(type))
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-10000000)

/*
** global variables used by the parser
*/
YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



#ifdef YYNMBCHARS
#define YYLEX()		yycvtok(yylex())
/*
** yycvtok - return a token if i is a wchar_t value that exceeds 255.
**	If i<255, i itself is the token.  If i>255 but the neither 
**	of the 30th or 31st bit is on, i is already a token.
*/
#if defined(__STDC__) || defined(__cplusplus)
int yycvtok(int i)
#else
int yycvtok(i) int i;
#endif
{
	int first = 0;
	int last = YYNMBCHARS - 1;
	int mid;
	wchar_t j;

	if(i&0x60000000){/*Must convert to a token. */
		if( yymbchars[last].character < i ){
			return i;/*Giving up*/
		}
		while ((last>=first)&&(first>=0)) {/*Binary search loop*/
			mid = (first+last)/2;
			j = yymbchars[mid].character;
			if( j==i ){/*Found*/ 
				return yymbchars[mid].tvalue;
			}else if( j<i ){
				first = mid + 1;
			}else{
				last = mid -1;
			}
		}
		/*No entry in the table.*/
		return i;/* Giving up.*/
	}else{/* i is already a token. */
		return i;
	}
}
#else/*!YYNMBCHARS*/
#define YYLEX()		yylex()
#endif/*!YYNMBCHARS*/

/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
#if defined(__STDC__) || defined(__cplusplus)
int yyparse(void)
#else
int yyparse()
#endif
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

#if defined(__cplusplus) || defined(lint)
/*
	hacks to please C++ and lint - goto's inside switch should never be
	executed; yypvt is set to 0 to avoid "used before set" warning.
*/
	static int __yaccpar_lint_hack__ = 0;
	switch (__yaccpar_lint_hack__)
	{
		case 1: goto yyerrlab;
		case 2: goto yynewstate;
	}
	yypvt = 0;
#endif

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

#if YYMAXDEPTH <= 0
	if (yymaxdepth <= 0)
	{
		if ((yymaxdepth = YYEXPAND(0)) <= 0)
		{
			yyerror("yacc initialization error");
			YYABORT;
		}
	}
#endif

	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */
	goto yystack;	/* moved from 6 lines above to here to please C++ */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			int yyps_index = (yy_ps - yys);
			int yypv_index = (yy_pv - yyv);
			int yypvt_index = (yypvt - yyv);
			int yynewmax;
#ifdef YYEXPAND
			yynewmax = YYEXPAND(yymaxdepth);
#else
			yynewmax = 2 * yymaxdepth;	/* double table size */
			if (yymaxdepth == YYMAXDEPTH)	/* first time growth */
			{
				char *newyys = (char *)YYNEW(int);
				char *newyyv = (char *)YYNEW(YYSTYPE);
				if (newyys != 0 && newyyv != 0)
				{
					yys = YYCOPY(newyys, yys, int);
					yyv = YYCOPY(newyyv, yyv, YYSTYPE);
				}
				else
					yynewmax = 0;	/* failed */
			}
			else				/* not first time */
			{
				yys = YYENLARGE(yys, int);
				yyv = YYENLARGE(yyv, YYSTYPE);
				if (yys == 0 || yyv == 0)
					yynewmax = 0;	/* failed */
			}
#endif
			if (yynewmax <= yymaxdepth)	/* tables not expanded */
			{
				yyerror( "yacc stack overflow" );
				YYABORT;
			}
			yymaxdepth = yynewmax;

			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
			skip_init:
				yynerrs++;
				/* FALLTHRU */
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 3:
# line 44 "parser.y"
{
			Resetregs();		/* reset machine state */
		} break;
case 4:
# line 48 "parser.y"
{		/* dump memory */
			Dump((uint32)yypvt[-2].num,(int)yypvt[-1].num);
		} break;
case 5:
# line 52 "parser.y"
{			/* save core image */
			Resetobj();		/* rewind core file */
			Saveobj();
		} break;
case 6:
# line 57 "parser.y"
{		/* start simulation at X */
			Startat((uint32)yypvt[-1].num);
		} break;
case 7:
# line 61 "parser.y"
{
			Waitcycle();		/* wait cycle */
		} break;
case 8:
# line 65 "parser.y"
{			/* activate interrupt */
			Interrupton(0);		/* default to int 0 */
		} break;
case 9:
# line 69 "parser.y"
{		/* activate interrupt */
			Interrupton((int)yypvt[-1].num);
		} break;
case 10:
# line 73 "parser.y"
{			/* deactivate interrupt */
			Interruptoff(0);	/* default to int 0 */
		} break;
case 11:
# line 77 "parser.y"
{		/* deactivate interrupt */
			Interruptoff((int)yypvt[-1].num);
		} break;
case 12:
# line 81 "parser.y"
{			/* start DMA */
			Dmastart();
		} break;
case 13:
# line 85 "parser.y"
{			/* end DMA */
			Dmaend();
		} break;
case 14:
# line 89 "parser.y"
{			/* single phase mode */
			Phase();
		} break;
case 15:
# line 93 "parser.y"
{			/* single cycle mode */
			Cycle();
		} break;
case 16:
# line 97 "parser.y"
{			/* non-interactive sim. */
			Sim((uint32)yypvt[-1].num,(int)1);
		} break;
case 17:
# line 101 "parser.y"
{	/* non-interactive sim. */
			Sim((uint32)yypvt[-3].num,(int)yypvt[-2].num);
		} break;
case 18:
# line 105 "parser.y"
{		/* non-interactive sim. */
			Simcycles((int)yypvt[-2].num);
		} break;
case 19:
# line 109 "parser.y"
{			/* clear statistics */
			Clearstats();
		} break;
case 20:
# line 113 "parser.y"
{		/* control address tracing */
			Tracemode(yypvt[-1].mode);
		} break;
case 21:
# line 117 "parser.y"
{		/* control vector generation */
			Vectormode(yypvt[-1].mode);
		} break;
case 22:
# line 121 "parser.y"
{		/* control screen update */
			Visualmode(yypvt[-1].mode);
		} break;
case 23:
# line 125 "parser.y"
{			/* stack a number */
			Stack((int32)yypvt[-1].num);
		} break;
case 24:
# line 129 "parser.y"
{				/* single step */
			Singlestep();
		} break;
case 25:
# line 133 "parser.y"
{
			yyerrok;
			yyclearin;
		} break;
	}
	goto yystack;		/* reset registers in driver code */
}

