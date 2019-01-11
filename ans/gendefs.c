/* (c) 1992 Johns Hopkins University / Applied Physics Laboratory */
/* This program measures some aspects of the host system for use
 by the forth kernel */

#include <stdio.h>

main()
{
	printf("/* This file was generated automatically - do not edit.\n");
	printf("   See gendefs.c */\n");
	if(sizeof(unsigned int) == 4){	/* assumes 4 bytes == 32 bits */
		if(sizeof(void *) != sizeof(unsigned int)){
			fprintf(stderr,
				"Ints and pointers must have same size!\n");
			exit(1);
		}
		printf("/*Needed for C version:*/\n");
		printf("#define CELLTYPE  unsigned int\n");
		printf("#define SCELLTYPE          int\n");
		printf("#define MSB  0x%x\n", ~((unsigned int)(~0)>>1));
		printf("#define MSB2 0x%x\n", ~((unsigned int)(~0)>>2));
		if(!((-1>>1) < 0)) printf("#define NO_ASR\n");
		printf("/*Needed for assembly version:*/\n");
		printf("#define IOBUFSIZE %d\n", sizeof(FILE));
	} else {
		fprintf(stderr, "Ints must be 32 bits!\n");
		exit(1);
	}
	return(0);
}
