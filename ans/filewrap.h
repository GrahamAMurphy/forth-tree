/* (c) 1998 Johns Hopkins University / Applied Physics Laboratory */
/* Defines interface to filewrap.c */

#include "AdjustWrite.h"

extern unsigned int result2;
FILE * create_file(char *name, int l, char *mode);
int delete_file(char *, int);
unsigned int file_size(FILE *);
struct stat * file_status(char *, int);
FILE * open_file(char *name, int l, char *mode);
int read_line(char *, int, FILE *);
int resize_file(off_t, FILE *);
void write_line(char *, int, FILE *);
int fkey(FILE *);
void change_dir(char *, int);
char * get_env(char *, int);
