#include <stdio.h>
#include <limits.h>

#define USAGE "usage: instrcount [-r] [-s start] [-l n] [file]\n"

#define INSTR(x)	(((x)&0x80000000)==0)
#define DLOAD(x)	(((x)&0x40000000)==0)

#define FALSE 0
#define TRUE 1

main(argc,argv)
int argc;
char *argv[];
{
	int c;
	extern int optind;		/* for getopt() */
	extern char *optarg;
	int raw = FALSE;
	unsigned int start = 0;		/* number to skip */
	unsigned int length = UINT_MAX;	/* default trace length */
	FILE *fp = stdin;		/* standard input by default */
	unsigned int trace;
	unsigned int ifetch, dfetch, dstore;

	while((c=getopt(argc,argv,"rl:s:")) != EOF){
		switch(c){
		   case 'r': raw = TRUE;
			     break;
		   case 'l': length = atoi(optarg);
			     break;
		   case 's': start = atoi(optarg);
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

	while(!feof(fp) && start){
		trace = getw(fp);
		start--;
	}
	ifetch = dfetch = dstore = 0;
	while(!feof(fp) && length){
		trace = getw(fp);
		if(INSTR(trace)){
			ifetch++;
		} else if(DLOAD(trace)){
			dfetch++;
		} else {
			dstore++;
		}
		length--;
	}
	if(raw){
		printf("%d %d %d\n", ifetch, dfetch, dstore);
	} else {
		unsigned int accesses = ifetch + dfetch + dstore;
		printf("%4.1f%% %4.1f%% %4.1f%%\n",
			(float)(100*ifetch)/(float)accesses,
			(float)(100*dfetch)/(float)accesses,
			(float)(100*dstore)/(float)accesses);
	}
	return(0);
}
