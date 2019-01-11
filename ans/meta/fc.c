/* (c) 1992 Johns Hopkins University / Applied Physics Laboratory */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "fc.h"

/****************************************************************************
 *			Program usage
 *	-m merges the code and dictionary (for DTC systems).
 *	-u generates upper-case dictionary entries (for ANS Forth conformance).
 *	-w warns about forward references.
 *	-o controls format of output. The argument is a string characters:
 *		'p' = prolog, 'c' = code, 'd' = data, 'i' = dictionary,
 *		't' = token list, and 'h' = hash table.
 */

#define USAGE "usage: fc [-m] [-u] [-w] [-o x] [-i dir] fl [fl ...] \n"

/****************************************************************************
 *
 *			global variables
 *
 */

/*	stack for forth outer interpreter */
cell stack[32];				/* stack */
cell *stackptr=stack;

int state=0;				/* compile state */
					/*   0 = interpret */
					/*   1 = compile */

/*	hash tables of structures describing compiler directives */
struct macro *compilermacros[HASHSIZE];
struct macro *interpmacros[HASHSIZE];

/* forward reference warnings */
int forwardwarn = FALSE;

/* convert dictionary names to upper case */
int uppercase = FALSE;

/* full path name of Forth system will reside. Experimental feature */
char *forthpath = "/home/users/local/bin/forth";  /* plausible default */

/* current source file name */
static char *srcfile;
/* current source code file pointer */
static FILE *srcfp;
/* current line number */
static int linenumber;

/* prolog tmpfile file pointer */
FILE *prolfp;
/* code tmpfile file pointer */
FILE *codefp;
/* data tmpfile file pointer */
FILE *datafp;
/* dictionary tmpfile file pointer */
FILE *dictfp;

/*
 *	Forward references
 */
static void Copyfile();
static void Fc();
static void Interp();
static int Number();
static int Hash();
static void Dumpprims();

/**************************************************************************
 *
 *			Main Program
 *
 *	fc compiles the given files in the order they appear on the 
 *	command line.
 *
 **************************************************************************/

main(argc,argv)
int argc;
char *argv[];
{
	int c;				/* command line flag */
	extern int optind;		/* for getopt() */
	extern char *optarg;
	int merge = FALSE;
	char *outputcontrol = "tdich",	/* default output format */
	     *s;

	while((c=getopt(argc,argv,"i:mo:uw")) != EOF){
		switch(c){
		   case 'i': forthpath = optarg; break;
		   case 'm': merge = TRUE; break;
		   case 'o': outputcontrol = optarg; break;
		   case 'u': uppercase = TRUE; break;
		   case 'w': forwardwarn = TRUE; break;
		   case '?': fprintf(stderr,USAGE);
			     exit(1);
			     break;
		}
	}
	Installdir();			/* setup up compiler directive list */
	dictfp = tmpfile();		/* tmp files for code and dictionary */
	codefp = merge ? dictfp : tmpfile();
	datafp = merge ? dictfp : tmpfile();
	prolfp = tmpfile();
	if(optind<argc){		/* if there are file arguments */
		for(;optind<argc;optind++){
			if((srcfp=fopen((srcfile=argv[optind]),"r")) == NULL){
				fprintf(stderr,
					"fc: can't open %s\n",srcfile);
				exit(1);
			}
			linenumber = 0;
			Fc();
			Flushmemory();
			fclose(srcfp);
		}
	} else {
		fprintf(stderr,USAGE);
		exit(1);
	}
	rewind(prolfp); rewind(dictfp); rewind(codefp); rewind(datafp);
	for(s = outputcontrol; *s; s++){
		switch(*s){
		   case 'c': Copyfile(codefp); break;
		   case 'd': Copyfile(datafp); break;
		   case 'i': Copyfile(dictfp); break;
		   case 'p': Copyfile(prolfp); break;
		   case 't': Dumpprims(); break;
		   case 'h': Dumplastheads(); break;
		   default: { fprintf(stderr, "Unknown output: %c\n", *s);
				exit(1); }
		}
	}
	return(0);
}

static void
Copyfile(fp)
FILE *fp;
{
	char buf[BUFSIZ];
	int actlen;
	while((actlen=fread(buf, sizeof(char), BUFSIZ, fp))){
		fwrite(buf, sizeof(char), actlen, stdout);
	}
}

/***************************************************************************
 *
 *		    forth compiler: simulates forth outer interpreter
 *
 *	fc:	processes one forth source code file.  Reads source text
 *		one line at a time, extracts each word from the line, and
 *		applies interp to the word.
 */

static void
Fc()
{
	char *s;			/* current line */
	char *name;			/* next name */

	while((s=Readline()) != NULL){	/* while there are input lines */
#ifdef DEBUG
		printf("%s",s);
#endif
		while(*(name=Word(' ')) != '\0'){
					/* while there are words on line */
			Interp(name);
			if(stackptr<stack){
					/* check for stack underflow */
				Warn("stack underflow\n");
				stackptr=stack;
			}
		}
	}
}

/*
 *	interp:	This is the key routine in the compiler.  It simulates the
 *		action of interpret in a forth outer interpreter.  Different
 *		actions are performed according to the compile state.
 */

static void
Interp(name)
char *name;
{
	struct macro *mac;
	struct symbol *sym;
	unsigned int num;		/* a number */

	if(state){
		if((mac=Findmacro(compilermacros, name)) != NULL){
			mac->doit();
		} else if((sym=Find(name)) != NULL){
			Refer(sym->mangled);
		} else if(Number(name,&num)){
			*stackptr++ = num;
			Literal();
		} else {
			Refer(Strsave(Mangle(name)));
			if(forwardwarn) Warn("forward reference to %s\n",name);
		}
	} else {
		if((mac=Findmacro(interpmacros, name)) != NULL){
			mac->doit();
		} else if(Number(name,&num)){
			*stackptr++  = num;
		} else {
			Warn("can't execute %s\n",name);
		}
	}
}

