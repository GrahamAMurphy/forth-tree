/* For cross-compiling C model to MIPS
   Assumes cross-compiler uses 32-bit ints and produces arithmetic
   right shift.
*/

#define CELLTYPE  unsigned int
#define SCELLTYPE          int
#define MSB  0x80000000
#define MSB2 0xc0000000
/*#define NO_ASR*/
