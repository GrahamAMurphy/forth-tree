/* (c) 1992 Johns Hopkins University / Applied Physics Laboratory */
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>

#ifdef sun
#include <inttypes.h>
#include <sys/termios.h>
#elif defined linux
#include <sys/types.h>
#endif
#include <memory.h>
#include <errno.h>
#include "filewrap.h"			/* defines this interface */

#define MAXSTRING 1024			/* maximum string length */

char *getenv();				/* forward reference */

unsigned int result2;			/* return extra stuff in global */

#include "AdjustWrite.c"

/* ------------------------------------------------------------------------
   convert a Forth character string into a C null-terminated string */
static char *
zstring(char *s, int l) {
	static char sbuf[MAXSTRING];

	(void) memcpy(sbuf,s,l);
	sbuf[l] = '\0';
	return(sbuf);
}

/* ------------------------------------------------------------------------
   close-file	( w -- ior )
	in assembly language */

/* ------------------------------------------------------------------------ 
   create-file ( c-addr u w1 -- w2 ior ) */

FILE *
create_file(char *name, int length, char *mode) {
	char *zname = zstring(name,length);
	int fd;

	if((fd=creat(zname,0666)) < 0 ) return(NULL);
	close(fd);
	return(fopen(zname,mode));
}

/* ------------------------------------------------------------------------ 
   delete-file	( c-addr u -- ior ) */

int
delete_file(char *name, int length) {
	return(unlink(zstring(name,length)));
}

/* ------------------------------------------------------------------------
   file-position ( fileid -- ud ior )
	in assembly language */

/* ------------------------------------------------------------------------
   file-size	( fileid -- ud ior ) */

unsigned int
file_size(FILE *fp) {
	struct stat sbuf;

	if(fstat(fileno(fp),&sbuf) < 0) return(-1);
	return(sbuf.st_size);
}

/* ------------------------------------------------------------------------
   file-status	( c-addr u -- w ior ) */

struct stat *
file_status(char *name, int length) {
	static struct stat sbuf;

	return(stat(zstring(name,length), &sbuf) < 0 ? NULL : &sbuf);
}

/* ------------------------------------------------------------------------
   open-file	( c-addr u w1 -- w2 ior ) */

FILE *
open_file(char *name, int length, char *mode) {
	return(fopen(zstring(name,length), mode));
}

/* ------------------------------------------------------------------------
   read-file	( c-addr u1 fileid -- u2 ior )
	in assembly language */

/* ------------------------------------------------------------------------
   read-line	( c-addr u1 fileid -- u2 flag ior ) */

int
read_line(char *buf, int len, FILE *fp) {
	int actlen = 0;
	int c;

	while((actlen < len) &&
	      ((c = getc(fp)) != EOF)){
		*buf++ = c;
		actlen++;
	      if(c == '\n') break;
	}
#ifdef AUX
    errno = 0 ; /* Under A/UX 3.0.1, getc always returns ENOTTY */
#endif
	return(actlen);
}

/* ------------------------------------------------------------------------
   reposition-file ( ud fileid -- ior )
	in assembly language */

/* ------------------------------------------------------------------------
   resize-file	( ud fileid -- ior ) */

int
resize_file(off_t size, FILE *fp) {
	return(ftruncate(fileno(fp), size)); /* TBD does this always work? */
}

/* ------------------------------------------------------------------------
   write-file	( c-addr u fileid -- ior )
		in assembly language */

/* ------------------------------------------------------------------------
   write-line	( c-addr u fileid -- ior ) */

void
write_line(char *buf, int length, FILE *fp) {
	fwrite(buf, 1, length, fp);
	putc('\n', fp);
}

/* ------------------------------------------------------------------------
   file-key	( fileid -- c )
 File-key is for use by interactive programs that read single characters
 from a terminal.  The nominal behavior of the Unix device driver is
 wait until an entire line is typed before returning a single character.
 To get around this, the driver can be temporarily reconfigured to
 return after a single character is typed.  Echoing is also disabled
 as is processing of special keys like ^C.  Once the character is received,
 the driver is set back to its initial configuration.  If the given
 file is not a terminal, reasonable things happen: the next character
 available is returned.

 If this cannot be done (i.e. no appropriate ifdefs for this machine,
 just return the next character.
 */

int
fkey(FILE *fp) {
#ifdef sun
	struct termios ttyorig;
	struct termios ttymod;
	int c;

	if(isatty(fileno(fp))){
		ioctl(fileno(fp), TCGETS, &ttyorig);
		ttymod = ttyorig;
		ttymod.c_lflag &= ~(ISIG | ICANON | ECHO);
		ttymod.c_cc[VMIN] = 1;
		ttymod.c_cc[VTIME] = 0;
		ioctl(fileno(fp), TCSETS, &ttymod);
	}

	c = getc(fp);

	if(isatty(fileno(fp))){
		ioctl(fileno(fp), TCSETS, &ttyorig);
	}

	return(c);
#else
	return(getc(fp));
#endif
}

/* ------------------------------------------------------------------------ 
   chdir ( c-addr u -- ) */

void
change_dir(char *name, int length) {
	(void)chdir(zstring(name, length));
}

/* ------------------------------------------------------------------------
   get-env	( c-addr1 u1 -- c-addr2 u2 ) */

char *
get_env(char *name, int length) {
	char *t = getenv(zstring(name, length));
	result2 = t ? strlen(t) : 0;
	return(t);
}