/*
 *	number:	Attempts to convert the given name into a hex number.
 *		If successful, the converted value is returned in numptr,
 *		and TRUE is returned.  Otherwise, FALSE is returned.
 */

static int
Number(name,numptr)
char *name;
unsigned int *numptr;
{
	char *scan=name;

	if(*scan=='-') scan++;		/* is first character a '-' */
	do{				/* scan 1 or more hex digits */
		if(!isxdigit(*scan++)){
			return(FALSE);
		}
	} while(*scan);
	sscanf(name,"%x",numptr);	/* convert number to hex */
	return(TRUE);
}

/*************************************************************************
 *		Line buffer management
 *
 *	Readline: Read the next line into a buffer.
 */

static char linebuf[LINESIZE];		/* input line buffer */
static char *bufptr;

char *
Readline()
{
	linenumber++;
	return(bufptr = fgets(linebuf, LINESIZE, srcfp));
}

/*	Putline: Emit the current line.
 */

void
Putline(fp)
FILE *fp;
{
	fputs(linebuf, fp);
}

/*
 *	Word:	Parse next word on input line using input delimiter.  
 *		Returns pointer to parsed word stored in a static area.
 *		If there are no more words in the line, a null string is
 *		returned.
 */

char *
Word(del)
char del;
{
	static char name[80];		/* next input word */
	char *nameptr;			/* pointer to name in stream */

	if(del == ' '){			/* white space? */
		while(*bufptr <= ' ' && *bufptr!='\n')
			bufptr++;	/* find first non-delimiter */
		nameptr=bufptr;		/* remember start of word */
		while(*bufptr>' ')
			bufptr++;	/* find delimiter or EOL */
	} else {
		while(*bufptr == del)	/* find first non-delimiter */
			bufptr++;
		nameptr=bufptr;		/* remember start of word */
		while(*bufptr!=del && *bufptr!='\n')
			bufptr++;	/* find delimiter or EOL */
	}
	strncpy(name,nameptr,bufptr-nameptr);
	name[bufptr-nameptr] = '\0';	/* copy word to static buffer */
	if(*bufptr!='\n') bufptr++;	/* do not go past \n character */
	return(name);
}

/***************************************************************************
 *
 *		Compiler Directive Data Structure Management
 *
 *	Installmacro: Install a function with the given name in a table.
 */

void
Installmacro(table, name, fn)
struct macro *table[];
char *name;
void (*fn)();
{
	int tableindex = Hash(name);
	struct macro *new = (struct macro *)malloc(sizeof(struct macro));

	new->name = Strsave(name);
	new->doit = fn;
	new->next = table[tableindex];
	table[tableindex] = new;
}

/*
 *	Findmacro:  Find function with given name in table.  If not
 *		found, return NULL.
 */

struct macro *
Findmacro(table, name)
struct macro *table[];
char *name;
{
	struct macro *s = table[Hash(name)];

	while(s){
		if(STREQ(s->name, name)) break;
		s = s->next;
	}
	return(s);
}

/*
 *	Hash:	Hash input name by adding up characters.
 */

static int
Hash(name)
char *name;
{
	int sum = 0;

	while(*name) sum += (int)*name++;
	return(sum & HASHMASK);
}

/***************************************************************************
 *
 *		Forth Words Data Structure Management
 *
 *	Install: Install given word (with immediate indicator)
 *		in dictionary.
 */

struct symbol *forthwords[HASHSIZE];

struct symbol *
Install(name, behead, reserve)
char *name;
int behead, reserve;
{
	int tableindex = Hash(name);
	struct symbol *new = (struct symbol *)malloc(sizeof(struct symbol));

	new->name = Strsave(name);
	if(reserve){
		static int counter = 0;
		char buf[100];
		sprintf(buf, "R%d", counter++);
		new->mangled = Strsave(buf);
	} else {
		new->mangled = Strsave(Mangle(name));
	}
	new->immediate = FALSE;
	new->primitive = FALSE;
	new->behead = behead;
	new->nextdictionary = NULL;
	new->next = forthwords[tableindex];
	forthwords[tableindex] = new;
	return(new);
}

/*
 *	Find:	Find Forth word with given name.  If not
 *		found, return NULL.
 */

struct symbol *
Find(name)
char *name;
{
	struct symbol *s = forthwords[Hash(name)];

	while(s){
		if(STREQ(s->name, name)) break;
		s = s->next;
	}
	return(s);
}

static void
Dumpprims()
{
	unsigned int i;
	struct symbol *s;

	printf("PRIMLIST()\n");
	for(i=0; i<HASHSIZE; i++){
		for(s=forthwords[i]; s; s=s->next){
			if(s->primitive) printf("PRIM(%s)\n", s->mangled);
		}
	}
	printf("ENDPRIMLIST()\n");
}

/***************************************************************************
 *
 *		Other storage management helpers.
 *
 *	Strsave: Allocate memory for given string, copy string into
 *		new memory, and return new string.
 */

char *
Strsave(string)
char *string;
{
	char *new;
	if((new=(char *)malloc(strlen(string)+1))==NULL){
		fprintf(stderr,"fc: can't allocate any more space\n");
		exit(1);
	}
	strcpy(new,string);
	return(new);
}

/***************************************************************************
 *
 *		Errors
 *
 *	Warn:	Print warning message for current file and line.
 */

void
Warn(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "fc: warning: (%s:%d) ", srcfile, linenumber);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

