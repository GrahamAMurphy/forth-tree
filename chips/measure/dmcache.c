/*	Direct-mapped (with sub-block placement) cache simulator
	Simulates a direct mapped cache given a trace of addresses.
	The addresses may be from instruction or data fetches; by default,
	only instruction fetches are considered.
	The addresses must be from a cell-addresses computer since
	all of the lsbs are used to compute tag address.
	If the line-size is greater than one cell, a newly allocated cache
	will contain only one cell.
	For mixed mode caches, data is written throught the cache; a variety
	of write allocation policies are possible; 

	The options are:
	-v	  Verbose summary mode; default is to report miss rate only.
	-b size	  Block/line size (in cells).
	-s size	  Size of cache (in cells).
	-c access Number of accesses to run before resetting
		miss history (this eliminates cold start behavior).
	-l access Number of accesses to simulate (after cold start).

	The following imply a mixed address/data cache (write-through).
	-ma	  Always allocate a cache line on write.
	-mm	  Allocate a cache line only on write miss.
	-mn	  Never allocate a cache line on a write.
 */

#include <stdio.h>

#define FALSE 0
#define TRUE 1

#define USAGE \
"usage: idmcache [-v] [-m(a|m|n)] [-b sz] [-s sz] [-c n] [-l n] [file]\n"

#define INSTR(x)	(((x)&0x80000000)==0)
#define READ(x)		(((x)&0x40000000)==0)
#define ADDR(x)		((x) &0x3fffffff)

unsigned int tagmask;
unsigned int setmask;
unsigned int linebits;
unsigned int *tagram;			/* dynamically allocated tag RAM */
unsigned int *valid;			/* valid bits */

#define INDEX(x)\
	(((x) & setmask)>>linebits)
#define MISS(x)\
	((tagram[INDEX(x)] != ((x) & tagmask)) ||\
	 (!valid[INDEX(x)]))
#define CACHE(x)\
	tagram[INDEX(x)] = (x) & tagmask;\
	valid[INDEX(x)] = TRUE;

unsigned int size = 256;		/* cache size (in cells) */
unsigned int linesize = 1;		/* block/line size */

unsigned int cpucycle;
unsigned int cachestall;
unsigned int miss;
unsigned int externalread;
unsigned int externalwrite;

void Run();
void Cacheinit();
void Simulate();
void SimulateMixedAlwaysAlloc();
void SimulateMixedMissAlloc();
void SimulateMixedNeverAlloc();
void Summarize();

main(argc,argv)
int argc;
char *argv[];
{
	int c;
	extern int optind;		/* for getopt() */
	extern char *optarg;
	int mixedcache = FALSE;		/* instruction vs. instruction/data */
	char *writeallocpolicy = "m";	/* default: allocate on write miss */
	unsigned int coldlength = 0;	/* cold start length */
	unsigned int length = 512;	/* trace length */
	int verbose = FALSE;		/* verbosity of summary report */
	FILE *fp = stdin;		/* standard input by default */

	while((c=getopt(argc,argv,"b:c:l:m:s:v")) != EOF){
		switch(c){
		   case 'b': linesize = atoi(optarg);
			     break;
		   case 'c': coldlength = atoi(optarg);
			     break;
		   case 'l': length = atoi(optarg);
			     break;
		   case 'm': mixedcache = TRUE; writeallocpolicy = optarg;
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
			fprintf(stderr,
				"dmcacche: can't open %s\n", argv[optind]);
			exit(1);
		}
	}
	Cacheinit(size,linesize);
	Run(mixedcache, writeallocpolicy, fp, coldlength);
	Run(mixedcache, writeallocpolicy, fp, length);
	Summarize(verbose);
	return(0);
}

