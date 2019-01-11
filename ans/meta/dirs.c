/* (c) 1992 Johns Hopkins University / Applied Physics Laboratory */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fc.h"

/****************************************************************************
 *
 *			global variables
 *
 */

static int indefinition = FALSE;	/* true if in a definition */
static int behead = FALSE;		/* true if next word is headless */
static int reserve = FALSE;		/* true if next word is reserved */
static char *codefield;			/* string naming code field type */
static int clue;			/* label for leave */
static int vocabtag = 0;		/* Forth wordlist id */
static struct symbol *currsym;		/* symbol of current definition */

/*
 *	Parameter field of current definition is compiled into the
 *	following buffer.
 */
static struct {
	enum {refer, symrefer, arrayrefer, literal, branch} memtype;
	union {
		char *reference;	/* a reference to a definition */
		char *sym;		/* a reference to a symbol, or array */
		cell value;		/* a numeric value */
		unsigned int branchindex; /* index into current definition */
	} memvalue;
} memory[1024];
unsigned int here = 0;			/* cell-addressed free pointer */

/*
 *	Forward References
 */
static void Symrefer();
static void Arrayrefer();
static void Value();
static void Backmark();
static void Backresolve();
static void Forwardmark();
static void Forwardresolve();
static void Forwardmarks();
static void Forwardresolves();
static void Allot();
static void Flushhead();
static char * Namefield();
static void Makehead();
static void Code();
static void Colon();
static void Variable();
static void Constant();
static void Lsconstant();
static void Laconstant();
static void Create();
static void Offset();
static void Region();
static void Alias();
static void Stringvalue();
static void Forthscript();
static void Reserved();
static void Verbatimcode();
static void Verbatimdata();
static void Verbatimdict();
static void Verbatimprolog();
static void Verbatim();
static void Nohead();
static void Rightbracket();
static void Immediate();
static void Wordlist();
static void Hex();
static void Semicolon();
static void Leftbracket();
static void Brackettick();
static void Postpone();
static void Bracketchar();
static void Leftparen();
static void Backslash();
static void Dotquote();
static void Qquote();
static void Cif();
static void Celse();
static void Cthen();
static void Cbegin();
static void Cwhile();
static void Crepeat();
static void Cuntil();
static void Cagain();
static void Cdo();
static void Cloop();
static void Cploop();
static void Cleave();
static void Csel();
static void Cltlt();
static void Ceqgt();
static void Ceqeqgt();
static void Cgtgt();
static void Cendsel();
static void Cdoes();
static void Creturn();
static struct symbol * Lasthead();
static int Forthhash();
static void Checkedrefer();

/***************************************************************************
 *		Interface between compiler and target language.
 *	The following routines build the current definition in memory.
 */

void
Refer(name)
char *name;
{
	memory[here].memtype = refer;
	memory[here++].memvalue.reference = name;
}

static void
Symrefer(name)
char *name;
{
	memory[here].memtype = symrefer;
	memory[here++].memvalue.sym = name;
}

static void
Arrayrefer(name)
char *name;
{
	memory[here].memtype = arrayrefer;
	memory[here++].memvalue.sym = name;
}

static void
Value(number)
cell number;
{
	memory[here].memtype = literal;
	memory[here++].memvalue.value = number;
}

static void
Backmark()
{
	*stackptr++ = here;
}

static void
Backresolve()
{
	memory[here].memtype = branch;
	memory[here++].memvalue.branchindex = *--stackptr;
}

static void
Forwardmark()
{
	*stackptr++ = here;
	memory[here++].memtype = branch;
}

static void
Forwardresolve()
{
	memory[*--stackptr].memvalue.branchindex = here;
}

static void
Forwardmarks()
{
	memory[here].memtype = branch;
	memory[here].memvalue.branchindex = clue;
	clue = here++;
}

static void
Forwardresolves()
{
	unsigned int next, resolve = clue;

	while(resolve){
		next = memory[resolve].memvalue.branchindex;
		memory[resolve].memvalue.branchindex = here;
		resolve = next;
	}
}

static void
Allot()
{
	cell count = *--stackptr;
	cell countincells;		/* BUG: rounds up allot to cell */

	countincells = count/sizeof(cell);
	if(count % sizeof(cell)) countincells++;
	while(countincells--){
		memory[here].memtype = literal;
		memory[here++].memvalue.value = 0;
	}
}

