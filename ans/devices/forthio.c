/*
 *	forth io routines: expect and type
 */

#define CNTLH	'\010'			/* ^H: delete char */
#define CNTLU	'\025'			/* ^U: delete line */

int
cmd_getline(char *addr, int len) {
	unsigned int i=0;
	unsigned char c;

	while(i<len-1){
		switch(c=cmd_getc()){
		   case CNTLH: if(i){
					cmd_putc(CNTLH);
					cmd_putc(' ');
					cmd_putc(CNTLH);
					i--;
				} break;
		   case CNTLU:	cmd_putc('\r'); cmd_putc('\n');
				i=0; break;
		   case '\n':	cmd_putc('\r'); cmd_putc('\n');
				*(addr+(i++))=c;
			     	return(i);
		   default:	cmd_putc(c);
				*(addr+(i++))=c; break;
		}
	}
	return(i);
}

void
cmd_putline(char *addr, int count) {
	while(count-- > 0){
		cmd_putc(*addr++);
	}
}

