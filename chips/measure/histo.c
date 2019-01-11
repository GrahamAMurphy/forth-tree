#include <stdio.h>

#define USAGE "usage: instr [-s start] [-l length] [-r range] [file]\n"

#define INSTR(x)	(((x)&0x80000000)==0)
#define ADDR(x)		((x) &0x3fffffff)

unsigned int *histogram;		/* dynamically allocated array */
unsigned int range = 65536;		/* address range: 0..range */

int Maximum();

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
	unsigned int i;

	while((c=getopt(argc,argv,"l:s:")) != EOF){
		switch(c){
		   case 'l': length = atoi(optarg);
			     break;
		   case 'r': range = atoi(optarg);
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

	histogram = (unsigned int *)malloc(range * sizeof(unsigned int));
	for(i=0; i<range; i++) histogram[i] = 0;
	while(!feof(fp) && start){
		trace = getw(fp);
		if(INSTR(trace)){
			start--;
		}
	}
	while(!feof(fp) && length){
		trace = getw(fp);
		if(INSTR(trace)){
			histogram[ADDR(trace)]++;
			length--;
		}
	}
	printf("%d\n",Maximum());
	return(0);
}

int
Maximum()
{
	unsigned int i;
	unsigned int maxsofar = 0;

	for(i=0; i<range; i++){
		if(histogram[i] > maxsofar) maxsofar = histogram[i];
	}
	return(maxsofar);
}
