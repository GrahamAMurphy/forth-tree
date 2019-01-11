#include <stdio.h>

main()
{
	char line[120];
	char block[1024];
	int i;

	/* first block is not used */
	for(i=0; i<1024; i++) block[i] = ' ';
	fwrite(block, sizeof(char), 1024, stdout);
	while(gets(line) != NULL){
		strcpy(block,line);
		for(i=strlen(line); i<1024; i++) block[i] = ' ';
		fwrite(block, sizeof(char), 1024, stdout);
	}
	return(0);
}