static void
Flushhead()
{
	if(currsym->behead){
		fprintf(dictfp, "HEADLESSDEF(%s,%d)\n", currsym->mangled, here);
	} else {
		struct symbol *last = Lasthead(currsym);
		char *namepadded = Namefield(currsym->name);
		char linkbuf[256];
		int i;
		if(last)	sprintf(linkbuf,"NAMEREF(%s)", last->mangled);
		else		sprintf(linkbuf,"NULLREF()");
		fprintf(dictfp, "%s(%s, %d, ",
			currsym->immediate ? "IHEADEDDEF" : "HEADEDDEF",
			currsym->mangled,
			strlen(currsym->name));
		for(i=0; i<NAMESIZE-1; i++){
			fprintf(dictfp, "%d,", namepadded[i]);
		}
		fprintf(dictfp, " %d, %s, %d)\n",
			vocabtag,
			linkbuf,
			here);
	}
}

static char *
Namefield(name)
char *name;
{
	static char namefield[NAMESIZE-1];
	char *namescan = namefield;
	int i;

	for(i=0; (i<NAMESIZE-1) && name[i]; i++){
		*namescan++ = uppercase ? toupper(name[i]) : name[i];
	}
	for( ; i<NAMESIZE-1; i++){
		*namescan++ = ' ';
	}
	return(namefield);
}

void
Flushmemory()
{
	int i;

	if(indefinition){
		Flushhead();
		fprintf(dictfp, codefield, currsym->mangled);
	}
	for(i=0; i<here; i++){
		switch(memory[i].memtype){
		   case refer:
			fprintf(dictfp, "REFER(%s)\n",
					memory[i].memvalue.reference);
			break;
		   case symrefer:
			fprintf(dictfp, "SYMREFER(%s)\n",
					memory[i].memvalue.sym);
			break;
		   case arrayrefer:
			fprintf(dictfp, "ARRAYREFER(%s)\n",
					memory[i].memvalue.sym);
			break;
		   case literal:
			fprintf(dictfp, "VALUE(0x%x)\n",
					memory[i].memvalue.value);
			break;
		   case branch:
			fprintf(dictfp, "BRANCH(%s,%d)\n", currsym->mangled,
				memory[i].memvalue.branchindex);
			break;
		   default:
			fprintf(stderr,"Unknown memory type\n");
			exit(1);
		}
	}
	here = 0;
	if(indefinition){
		fprintf(dictfp, "ENDDEF()\n");
		indefinition = FALSE;
	}
}

/*
 *		Interface between compiler and target language.
 *
 *	Makehead: Install a name in the dictionary.  Generate
 *		a HEADEDDEF or HEADLESSDEF command as appropriate.
 */

static void
Makehead(name)
char *name;
{
	Flushmemory();
	currsym = Install(name, behead, reserve);
	behead = reserve = FALSE;
	indefinition = TRUE;
}

/*
 *	Code:	Translate a code definition by copying generating
 *		a CODE command and copying input lines between
 *		code and end verbatim.
 */

static void
Code()
{
	Makehead(Word(' '));
	codefield = "SELF(%s)\n";
	currsym->primitive = TRUE;
	Flushmemory();
	fprintf(codefp, "CODE(%s)\n", currsym->mangled);
	Verbatimcode();
	fprintf(codefp, "ENDCODE()\n");
}

/*
 *	Colon:	Start a colon definition: generate a COLON command
 *		and enter compile mode.
 */

static void
Colon()
{
	Makehead(Word(' '));
	codefield = "COLON()\n";
	state = 1;
}

/*
 *	Variable: Create a variable by generating a VARIABLE command.
 */

static void
Variable()
{
	Makehead(Word(' '));
	codefield = "VARIABLE()\n";
	Value(0);			/* default variable contents */
}

/*
 *	Constant: Create a constant by generating a CONSTANT command
 *		from the number of the stack.
 */

static void
Constant()
{
	Makehead(Word(' '));
	codefield = "CONSTANT()\n";
	Value(*--stackptr);
}

static void
Lsconstant()
{
	Makehead(Word(' '));
	codefield = "CONSTANT()\n";
	Symrefer(Strsave(Word(' ')));
}