void
Run(mixedcache, writeallocpolicy, fp, length)
int mixedcache;
char *writeallocpolicy;
FILE *fp;
unsigned int length;
{
	cpucycle = cachestall = miss = externalread = externalwrite = 0;
	if(mixedcache) {
		switch(*writeallocpolicy){
		   case 'a':	SimulateMixedAlwaysAlloc(fp,length); break;
		   case 'm':	SimulateMixedMissAlloc(fp,length); break;
		   case 'n':	SimulateMixedNeverAlloc(fp,length); break;
		   default:	fprintf(stderr,"Unknown write alloc policy\n");
				exit(1);
		}
	} else {
		Simulate(fp,length);
	}
}

void
Cacheinit(size,linesize)
unsigned int size, linesize;
{
	unsigned int i;
	unsigned int numberoftags = size/linesize;
	unsigned int tmp;

	tagram = (unsigned int *)malloc(numberoftags * sizeof(unsigned int));
	if(tagram == NULL){
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	valid  = (unsigned int *)malloc(numberoftags * sizeof(unsigned int));
	if(valid == NULL){
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	for(i=0; i<numberoftags; i++) valid[i] = FALSE;
	for(linebits=0, tmp=linesize; tmp>1; tmp>>=1) linebits++;
	setmask = size-1;
	tagmask = ~setmask;
	setmask ^= linesize-1;
}

void
Simulate(fp,remainingaccess)
FILE *fp;
unsigned int remainingaccess;
{
	unsigned int trace, x;

	while(!feof(fp) && remainingaccess){
		cpucycle++;
		trace = getw(fp);
		if(INSTR(trace)){
			x = ADDR(trace);
			if(MISS(x)){
				miss++;
				if(READ(trace))	externalread+=linesize;
				else		/* can't happen */ ;
				CACHE(x);
			}
		} else {
			miss++;
			if(READ(trace))	externalread++;
			else		externalwrite++;
		}
		remainingaccess--;
	}
}

void
SimulateMixedAlwaysAlloc(fp,remainingaccess)
FILE *fp;
unsigned int remainingaccess;
{
	unsigned int trace, x;

	while(!feof(fp) && remainingaccess){
		cpucycle++;
		trace = getw(fp);
		x = ADDR(trace);
		if(READ(trace)){
			if(MISS(x)){
				miss++;
				externalread+=linesize;
				CACHE(x);
			}
		} else {
			if(MISS(x)){
				miss++; /* TBD: is this right? */
			}
			externalwrite++;
			externalread+=(linesize-1);
			CACHE(x);
		}
		remainingaccess--;
	}
}

void
SimulateMixedMissAlloc(fp,remainingaccess)
FILE *fp;
unsigned int remainingaccess;
{
	unsigned int trace, x;

	while(!feof(fp) && remainingaccess){
		cpucycle++;
		trace = getw(fp);
		x = ADDR(trace);
		if(READ(trace)){
			if(MISS(x)){
				miss++;
				externalread+=linesize;
				CACHE(x);
			}
		} else {
			externalwrite++;
			cachestall++;  /* TBD can be in parallel with write? */
			if(MISS(x)){
				miss++;
				externalread+=(linesize-1);
				CACHE(x);
			}
		}
		remainingaccess--;
	}
}

void
SimulateMixedNeverAlloc(fp,remainingaccess)
FILE *fp;
unsigned int remainingaccess;
{
	unsigned int trace, x;

	while(!feof(fp) && remainingaccess){
		cpucycle++;
		trace = getw(fp);
		x = ADDR(trace);
		if(READ(trace)){
			if(MISS(x)){
				miss++;
				externalread+=linesize;
				CACHE(x);
			}
		} else {
			externalwrite++;
			if(MISS(x)){
				miss++;
			}
		}
		remainingaccess--;
	}
}

void
Summarize(verbose)
int verbose;
{
	if(verbose){
		printf("%d:%d:%d:%d\n",
			cpucycle, cachestall, externalread, externalwrite);
	} else {
		printf("%f\n",
			(float)miss /
			(float)cpucycle);
	}
}
