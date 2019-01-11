#include <stdio.h>
#include "masks4c.h"

#define SYMBOLIC 1			/* use finesse symbolic syntax */
#define BINARY 0			/* use #define syntax */

#define BUFFERSIZE 16384

void Start();
void Assign();
void Sassign();
void End();
char *Binary();

main()
{
	Start("STATECODE", 4, SYMBOLIC);
	Assign("int", 0);
	Assign("exec", 1);
	Assign("load", 2);
	Assign("store", 3);
	Assign("squash",4);
	Assign("rover1", 8);
	Assign("rover2", 9);
	Assign("runder1", 10);
	Assign("runder2", 11);
	Assign("pover1", 12);
	Assign("pover2", 13);
	Assign("punder1", 14);
	Assign("punder2", 15);
	End();

	Start("IOCODE", 4, BINARY);
	Assign("rombase", ROMPAGE);
	Assign("iobase", IOPAGE);
	End();

	Start("REGCODE", 5, BINARY);
	Assign("rsp", R_RSP);
	Assign("psp", R_PSP);
	Assign("scc", R_SCC);
	Assign("mask", R_MASK);
	Assign("mode", R_MODE);
	Assign("edge", R_EDGE);
	Assign("levl", R_LEVL);
	Assign("attrfirst", R_ATTR);
	Sassign("attrrange", R_ATTRRANGE);
	End();

	Start("TYPECODE", 3, SYMBOLIC);
	Assign("flow", B_FLOW);
	Assign("micro", B_MICRO);
	Assign("lal", B_LAL);
	Assign("lah", B_LAH);
	Assign("loadcell", B_LOAD);
	Assign("loadbyte", B_LOADBYTE);
	Assign("storecell", B_STORE);
	Assign("storebyte", B_STOREBYTE);
	End();

	Start("FLOWCODE", 2, SYMBOLIC);
	Assign("call", B_COLON);
	Assign("does", B_DOES);
	Assign("branch", B_BRANCH);
	Assign("qbranch", B_QBRANCH);
	End();

	Start("RETCODE", 1, SYMBOLIC);
	Assign("noreturn", B_NORETURN);
	Assign("return", B_RETURN);
	End();

	Start("RCODE", 4, SYMBOLIC);
	Assign("s0", B_S0);
	Assign("s1", B_S1);
	Assign("s2", B_S2);
	Assign("s3", B_S3);
	Assign("r0", B_R0);
	Assign("r1", B_R1);
	Assign("r2", B_R2);
	Assign("r3", B_R3);
	Assign("u0", B_U0);
	Assign("u1", B_U1);
	Assign("u2", B_U2);
	Assign("u3", B_U3);
	Assign("pc", B_IAR);
	Assign("psw", B_PSW);
	Assign("zero", B_ZERO);
	Assign("y", B_Y);
	End();

	Start("PSCODE", 2, SYMBOLIC);
	Assign("nopps", B_NOPPS);
	Assign("popps", B_POPPS);
	Assign("pushps", B_PUSHPS);
	End();

	Start("RSCODE", 2, SYMBOLIC);
	Assign("noprs", B_NOPRS);
	Assign("poprs", B_POPRS);
	Assign("pushrs", B_PUSHRS);
	End();

	Start("CLASSCODE", 2, SYMBOLIC);
	Assign("alu", B_ALU);
	Assign("test", B_TEST);
	Assign("step", B_STEP);
	Assign("shift", B_SHIFT);
	End();

	Start("ALUCONDCODE", 4, SYMBOLIC);
	Assign("clear", B_CLEAR);
	Assign("set", B_SET);
	Assign("ovflow", B_OVFLOW);
	Assign("vbar", B_VBAR);
	Assign("greater", B_GREATER);
	Assign("lesseq", B_LESSEQ);
	Assign("neg", B_NEG);
	Assign("nbar", B_NBAR);
	Assign("equal", B_EQUAL);
	Assign("nequal", B_NEQUAL);
	Assign("ugreater", B_UGREATER);
	Assign("ulesseq", B_ULESSEQ);
	Assign("less", B_LESS);
	Assign("greatereq", B_GREATEREQ);
	Assign("cout", B_COUT);
	Assign("cbar", B_CBAR);
	End();

	Start("FLAGCODE", 1, SYMBOLIC);
	Assign("noflag", B_NOFLAG);
	Assign("flag", B_FLAG);
	End();

	Start("CINCODE", 2, SYMBOLIC);
	Assign("czero", B_CZERO);
	Assign("cone", B_CONE);
	Assign("cflag", B_CFLAG);
	Assign("cflagbar", B_CFLAGBAR);
	End();

	Start("ALUOPCODE", 6, BINARY);
	Assign("ones", B_ONES);
	Assign("zeros", B_ZEROS);
	Assign("nopa", B_NOPA);
	Assign("nopb", B_NOPB);
	Assign("nota", B_NOTA);
	Assign("notb", B_NOTB);
	Assign("aandb", B_AANDB);
	Assign("abarandb", B_ABARANDB);
	Assign("aandbbar", B_AANDBBAR);
	Assign("anandb", B_ANANDB);
	Assign("aorb", B_AORB);
	Assign("abarorb", B_ABARORB);
	Assign("aorbbar", B_AORBBAR);
	Assign("anorb", B_ANORB);
	Assign("axorb", B_AXORB);
	Assign("anxorb", B_ANXORB);
	Assign("aplusb", B_APLUSB);
	Assign("aminusb", B_AMINUSB);
	Assign("bminusa", B_BMINUSA);
	Assign("negb", B_NEGB);
	Assign("incb", B_INCB);
	Assign("decb", B_DECB);
	Assign("bplusb", B_BPLUSB);
	End();

	Start("FSRCCODE", 2, BINARY);
	Assign("flalucond", B_FLALUCOND);
	Assign("fly1", B_FLY1);
	Assign("fldivhelp", B_FLDIVHELP);
	End();

	Start("LEFTCODE", 1, SYMBOLIC);
	Assign("noleftshift", B_NOLEFTSHIFT);
	Assign("left", B_LEFT);
	End();

	Start("YCODE", 2, BINARY);
	Assign("ynop", B_YNOP);
	Assign("yleft", B_YLEFT);
	Assign("yright", B_YRIGHT);
	End();

	Start("STEPCODE", 2, SYMBOLIC);
	Assign("stepb", B_STEPB);
	Assign("cadd", B_CADD);
	Assign("div", B_DIV);
	Assign("mul", B_MUL);
	End();

	Start("SHIFTMODECODE", 3, SYMBOLIC);
	Assign("lsr", B_LSR);
	Assign("asr", B_ASR);
	Assign("lsl", B_LSL);
	Assign("rol", B_ROL);
	Assign("srflag", B_SRFLAG);
	Assign("srflagbar", B_SRFLAGBAR);
	Assign("pri", B_PRI);
	End();

	Start("IMMCODE", 1, SYMBOLIC);
	Assign("noimm", B_NOIMM);
	Assign("imm", B_IMM);
	End();
}
/* ------------------------------------------------------------------------ */
int fieldwidth;
int defmode = SYMBOLIC;
char symbolbuf[BUFFERSIZE];
char *symbol;
char codebuf[BUFFERSIZE];
char *code;

