/* (c) 1992 Johns Hopkins University / Applied Physics Laboratory */
/* This file defines the interpreter's TTY interface */

/*
Interpreter I/O comes through here.  The routines cmd_getc, cmd_getline,
cmd_putc, and cmd_putline are supplied separately so that a user can
provide his own versions if the Forth system is being linked into a
larger C program.  The routines must obey a few rules:

int cmd_getc() returns character; EOF on end of file
int cmd_getline(char *, int) accumulates characters into given buffer
	and returns actual number of characters.  The line terminatator
	is placed in the buffer and included in the count.  An end of
	file returns zero and a blank line returns 1.  BUG: If there is
	no line terminator on last line, what happens?
void cmd_putc(c) outputs one character
void cmd_putline(char *, int) outputs the given buffer of characters.
	No line terminator is appended.
void cmd_ioinit() initializes.
*/
int cmd_getc(void);
int cmd_getline(char *buf, int len);
void cmd_putc(unsigned char c);
void cmd_putline(char *buf, int len);
void cmd_ioinit(void);