static void
Laconstant()
{
	Makehead(Word(' '));
	codefield = "CONSTANT()\n";
	Arrayrefer(Strsave(Word(' ')));
}

static void
Create()
{
	Makehead(Word(' '));
	codefield = "CREATE()\n";
	Symrefer("DEFAULTDOES");
}

static void
Offset()
{
	Makehead(Word(' '));
	codefield = "OFFSET()\n";
	Value(*--stackptr);
}

static void
Region()
{
	Makehead(Word(' '));
	codefield = "VARIABLE()\n";
	Forwardmark(); Forwardresolve();
	Allot();
}

/*
 *	Alias:	Create a code defintion whose code field points to
 *		code at the given label.
 *		NOTE: this only works on ITC systems and should not be used
 *		otherwise.
 *		The implementation is a hack.
 */

static void
Alias()
{
	Makehead(Word(' '));
	codefield = "ALIAS()\n";
	Symrefer(Strsave(Word(' ')));
}

/***************************************************************************
 *	Stringvalue: Generate BYTE commands for the given string.
 *		Alignment to quad-byte is done.
 */

static void
Stringvalue(name)
char *name;
{
	unsigned int lengthincells, length=strlen(name);
	union { cell scell[256]; char schar[256];} s;
	cell *scell;

	memset(&s.schar[0], 0, sizeof(s));
	s.schar[0] = (char)length;  strcpy(&s.schar[1],name);
	lengthincells = (length+1)/sizeof(cell);
	if((length+1) % sizeof(cell)) lengthincells++;
	for(scell= &s.scell[0]; lengthincells; scell++, lengthincells--){
		Value(*scell);
	}
}

/*
 *	Forthscript: Start a colon definition: generate a COLON command
 *		and enter compile mode.  The name of the definition
 *		is of the form #!<path>.  Experimental feature.
 */

static void
Forthscript()
{
	char buf[256];
	sprintf(buf, "#!%s", forthpath);
	Makehead(buf);
	codefield = "COLON()\n";
	state = 1;
}

/*
 *	Reserved: The most recent definition uses a reserved word
 *		in the target language; replace it with something
 *		sure to be innocuous.
 */

static void
Reserved()
{
	reserve = TRUE;
}

/*************************************************************************
 */

static void
Verbatimcode()
{
	Verbatim(codefp);
}

static void
Verbatimdata()
{
	Verbatim(datafp);
}

static void
Verbatimdict()
{
	Verbatim(dictfp);
}

static void
Verbatimprolog()
{
	Verbatim(prolfp);
}

static void
Verbatim(fp)
FILE *fp;
{
	char *s;

	Flushmemory();
	while((s = Readline()) != NULL){
		if(strncmp(s, "end", 3) == 0) break;
		Putline(fp);
	}
	(void) Word(' ');		/* discard end */
}

static void
Nohead()
{
	behead = TRUE;			/* behead next definition */
}

static void
Rightbracket()
{
	state=1;
}

static void
Immediate()
{
	currsym->immediate = TRUE;
}

static void
Wordlist()
{
	static int nextwid = 1;		/* 0 is preallocated for Forth-wid */

	*stackptr++ = nextwid++;
}

static void
Hex()
{
	/* TBD  */
}

/*************************************************************************/

static void
Semicolon()
{
	Checkedrefer("return");
	state = 0;
}

static void
Leftbracket()
{
	state=0;
}

static void
Brackettick()
{
	Checkedrefer("(literal)");
	Checkedrefer(Word(' '));
}

static void
Postpone()
{
	char *name = Word(' ');
	struct symbol *sym = Find(name);

	if(sym){
		if(!sym->immediate) Checkedrefer("compile");
		Refer(sym->mangled);
	} else {
		Warn("can't postpone %s\n", name);
	}
}

void
Literal()
{
	Checkedrefer("(literal)");
	Value(*--stackptr);
}

static void
Bracketchar()
{
	*stackptr++ = *Word(' ');
	Literal();
}

static void
Leftparen()
{
	Word(')');
}

static void
Backslash()
{
	Word('\000');
}

static void
Dotquote()
{
	Checkedrefer("(.\")");
	Stringvalue(Word('"'));
}

static void
Qquote()
{
	Checkedrefer("(\")");
	Stringvalue(Word('"'));
}

/*
 *	Control flow words
 */

