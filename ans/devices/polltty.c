/*
 * Forth I/O routines: emit and key
 */


/* TBD: maybe this should be unsigned int */
int
cmd_getc(void) {
	unsigned int c;
	while((*STATUS&RECV_MASK) == 0) ;	/* wait for data */
	c = *RECV;
	return(c=='\r' ? '\n' : c);	/* map '\r' to '\n' */
}

void
cmd_putc(unsigned char c) {
	if(c=='\n') {			/* map '\n' to '\r'/'\n' */
		while((*STATUS&XMIT_MASK) == 0) ;
		*XMIT = '\r';
	}
	while((*STATUS&XMIT_MASK) == 0) ;
	*XMIT = c;
}
