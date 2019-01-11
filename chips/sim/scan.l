%{

#undef getc
#define getc(x)	Getcmdc(x)		/* use getc through curses */

%}

%%

reset			{ return(RESET); }

dump			{ return(DUMP); }

save			{ return(SAVE); }

startat			{ return(STARTAT); }

wait			{ return(WAIT); }

inton			{ return(INTON); }

intoff			{ return(INTOFF); }

dmastart		{ return(DMASTART); }
dmaend			{ return(DMAEND); }

phase			{ return(PHASE); }

cycle			{ return(CYCLE); }

sim			{ return(SIM); }

cycles			{ return(CYCLES); }

times			{ return(TIMES); }

clear			{ return(CLEAR); }

trace			{ return(TRACE); }

vectors			{ return(VECTORS); }

visual			{ return(VISUAL); }

bye			{ return(EOF); }

off			{ yylval.mode = 0; return(MODE); }

on			{ yylval.mode = 1; return(MODE); }

[0-9]+			{ yylval.num = atoi(yytext);
			  return(NUM); }

0[0-7]+			{ int value = 0;
			  char *cptr = yytext+1;

			  while(*cptr){
				value *= 8;
				value += *cptr++ - '0';
			  }
			  yylval.num = value;
			  return(NUM); }

0[xX][0-9A-Fa-f]+	{ int value = 0;
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

[\t ]+			;

\n			{ return(TERM); }

.			;		/* ignore garbage */

%%
yywrap()
{
	return(1);
}

