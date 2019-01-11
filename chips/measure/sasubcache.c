/*	Set-associative (with sub-block placement) cache simulator
	Simulates a set-associative cache given a trace of addresses.
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
	-a size	  Associativity (default = 1)
	-b size	  Block/line size (in cells).
	-s size	  Size of cache (in cells).
	-c access Number of accesses to run before resetting
		miss history (this eliminates cold start behavior).
	-l access Number of accesses to simulate (after cold start).

	The following imply a mixed address/data cache (write-through).
	-mm	  Allocate a cache line only on write miss.
	-mn	  Never allocate a cache line on a write.
 */

#include <stdio.h>

#define FALSE 0
#define TRUE 1

#define USAGE \
"usage: idmcache [-v] [-m(m|n)] [-a sz] [-b sz] [-s sz] [-c n] [-l n] [file]\n"

#define INSTR(x)	(((x)&0x80000000)==0)
#define READ(x)		(((x)&0x40000000)==0)
#define ADDR(x)		((x) &0x3fffffff)

unsigned int tagmask;
unsigned int setmask;
unsigned int submask;
unsigned int linebits;
unsigned int **tagram;			/* dynamically allocated tag RAMs */
unsigned int **valid;			/* valid bits */

#define INDEX(x)\
	(((x) & setmask)>>linebits)
#define SUBINDEX(x)\
	((x)&submask)
#define INLINE(i,x)\
	(tagram[i][INDEX(x)] == ((x) & tagmask))
#define SUBMISS(i,x)\
	 ((valid[i][INDEX(x)] & (1<<SUBINDEX(x))) == 0)
#define ALLOCLINE(i,x)\
	tagram[i][INDEX(x)] = (x) & tagmask;\
	valid[i][INDEX(x)] = 0
#define ADDTOLINE(i,x)\
	valid[i][INDEX(x)] |= 1<<SUBINDEX(x)
#define CHOOSE \
	((unsigned int)random() & (assoc-1))

unsigned int size = 256;		/* cache size (in cells) */
unsigned int assoc = 1;			/* associativity */
unsigned int linesize = 1;		/* block/line size */

unsigned int cpucycle;
unsigned int cachestall;
unsigned int miss;
unsigned int externalread;
unsigned int externalwrite;

void Run();
void Cacheinit();
void Simulate();
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

	while((c=getopt(argc,argv,"a:b:c:l:m:s:v")) != EOF){
		switch(c){
		   case 'a': assoc = atoi(optarg);
			     break;
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
	Cacheinit();
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
Cacheinit()
{
	unsigned int i, j;
	unsigned int numberoftags = size/(linesize*assoc);
	unsigned int tmp;

	tagram = (unsigned int **)malloc(assoc * sizeof(unsigned int *));
	if(tagram == NULL){
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	for(i=0; i<assoc; i++){
		tagram[i] = (unsigned int *)malloc(numberoftags *
							sizeof(unsigned int));
		if(tagram[i] == NULL){
			fprintf(stderr, "malloc failed\n");
			exit(1);
		}
	}
	valid  = (unsigned int **)malloc(assoc * sizeof(unsigned int *));
	if(valid == NULL){
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	for(i=0; i<assoc; i++){
		valid[i]  = (unsigned int *)malloc(numberoftags *
							sizeof(unsigned int));
		if(valid[i] == NULL){
			fprintf(stderr, "malloc failed\n");
			exit(1);
		}
	}
	for(i=0; i<assoc; i++) for(j=0; j<numberoftags; j++) valid[i][j]=FALSE;
	for(linebits=0, tmp=linesize; tmp>1; tmp>>=1) linebits++;
	setmask = (size/assoc)-1;
	tagmask = ~setmask;
	submask = linesize-1;
	setmask ^= submask;
}

void
Simulate(fp,remainingaccess)
FILE *fp;
unsigned int remainingaccess;
{
	unsigned int trace;

	while(!feof(fp) && remainingaccess){
		cpucycle++;
		trace = getw(fp);
		if(INSTR(trace)){
			int inset, setindex;
			unsigned int x = ADDR(trace);
			for(inset=FALSE,setindex=0; setindex<assoc; setindex++){
				if(INLINE(setindex,x)){
					inset=TRUE; break;
				}
			}
			if(!inset){
				miss++;
				externalread++;
				setindex = CHOOSE;
				ALLOCLINE(setindex,x);
				ADDTOLINE(setindex,x);
			} else if(SUBMISS(setindex,x)){
				miss++;
				externalread++;
				ADDTOLINE(setindex,x);
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
SimulateMixedMissAlloc(fp,remainingaccess)
FILE *fp;
unsigned int remainingaccess;
{
	unsigned int trace, x;
	int inset, setindex;

	while(!feof(fp) && remainingaccess){
		cpucycle++;
		trace = getw(fp);
		x = ADDR(trace);
		for(inset=FALSE,setindex=0; setindex<assoc; setindex++){
			if(INLINE(setindex,x)){
				inset=TRUE; break;
			}
		}
		if(READ(trace)){
			if(!inset){
				miss++;
				externalread++;
				setindex = CHOOSE;
				ALLOCLINE(setindex,x);
				ADDTOLINE(setindex,x);
			} else if(SUBMISS(setindex,x)){
				miss++;
				externalread++;
				ADDTOLINE(setindex,x);
			}
		} else {
			externalwrite++;
			cachestall++;
			if(!inset){
				miss++;
				setindex = CHOOSE;
				ALLOCLINE(setindex,x);
				ADDTOLINE(setindex,x);
			} else if(SUBMISS(setindex,x)){
				miss++;
				ADDTOLINE(setindex,x);
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
	int inset, setindex;

	while(!feof(fp) && remainingaccess){
		cpucycle++;
		trace = getw(fp);
		x = ADDR(trace);
		for(inset=FALSE,setindex=0; setindex<assoc; setindex++){
			if(INLINE(setindex,x)){
				inset=TRUE; break;
			}
		}
		if(READ(trace)){
			if(!inset){
				miss++;
				externalread++;
				setindex = CHOOSE;
				ALLOCLINE(setindex,x);
				ADDTOLINE(setindex,x);
			} else if(SUBMISS(setindex,x)){
				miss++;
				externalread++;
				ADDTOLINE(setindex,x);
			}
		} else {
			externalwrite++;
			if(!inset){
				miss++;
			} else if(SUBMISS(setindex,x)){
				miss++;
				cachestall++;
				ADDTOLINE(setindex,x);
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