static void
Cif()
{
	Checkedrefer("?branch");
	Forwardmark();
}

static void
Celse()
{
	int tmp;

	Checkedrefer("branch");
	Forwardmark();
	tmp = *--stackptr;
	Forwardresolve();
	*stackptr++ = tmp;
}

static void
Cthen()
{
	Forwardresolve();
}

static void
Cbegin()
{
	Backmark();
}

static void
Cwhile()
{
	int tmp = *--stackptr;

	Cif();
	*stackptr++ = tmp;
}

static void
Crepeat()
{
	Checkedrefer("branch");
	Backresolve();
	Forwardresolve();
}

static void
Cuntil()
{
	Checkedrefer("?branch");
	Backresolve();
}

static void
Cagain()
{
	Checkedrefer("branch");
	Backresolve();
}

static void
Cdo()
{
	Checkedrefer("(do)");
	*stackptr++ = clue;
	clue = 0;
	Backmark();
}

static void
Cloop()
{
	Checkedrefer("(loop)");
	Backresolve();
	Forwardresolves();
	clue = *--stackptr;
}

static void
Cploop()
{
	Checkedrefer("(+loop)");
	Backresolve();
	Forwardresolves();
	clue = *--stackptr;
}

static void
Csel()
{
	*stackptr++ = 0;
}

static void
Cltlt()
{
	Checkedrefer("dup");
}

static void
Ceqgt()
{
	(*(stackptr-1))++;
	Cif();
	Checkedrefer("drop");
}

static void
Ceqeqgt()
{
	Checkedrefer("=");
	Ceqgt();
}

static void
Cgtgt()
{
	int tmp;

	Celse();
	tmp = *(stackptr-1);
	*(stackptr-1) = *(stackptr-2);
	*(stackptr-2) = tmp;
}

static void
Cendsel()
{
	int i;

	Checkedrefer("drop");
	for(i=*--stackptr; i; i--) {
		Cthen();
	}
}

static void
Cleave()
{
	Forwardmarks();
}

static void
Cdoes()
{
	Checkedrefer("does");
}

static void
Creturn()
{
	Checkedrefer("return");
}

/*
 *	Installdir: Install interpreter and compiler macros.
 */

void
Installdir()
{
	Installmacro(interpmacros, "code", Code);
	Installmacro(interpmacros, "alias", Alias);
	Installmacro(interpmacros, ":", Colon);
	Installmacro(interpmacros, "variable", Variable);
	Installmacro(interpmacros, "create", Create);
	Installmacro(interpmacros, "constant", Constant);
	Installmacro(interpmacros, "lsconstant", Lsconstant);
	Installmacro(interpmacros, "laconstant", Laconstant);
	Installmacro(interpmacros, "offset:", Offset);
	Installmacro(interpmacros, "region", Region);
	Installmacro(interpmacros, "allot", Allot);
	Installmacro(interpmacros, "verbatim", Verbatimcode);
	Installmacro(interpmacros, "verbatim-data", Verbatimdata);
	Installmacro(interpmacros, "verbatim-dict", Verbatimdict);
	Installmacro(interpmacros, "verbatim-prolog", Verbatimprolog);
	Installmacro(interpmacros, "~", Nohead);
	Installmacro(interpmacros, "nohead", Nohead);	/* alias for ~ */
	Installmacro(interpmacros, "]", Rightbracket);
	Installmacro(interpmacros, "immediate", Immediate);
	Installmacro(interpmacros, "hex", Hex);
	Installmacro(interpmacros, "(", Leftparen);
	Installmacro(interpmacros, "\\", Backslash);
	Installmacro(interpmacros, "wordlist", Wordlist);
	Installmacro(interpmacros, "forthscript", Forthscript);
	Installmacro(interpmacros, "reserve", Reserved);

	Installmacro(compilermacros, ";", Semicolon);
	Installmacro(compilermacros, "[", Leftbracket);
	Installmacro(compilermacros, "[']", Brackettick);
	Installmacro(compilermacros, "postpone", Postpone);
	Installmacro(compilermacros, "literal", Literal);
	Installmacro(compilermacros, "[char]", Bracketchar);
	Installmacro(compilermacros, "(", Leftparen);
	Installmacro(compilermacros, "\\", Backslash);
	Installmacro(compilermacros, ".\"", Dotquote);
	Installmacro(compilermacros, "\"\"", Qquote);
	Installmacro(compilermacros, "if", Cif);	
	Installmacro(compilermacros, "then", Cthen);
	Installmacro(compilermacros, "else", Celse);
	Installmacro(compilermacros, "begin", Cbegin);
	Installmacro(compilermacros, "until", Cuntil);
	Installmacro(compilermacros, "again", Cagain);
	Installmacro(compilermacros, "while", Cwhile);
	Installmacro(compilermacros, "repeat", Crepeat);
	Installmacro(compilermacros, "do", Cdo);
	Installmacro(compilermacros, "loop", Cloop);
	Installmacro(compilermacros, "+loop", Cploop);
	Installmacro(compilermacros, "sel", Csel);
	Installmacro(compilermacros, "<<", Cltlt);
	Installmacro(compilermacros, "=>", Ceqgt);
	Installmacro(compilermacros, "==>", Ceqeqgt);
	Installmacro(compilermacros, ">>", Cgtgt);
	Installmacro(compilermacros, "endsel", Cendsel);
	Installmacro(compilermacros, "leave", Cleave);
	Installmacro(compilermacros, "does>", Cdoes);
	Installmacro(compilermacros, "exit", Creturn);
}

