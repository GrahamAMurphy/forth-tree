/* (c) 1992 Johns Hopkins University / Applied Physics Laboratory */
/* This file defines the interface to the Forth interpreter */

void init_forth(void);
void do_forth(int argc, char *argv[]);
void eval_forth(char *s);
