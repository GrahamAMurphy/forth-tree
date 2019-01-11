#include <stdio.h>

#define USAGE "usage: instr [-s start] [-l n] [file]\n"

#define INSTR(x)	(((x)&0x80000000)==0)
#define ADDR(x)		((x) &0x3fffffff)

main(argc,argv)
int argc;
char *argv[];
{
	int c;
	extern int optind;		/* for getopt() */
	extern char *optarg;
	unsigned int start = 0;		/* number to skip */
	unsigned int length = 512;	/* default trace length */
	FILE *fp = stdin;		/* standard input by default */
	unsigned int trace;

	while((c=getopt(argc,argv,"l:s:")) != EOF){
		switch(c){
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
		if(INSTR(trace)){
			start--;
		}
	}
	while(!feof(fp) && length){
		trace = getw(fp);
		if(INSTR(trace)){
			printf("%d\n", ADDR(trace));
			length--;
		}
	}
	return(0);
}
