/* (c) 1992 Johns Hopkins University / Applied Physics Laboratory */
#include <stdio.h>

main(argc,argv)
int argc;
char *argv[];
{
	char *Mangle();
	for(argc--, argv++; argc; argc--, argv++){
		printf("%s ",Mangle(*argv));
	}
	putchar('\n');
	return(0);
}

/*
 *	Mangle:	Convert a string consisting of arbitrary characters
 *		into one where the only characters are _ (underscore),
 *		A-Z (upper case letters), and a-z (lower case letters).
 */

char *
Mangle(name)
char *name;
{
	static char buf[80];
	char *d;
	char *s;
	for(s = name, d = buf; *s; s++){
		if(*s>='!' && *s<=':'){
			*d++ = '_'; *d++ = *s + 'A' - '!';
		} else if(*s>=';' && *s<='@'){
			*d++ = '_'; *d++ = *s + 'a' - ';';
		} else if(*s>='A' && *s<='Z'){
			*d++ = *s;
		} else if(*s>='[' && *s<='`'){
			*d++ = '_'; *d++ = *s + 'g' - '[';
		} else if(*s>='a' && *s<='z'){
			*d++ = *s;
		} else if(*s>='{' && *s<='~'){
			*d++ = '_'; *d++ = *s + 'm' - '{';
		} else {
			printf("bad string to mangle\n");
		}
	}
	*d = '\0';
	return(buf);
}
