/*	Instruction direct-mapped (with dynamic exclusion) cache simulator
	Simulates a direct mapped cache given a trace of addresses.
	The addresses may be from instruction or data fetches; data
	fetches are ignored.
	The addresses must be from a cell-addresses computer since
	all of the lsbs are used to compute tag address.

	The cache replacement algorithm attempts to exclude data that
	will not be used.  Each cache entry includes a counter that
	records the number of misses on that entry.  The counter is
	zeroed on hits and when the entry is replaced.  Each miss
	increments the count.

	The options are:
	-v	  Verbose summary mode; default is to report miss rate only.
	-s size	  Size of cache in instructions.
	-c instr  Number of instructions to run before resetting
		miss history (this eliminates cold start behavior).
	-l instr  Number of instructions to simulate (after cold start).
	-t misses Number of misses on a cache line before line can be
		  replaced.  misses=0 is equivalent to no exclusion.
 */

#include <stdio.h>

#define FALSE 0
#define TRUE 1

#define LARGEINT 0x7fffffff

#define USAGE \
"usage: idmexcache [-v] [-s size] [-c instr] [-l instr] [-t misses] [file]\n"

#define INSTR(x)	(((x)&0x80000000)==0)
#define ADDR(x)		((x) &0x3fffffff)

unsigned int setmask;
unsigned int *tagram;			/* dynamically allocated tag RAM */
unsigned int *valid;			/* valid bits */
unsigned int *misscounts;		/* number of misses on cache entry */

#define INDEX(x)\
	((x) & setmask)
#define MISS(x)\
	((tagram[(x) & setmask] != ((x) & ~setmask)) ||\
	 (!valid[(x) & setmask]))
#define CACHE(x)\
	tagram[(x) & setmask] = (x) & ~setmask;\
	valid[(x) & setmask] = TRUE;

unsigned int size = 256;		/* cache size (in instructions) */
unsigned int accesses = 0;
unsigned int misses = 0;

void Cacheinit();
void Simulate();
void Summarize();

main(argc,argv)
int argc;
char *argv[];
{
	int c;
	extern int optind;		/* for getopt() */
	extern char *optarg;
	unsigned int coldlength = 0;	/* cold start length */
	unsigned int length = 512;	/* trace length */
	unsigned int missthreshold = 0;	/* no exclusion by default */
	int verbose = FALSE;		/* verbosity of summary report */
	FILE *fp = stdin;		/* standard input by default */

	while((c=getopt(argc,argv,"c:l:s:t:v")) != EOF){
		switch(c){
		   case 'c': coldlength = atoi(optarg);
			     break;
		   case 'l': length = atoi(optarg);
			     break;
		   case 's': size = atoi(optarg);
			     break;
		   case 't': missthreshold = atoi(optarg);
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
	Cacheinit(size);
	Simulate(fp,coldlength,0);
	accesses = misses = 0;
	Simulate(fp,length,missthreshold);
	Summarize(verbose);
	return(0);
}

void
Cacheinit(size)
unsigned int size;
{
	unsigned int i;

	tagram = (unsigned int *)malloc(size * sizeof(unsigned int));
	if(tagram == NULL){
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	valid  = (unsigned int *)malloc(size * sizeof(unsigned int));
	if(valid == NULL){
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	for(i=0; i<size; i++) valid[i] = FALSE;
	misscounts = (unsigned int *)malloc(size * sizeof(unsigned int));
	if(misscounts == NULL){
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	for(i=0; i<size; i++) misscounts[i] = LARGEINT;
	setmask = size-1;
}

void
Simulate(fp,instructions,missthreshold)
FILE *fp;
unsigned int instructions;
unsigned int missthreshold;
{
	unsigned int trace, x;

	while(!feof(fp) && instructions){
		trace = getw(fp);
		if(INSTR(trace)){
			x = ADDR(trace);
#ifdef DEBUG
			printf("%x", x);
#endif
			if(MISS(x)){
#ifdef DEBUG
				printf(" miss");
#endif
				if(misscounts[INDEX(x)] >= missthreshold){
					CACHE(x);
					misscounts[INDEX(x)] = 0;
#ifdef DEBUG
					printf(" replaced");
#endif
				} else {
					misscounts[INDEX(x)]++;
				}
				misses++;
			}
			accesses++;
			instructions--;
#ifdef DEBUG
			putchar('\n');
#endif
		}
	}
}

void
Summarize(verbose)
int verbose;
{
	if(verbose){
		printf("cache size = %d instructions\n",size);
		printf("%d misses and %d accesses\n", misses, accesses);
		printf("miss rate = %f\n",(float)misses/(float)accesses);
	} else {
		printf("%f\n",(float)misses/(float)accesses);
	}
}