void
Start(macroname, width, mode)
char *macroname;
int width;
int mode;
{
	fieldwidth = width;
	defmode = mode;
	if(defmode==SYMBOLIC){
		printf("#define %s \\\n", macroname);
		strcpy(symbolbuf, "{");	symbol = &symbolbuf[1];
		strcpy(codebuf, "{");	code = &codebuf[1];
	}
}

void
Assign(newsymbol, newcode)
char *newsymbol;
unsigned int newcode;
{
	if(defmode==SYMBOLIC){
		sprintf(symbol, "%s, \\\n", newsymbol);
		symbol += strlen(symbol);
		sprintf(code, "%s, \\\n", Binary(newsymbol, newcode));
		code += strlen(code);
	} else {
		printf("#define %s ^b%s\n", newsymbol,
				Binary(newsymbol, newcode));
	}
}

void
Sassign(newsymbol, newcode)
char *newsymbol;
char *newcode;
{
	if(defmode==SYMBOLIC){
		sprintf(symbol, "%s, \\\n", newsymbol);
		symbol += strlen(symbol);
		sprintf(code, "%s, \\\n", newcode);
		code += strlen(code);
	} else {
		printf("#define %s ^b%s\n", newsymbol, newcode);
	}
}

void
End()
{
	if(defmode==SYMBOLIC){
		strcpy(symbol-4, "} \\\n");
		fputs(symbolbuf, stdout);
		strcpy(code-4, "}\n");
		fputs(codebuf, stdout);
	}
}

char *
Binary(symbol, code)
char *symbol;
unsigned int code;
{
	static char binary[33];
	char *t=&binary[32];
	int i;

	for(*t='\0', i=0; i<fieldwidth; i++){
		*--t = code%2 ? '1' : '0';
		code /= 2;
	}
	if(code != 0){
		fprintf(stderr,"code for %s too large!\n", symbol);
		exit(1);
	}
	return(t);
}