/*************************************************************************
 *
 *	Forth Dictionary Hash Table Simulation
 *
 *	Lasthead: Install the given symbol at the beginning of the
 *		appropriate hash table collision chain and return
 *		the symbol originally at the beginning of the chain.
 */

static struct symbol *lastheads[LASTHEADSSIZE];

static struct symbol *
Lasthead(s)
struct symbol *s;
{
	unsigned int tableindex = Forthhash(s->name);

	s->nextdictionary = lastheads[tableindex];
	lastheads[tableindex] = s;
	return(s->nextdictionary);
}

/*
 *	Forthhash: Hash input name by adding up characters.
 */

static int
Forthhash(name)
char *name;
{
	int sum = 0;

	if(uppercase) while(*name) sum += (int)toupper(*name++);
	else	      while(*name) sum += (int)*name++;
	return(sum & (LASTHEADSSIZE-1));
}

/*
 *	Dumplastheads: Dump the Forth dictionary hash table in
 *		macro language format.
 */

void
Dumplastheads()
{
	int i;
	struct symbol *s;

	printf("TABLE(HASHTABLE)\n");
	for(i=0; i<LASTHEADSSIZE; i++){
		if((s=lastheads[i]) == NULL)	printf("NULLREF()\n");
		else			printf("NAMEREF(%s)\n", s->mangled);
	}
	printf("ENDTABLE()\n");
}

/*************************************************************************
 *
 *	Helpers
 *
 *	Checkedrefer: Check given name to see if it appears in the
 *		Forth dictionary; give a forward reference warning
 *		if not.  Compile a reference to the name regardless.
 */

static void
Checkedrefer(name)
char *name;
{
	struct symbol *sym;

	if((sym=Find(name)) != NULL){
		Refer(sym->mangled);
	} else {
		Refer(Strsave(Mangle(name)));
		if(forwardwarn) Warn("forward reference to %s\n",name);
	}
}

/*
 *	Mangle:	Convert a string consisting of arbitrary characters
 *		into one where the only characters are _ (underscore),
 *		A-Z (upper case letters), and a-z (lower case letters).
 */

char *
Mangle(name)
char *name;
{
	static char buf[80];
	char *d;
	char *s;
	for(s = name, d = buf; *s; s++){
		if(*s>='!' && *s<=':'){
			*d++ = '_'; *d++ = *s + 'A' - '!';
		} else if(*s>=';' && *s<='@'){
			*d++ = '_'; *d++ = *s + 'a' - ';';
		} else if(*s>='A' && *s<='Z'){
			*d++ = *s;
		} else if(*s>='[' && *s<='`'){
			*d++ = '_'; *d++ = *s + 'g' - '[';
		} else if(*s>='a' && *s<='z'){
			*d++ = *s;
		} else if(*s>='{' && *s<='~'){
			*d++ = '_'; *d++ = *s + 'm' - '{';
		} else {
			Warn("bad string to mangle: %s\n", name);
		}
	}
	*d = '\0';
	return(buf);
}
