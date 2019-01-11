/*
 *		Forth Chip Instruction Encoding
 *
 *		Bit patterns to encode all values of all fields
TBD: all encodings of new fields are open to negotiation.
 */

#define B_DEFAULT	-1		/* out of range value to
					   represent default value
					   expanded opcode */

/*			Instruction type				 */

#define B_COLON		0		/* colon call */
#define B_BRANCH	1		/* branch */
#define B_QBRANCH	2		/* ?branch */
#define B_MICRO		3		/* micro */
#define B_LOAD		4		/* load */
#define B_STORE		5		/* store */
#define B_LAL		6		/* load address low */
#define B_LAH		7		/* load address high */

/*			Return						*/

#define B_NORETURN	0			/* don't return from call */
#define B_RETURN	1			/* return from call */

/*			Reg1 - B bus source during phi 1		*/

#define B_S0R1		0
#define B_S1R1		1
#define B_S2R1		2
#define B_S3R1		3
#define B_R0R1		4
#define B_R1R1		5
#define B_R2R1		6
#define B_R3R1		7
#define B_U0R1		8
#define B_U1R1		9
#define B_U2R1		10
#define B_U3R1		11
#define B_IARR1		12
#define B_PSWR1		13
#define B_ZEROR1	14
#define B_YR1		15

/*			Reg2 - B bus destination			*/

#define B_S0R2		0
#define B_S1R2		1
#define B_S2R2		2
#define B_S3R2		3
#define B_R0R2		4
#define B_R1R2		5
#define B_R2R2		6
#define B_R3R2		7
#define B_U0R2		8
#define B_U1R2		9
#define B_U2R2		10
#define B_U3R2		11
#define B_IARR2		12
#define B_PSWR2		13
#define B_ZEROR2	14
#define B_YR2		15

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

#define B_CLEAR		0x0		/*  0 - clear */
#define B_SET		0x1		/*  1 - set */

#define B_OVFLOW	0x2		/*  V - overflow */
#define B_VBAR		0x3		/* ~V - not overflow */

#define B_GREATER	0x4		/* ~(N^V | Z) - greater */
#define B_LESSEQ	0x5		/*  (N^V | Z) - less or equal */

#define B_NEG		0x6		/*  N - negative */
#define B_NBAR		0x7		/* ~N - not negative */

#define B_EQUAL		0x8		/*  Z - equal to zero */
#define B_NEQUAL	0x9		/* ~Z - not equal to zero */

#define B_UGREATER	0xa		/* ~(~C | Z) - unsigned greater */
#define B_ULESSEQ	0xb		/*  (~C | Z) - unsigned less or equal */

#define B_LESS		0xc		/*  (N^V) - less */
#define B_GREATEREQ	0xd		/* ~(N^V) - greater or equal */

#define B_COUT		0xe		/*  C - unsigned greater or equal */
#define B_CBAR		0xf		/* ~C - unsigned less */

/*			Flag load					*/

#define B_NOFLAG	0
#define B_FLAG		1		/* load the flag */

/*			Carry in					*/

/* TBD: is there any optimization that can be done here? */
#define B_ZERO		0		/* carry in is zero */
#define B_ONE		1		/*  one */
#define B_CFLAG		2		/*  FL' */
#define B_CFLAGBAR	3		/*  ~FL' */

/*			ALU						*/

#define B_ONES		0x30		/* -1 */
#define B_ZEROS		0x70		/* 0 */
#define B_NOPA		0x74		/* S0 */
#define B_NOPB		0x71		/* Src */
#define B_NOTA		0x34		/* ~Src */
#define B_NOTB		0x31		/* ~Src */
#define B_AANDB		0x3f		/* S0 & Src */
#define B_ABARANDB	0x37		/* ~S0 & Src */
#define B_AANDBBAR	0x3d		/* S0 & ~Src */
#define B_ANANDB	0x7f		/* ~(S0 & Src) */
#define B_AORB		0x75		/* S0 | Src */
#define B_ABARORB	0x7d		/* ~S0 | Src */
#define B_AORBBAR	0x77		/* S0 | ~Src */
#define B_ANORB		0x35		/* ~(S0 | Src) */
#define B_AXORB		0x65		/* S0 xor Src */
#define B_ANXORB	0x25		/* ~(S0 xor Src) */

#define B_APLUSB	0x45		/* S0 + Src + Cin */
#define B_AMINUSB	0x47		/* S0 - Src - ~Cin */
#define B_BMINUSA	0x4d		/* Src - S0 - ~Cin */
#define B_NEGA		0x4c		/* ~S0 + Cin */
#define B_NEGB		0x43		/* ~Src + Cin */
#define B_INCA		0x44		/* S0 + Cin */
#define B_INCB		0x41		/* Src + Cin */
#define B_DECA		0x0c		/* S0 - Cin */
#define B_DECB		0x03		/* Src - Cin */
#define B_ABARPLUSBBAR	0x4f		/* ~S0 + ~Src + Cin */

/*			Step operation					*/

#define B_STEPB		0		/* Src */
#define B_CADD		1		/* if(~FL') Src + S0 else Src */
#define B_DIV		2		/* if(~FL') Src + S0 else Src - S0 */
#define B_MUL		3		/* Booth (two lsbs of Y and FL' */

/*			Left shift operation				*/

#define B_NOLEFTSHIFT	0
#define B_LEFT		1		/* Shift Src left */

/*			Right shift operation				*/

#define B_NORIGHTSHIFT	0
#define B_RIGHT		1		/* Shift ALU result right */

/*			Right shift source				*/

#define B_SALUCOND	0		/* ALU condition */
#define B_SFLAG		1		/* FL' */

/*			Source of flag input				*/

#define B_FLALUCOND	0		/* ALU condition */
#define B_FLY1		1		/* bit 1 of Y register */
#define B_FLDIVHELP	2		/* help for non-restoring divide */
#define B_FLRIGHTSHIFT	3		/* right shift output */

/*			Y Shift register				*/
#define B_YNOP		0
#define B_YLEFT		1		/* shift Y left one bit */
#define B_YRIGHT	2		/* shift Y right two bits */

/*			Shift Mode					*/

#define B_LSR		0		/* logical shift right */
#define B_ASR		1		/* arithmetic shift right */
#define B_LSL		2		/* logical shift left */
#define B_ROL		3		/* rotate left */

/*			Leading Zero Counter				*/

#define B_NOPRI		0
#define B_PRI		1		/* count leading zeros of Src */

/*			Immediate Operand				*/

#define B_NOIMM		0
#define B_IMM		1		/* shift extent in instruction */
