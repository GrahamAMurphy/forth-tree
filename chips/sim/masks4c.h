/*
 *		Forth Chip Instruction Encoding
 *
 *		Bit patterns to encode all values of all fields
TBD: all encodings of new fields are open to negotiation.
 */

#define B_DEFAULT	-1		/* out of range value to
					   represent default value
					   expanded opcode */

/*************************************************************************/
/*		Internal Memory-mapped IO					 */
#define ROMPAGE	1			/* IBROM base address (ms nibble) */
#define IOPAGE	2			/* I/O base address (ms nibble) */

#define R_RSP	0			/* return stack overflow pointer */
#define R_PSP	1			/* parameter stack overflow pointer */
#define R_SCC	2			/* stack cache controller */
#define R_MASK	4			/* interrupt mask */
#define R_MODE	5			/* interrupt mode */
#define R_EDGE	6			/* interrupt edge detector */
#define R_LEVL	7			/* interrupt level */
#define R_ATTR	16			/* attribute RAM */
#define R_ATTRRANGE "1----"		/* range of attribute RAM */
#define NATTRIBBITS 4


/*************************************************************************/
/*			Instruction type				 */

#define B_FLOW		0		/* control flow */
#define B_MICRO		1		/* micro */
#define B_LAL		2		/* load address low */
#define B_LAH		3		/* load address high */
#define B_LOAD		4		/* load */
#define B_LOADBYTE	5		/* load byte */
#define B_STORE		6		/* store */
#define B_STOREBYTE	7		/* store byte */

/*			Control flow type				*/

#define B_COLON		0		/* colon call */
#define B_DOES		1		/* does call */
#define B_BRANCH	2		/* branch */
#define B_QBRANCH	3		/* ?branch */

/*			Return						*/

#define B_NORETURN	0			/* don't return from call */
#define B_RETURN	1			/* return from call */

/*			Reg1 - source , Reg2 - destination		*/

#define B_S0		0
#define B_S1		1
#define B_S2		2
#define B_S3		3
#define B_R0		4
#define B_R1		5
#define B_R2		6
#define B_R3		7
#define B_U0		8
#define B_U1		9
#define B_U2		10
#define B_U3		11
#define B_IAR		12
#define B_PSW		13
#define B_ZERO		14
#define B_Y		15

/*			Stack operation					*/

#define B_NOPPS		0		/* nop parameter stack */
#define B_POPPS		1		/* pop parameter stack */
#define B_PUSHPS	2		/* push parameter stack */

#define B_NOPRS		B_NOPPS		/* nop return stack */
#define B_POPRS		B_POPPS		/* pop return stack */
#define B_PUSHRS	B_PUSHPS	/* push return stack */

/*			Class of computation				*/

/* TBD: is there any optimization that can be done here? */
#define B_ALU		0		/* ALU operation */
#define B_TEST		1		/* ALU condition */
#define B_STEP		2		/* multiply/divide/shift */
#define B_SHIFT		3		/* shift */

/*			ALU condition select				*/
/* The following odd encoding was chosen by Finesse */

#define B_CLEAR		0xb		/*  0 - clear */
#define B_SET		0x0		/*  1 - set */

#define B_OVFLOW	0xa		/*  V - overflow */
#define B_VBAR		0xc		/* ~V - not overflow */

#define B_GREATER	0x7		/* ~(N^V | Z) - greater */
#define B_LESSEQ	0x8		/*  (N^V | Z) - less or equal */

#define B_NEG		0x4		/*  N - negative */
#define B_NBAR		0xe		/* ~N - not negative */

#define B_EQUAL		0x3		/*  Z - equal to zero */
#define B_NEQUAL	0x1		/* ~Z - not equal to zero */

#define B_UGREATER	0xf		/* ~(~C | Z) - unsigned greater */
#define B_ULESSEQ	0x2		/*  (~C | Z) - unsigned less or equal */

#define B_LESS		0x9		/*  (N^V) - less */
#define B_GREATEREQ	0x6		/* ~(N^V) - greater or equal */

#define B_COUT		0x5		/*  C - unsigned greater or equal */
#define B_CBAR		0xd		/* ~C - unsigned less */

/*			Flag load					*/

#define B_NOFLAG	0
#define B_FLAG		1		/* load the flag */

/*			Carry in					*/

/* TBD: is there any optimization that can be done here? */
#define B_CZERO		0		/* carry in is zero */
#define B_CONE		1		/*  one */
#define B_CFLAG		2		/*  FL' */
#define B_CFLAGBAR	3		/*  ~FL' */

/*			ALU						*/

#define B_ONES		0x03		/* -1 */
#define B_ZEROS		0x0c		/* 0 */
#define B_NOPA		0x0a		/* S0 */
#define B_NOPB		0x0f		/* Src */
#define B_NOTA		0x05		/* ~Src */
#define B_NOTB		0x00		/* ~Src */
#define B_AANDB		0x0e		/* S0 & Src */
#define B_ABARANDB	0x0d		/* ~S0 & Src */
#define B_AANDBBAR	0x08		/* S0 & ~Src */
#define B_ANANDB	0x01		/* ~(S0 & Src) */
#define B_AORB		0x0b		/* S0 | Src */
#define B_ABARORB	0x07		/* ~S0 | Src */
#define B_AORBBAR	0x02		/* S0 | ~Src */
#define B_ANORB		0x04		/* ~(S0 | Src) */
#define B_AXORB		0x09		/* S0 xor Src */
#define B_ANXORB	0x06		/* ~(S0 xor Src) */

#define B_APLUSB	0x19		/* S0 + Src + Cin */
#define B_AMINUSB	0x39		/* S0 - Src - ~Cin */
#define B_BMINUSA	0x16		/* Src - S0 - ~Cin */
#define B_NEGB		0x3f		/* ~Src + Cin */
#define B_INCB		0x1f		/* Src + Cin */
#define B_DECB		0x10		/* Src - Cin */
#define B_BPLUSB	0x1c		/* Src + Src + Cin */

/*			Source of flag input				*/

#define B_FLALUCOND	0		/* ALU condition */
#define B_FLY1		1		/* bit 1 of Y register */
#define B_FLDIVHELP	2		/* help for non-restoring divide */

/*			Left shift operation				*/

#define B_NOLEFTSHIFT	0
#define B_LEFT		1		/* Shift Src left */

/*			Y Shift register				*/

#define B_YNOP		0
#define B_YLEFT		1		/* shift Y left one bit */
#define B_YRIGHT	2		/* shift Y right two bits */

/*			Step operation					*/

#define B_STEPB		0		/* Src */
#define B_CADD		1		/* if(~FL') Src + S0 else Src */
#define B_DIV		2		/* if(~FL') Src + S0 else Src - S0 */
#define B_MUL		3		/* Booth (two lsbs of Y and FL' */

/*			Shift Mode					*/

#define B_LSR		0		/* logical shift right */
#define B_ASR		1		/* arithmetic shift right */
#define B_LSL		2		/* logical shift left */
#define B_ROL		3		/* rotate left */
#define B_SRFLAG	4		/* Src>>1 FL' -> msb*/
#define B_SRFLAGBAR	5		/* Src>>1 not FL' -> msb*/
#define B_PRI		7		/* count leading zeros of Src */

/*			Immediate Operand				*/

#define B_NOIMM		0
#define B_IMM		1		/* shift extent in instruction */
