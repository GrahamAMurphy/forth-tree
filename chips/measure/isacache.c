/*	Instruction cache simulator
	Simulates a set-associative cache given a trace of addresses.
	The addresses may be from instruction or data fetches; by default
	only instructions fetches are considered.
	The addresses must be from a cell-addresses computer since
	all of the lsbs are used to compute tag address.

	The options are:
	-v	  Verbose summary mode; default is to report miss rate only.
	-a	  Degree of associativity.
	-b	  Block/line size (in instructions).
	-s size	  Size of cache (in instructions).
	-c instr  Number of instructions to run before resetting
		miss history (this eliminates cold start behavior).
	-l instr  Number of instructions to simulate (after cold start).
	-m	  Mixed address/data cache (write-through).

	This program is a modified version of idmcache.c
 */

#include <stdio.h>

#define FALSE 0
#define TRUE 1

#define USAGE \
"usage: idmcache [-v] [-a asc] [-b line] [-s sz] [-c instr] [-l instr] [-m] [file]\n"

#define INSTR(x)	(((x)&0x80000000)==0)
#define ADDR(x)		((x) &0x3fffffff)

unsigned int setmask;
unsigned int tagmask;
unsigned int linebits;
unsigned int **tags;			/* dynamically allocated tag RAM */
unsigned int **valid;			/* valid bits */

#define INDEX(x)\
	(((x) & setmask)>>linebits)
#define INSET(x,y)\
	((tags[INDEX(x)][(y)] == ((x) & tagmask)) &&\
	 (valid[INDEX(x)][(y)]))
#define CACHE(x,y)\
	tags[INDEX(x)][(y)] = (x) & tagmask;\
	valid[INDEX(x)][(y)] = TRUE;
#define REPLACEMENT \
	((unsigned int)random() & (assoc-1))

unsigned int size = 256;		/* cache size (in instructions) */
unsigned int assoc = 1;			/* degree of associativity */
unsigned int cachableaccesses = 0;
unsigned int uncachableaccesses = 0;
unsigned int misses = 0;

void Cacheinit();
void Simulate();
void Simulatemixed();
void Summarize();

main(argc,argv)
int argc;
char *argv[];
{
	int c;
	extern int optind;		/* for getopt() */
	extern char *optarg;
	unsigned int linesize = 1;	/* block/line size */
	unsigned int coldlength = 0;	/* cold start length */
	unsigned int length = 512;	/* trace length */
	int mixedcache = FALSE;		/* instructions vs. instructions/data */
	int verbose = FALSE;		/* verbosity of summary report */
	FILE *fp = stdin;		/* standard input by default */

	while((c=getopt(argc,argv,"a:b:c:l:ms:v")) != EOF){
		switch(c){
		   case 'a': assoc = atoi(optarg);
			     break;
		   case 'b': linesize = atoi(optarg);
			     break;
		   case 'c': coldlength = atoi(optarg);
			     break;
		   case 'l': length = atoi(optarg);
			     break;
		   case 'm': mixedcache = TRUE;
			     break;
		   case 's': size = atoi(optarg);
			     break;
		   case 'v': verbose = TRUE;
			     break;
		   case '?': fprintf(stderr,USAGE);
			     exit(1);
			     break;
		}
	}
	if(optind < argc-1){
		fprintf(stderr,USAGE);
		exit(1);
	}
	if(optind == argc-1){
		if((fp=fopen(argv[optind],"r")) == NULL){
			fprintf(stderr,"instr: can't open %s\n", argv[optind]);
			exit(1);
		}
	}
	Cacheinit(size,linesize,assoc);
	mixedcache ? Simulatemixed(fp,coldlength) : Simulate(fp,coldlength);
	cachableaccesses = uncachableaccesses = misses = 0;
	mixedcache ? Simulatemixed(fp,length) : Simulate(fp,length);
	Summarize(verbose);
	return(0);
}

void
Cacheinit(size,linesize,assoc)
unsigned int size, linesize, assoc;
{
	unsigned int i;
	unsigned int numberoftags = size/linesize;
	unsigned int tmp;
	unsigned int *tagram;
	unsigned int *validram;

	tagram = (unsigned int *)malloc(numberoftags * sizeof(unsigned int));
	if(tagram == NULL){
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	validram  = (unsigned int *)malloc(numberoftags * sizeof(unsigned int));
	if(validram == NULL){
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	for(i=0; i<numberoftags; i++) validram[i] = FALSE;
	tags = (unsigned int **)malloc((numberoftags/assoc)
						* sizeof(unsigned int));
	if(tags == NULL){
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	valid = (unsigned int **)malloc((numberoftags/assoc)
						* sizeof(unsigned int));
	if(valid == NULL){
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	for(i=0; i<(numberoftags/assoc); i++){
		tags[i] = tagram;	tagram += assoc;
		valid[i] = validram;	validram += assoc;
	}
	for(linebits=0, tmp=linesize; tmp>1; tmp>>=1) linebits++;
	tagmask = ~((size/assoc)-1);
	setmask = ~(tagmask | (linesize-1));
	srandom(0);
}

void
Simulate(fp,remainingaccess)
FILE *fp;
unsigned int remainingaccess;
{
	unsigned int trace, x;
	unsigned int miss;
	unsigned int set;

	while(!feof(fp) && remainingaccess){
		trace = getw(fp);
		if(INSTR(trace)){
			x = ADDR(trace);
#ifdef DEBUG
			printf("%x", x);
#endif
			for(miss=TRUE,set=0; set<assoc; set++)
				if(INSET(x,set)) { miss = FALSE; break; }
			if(miss){
				misses++;
				CACHE(x,REPLACEMENT);
#ifdef DEBUG
				printf(" miss");
#endif
			}
			cachableaccesses++;
#ifdef DEBUG
			putchar('\n');
#endif
		} else {
			uncachableaccesses++;
		}
		remainingaccess--;
	}
}

void
Simulatemixed(fp,remainingaccess)
FILE *fp;
unsigned int remainingaccess;
{
	unsigned int trace, x;
	unsigned int miss;
	unsigned int set;

	while(!feof(fp) && remainingaccess){
		trace = getw(fp);
		x = ADDR(trace);
#ifdef DEBUG
		printf("%x", x);
#endif
		for(miss=TRUE,set=0; set<assoc; set++)
			if(INSET(x,set)) { miss = FALSE; break; }
		if(miss){
			misses++;
			CACHE(x,REPLACEMENT);
#ifdef DEBUG
			printf(" miss");
#endif
		}
		cachableaccesses++;
		remainingaccess--;
#ifdef DEBUG
		putchar('\n');
#endif
	}
}

void
Summarize(verbose)
int verbose;
{
	if(verbose){
		printf("%d:%d:%d\n", cachableaccesses, uncachableaccesses,
			misses);
	} else {
		printf("%f\n",(float)misses/(float)cachableaccesses);
	}
}
