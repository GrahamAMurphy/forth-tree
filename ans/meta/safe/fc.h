/* (c) 1992 Johns Hopkins University / Applied Physics Laboratory */
/*		general purpose constants and macros		*/

#define FALSE	(0)
#define TRUE	(1)

#define STREQ(x,y) (strcmp(x,y)==0)

/*		data structures		*/

typedef unsigned int cell;

struct macro {
	struct macro *next;		/* next one in list */
	char *name;			/* name of macro */
	void (*doit)();			/* function to perform  macro */
};

struct symbol {
	struct symbol *next;		/* next in symbol table */
	struct symbol *nextdictionary;	/* next in Forth dictionary hash chain*/
	char *name;			/* name of Forth word */
	char *mangled;			/* mangled name of Forth word */
	int immediate;			/* immediate flag */
	int primitive;			/* primitive flag */
	int behead;			/* behead flag */
};

/*		compile time definitions		*/

#define LINESIZE	(1024)		/* line buffer size */

#define HASHSIZE	(1024)		/* hash table size (power of 2) */
#define HASHMASK	(HASHSIZE-1)

/*		Forth run-time data definitions		*/

#define NAMESIZE 6			/* 6 bytes in name field */

#define LASTHEADSSIZE	(256)		/* dictionary hash table size (^2) */

/*		global definitions */

extern cell *stackptr;

extern int state;			/* compile state */

extern struct macro *compilermacros[];
extern struct macro *interpmacros[];

extern int forwardwarn;

extern int uppercase;

extern char *forthpath;

extern FILE *prolfp, *codefp, *datafp, *dictfp;

char * Readline();
void Putline();
char * Word();
void Installmacro();
struct macro * Findmacro();
struct symbol * Install();
struct symbol * Find();
char * Strsave();
void Warn();
void Refer();
void Flushmemory();
void Literal();
void Installdir();
void Dumplastheads();
char * Mangle();
