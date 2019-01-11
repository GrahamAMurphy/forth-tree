/*
 *		Forth Chip Instruction Encoding
 *
 *		Bit patterns to encode all values of all fields
 */

#define B_DEFAULT	-1			/* out of range value to
						   represent default value
						   expanded opcode */

/*			Instruction type				 */

#define B_COLON		0			/* colon call */
#define B_BRANCH	1			/* branch */
#define B_QBRANCH	2			/* ?branch */
#define B_MICRO		3			/* micro */
#define B_LOAD		4			/* load */
#define B_STORE		5			/* store */
#define B_LAL		6			/* load address low */
#define B_LAH		7			/* load address high */

/*			Return						*/

#define B_NORETURN	0			/* don't return from call */
#define B_RETURN	1			/* return from call */

/*			Reg1 - B bus source during phi 1		*/

#define B_TOSR1		0
#define B_SOSR1		1
#define B_ROSR1		2
#define B_FOSR1		3
#define B_TORR1		4
#define B_SORR1		5
#define B_RORR1		6
#define B_FORR1		7
#define B_UDR0R1	8
#define B_UDR1R1	9
#define B_UDR2R1	10
#define B_UDR3R1	11
#define B_IARR1		12
#define B_PSWR1		13
#define B_ZEROR1	14
#define B_NONER1	15

/*			Reg2 - B bus destination			*/

#define B_TOSR2		0
#define B_SOSR2		1
#define B_ROSR2		2
#define B_FOSR2		3
#define B_TORR2		4
#define B_SORR2		5
#define B_RORR2		6
#define B_FORR2		7
#define B_UDR0R2	8
#define B_UDR1R2	9
#define B_UDR2R2	10
#define B_UDR3R2	11
#define B_IARR2		12
#define B_PSWR2		13
#define B_ZEROR2	14
#define B_NONER2	15

/*			Stack operation					*/

#define B_NOPPS		0
#define B_POPPS		1
#define B_PUSHPS	2

#define B_NOPRS		0
#define B_POPRS		1
#define B_PUSHRS	2

/*			ALU mode					*/

#define B_ALU		0			/* ALU operation */
#define B_SHIFT		1			/* shift, multiply, or divide*/

/*			Bsrc - FL or DL to bbus?			*/

#define B_FL		0			/* FL -> bbus */
#define B_DL		1			/* DL -> bbus */

/*			ALU condition select				*/

#define B_CLEAR		0x0
#define B_SET		0x1

#define B_OVFLOW	0x2
#define B_VBAR		0x3

#define B_GREATER	0x4
#define B_LESSEQ	0x5

#define B_NEG		0x6
#define B_NBAR		0x7

#define B_EQUAL		0x8
#define B_NEQUAL	0x9

#define B_UGREATER	0xa
#define B_ULESSEQ	0xb

#define B_LESS		0xc
#define B_GREATEREQ	0xd

#define B_COUT		0xe
#define B_CBAR		0xf

/*			Carry in					*/

#define B_ZERO		0
#define B_ONE		1
#define B_CFLAG		2
#define B_CFLAGBAR	3

/*			Flag load					*/

#define B_NOFLAG	0
#define B_FLAG		1

/*			ALU						*/

#define B_ONES		0x22
#define B_ZEROS		0x20
#define B_NOPB		0x2c
#define B_NOPA		0x23
#define B_NOTB		0x24
#define B_NOTA		0x21
#define B_AANDB		0x55
#define B_AORB		0x1f
#define B_AXORB		0x2f
#define B_ANANDB	0x15
#define B_ANORB		0x5f
#define B_ANXORB	0x6f
#define B_AANDBBAR	0x5d
#define B_ABARANDB	0x57

#define B_BMINUSA	0x4d
#define B_AMINUSB	0x47
#define B_APLUSB	0x4f
#define B_INCB		0x4c
#define B_INCA		0x43
#define B_NEGB		0x44
#define B_NEGA		0x41
#define B_DECB		0x4e
#define B_DECA		0x4b

/*			Shift operation					*/

#define B_NOSHIFT	0
#define B_RIGHT		1
#define B_LEFT		2

/*			Right shift souce				*/

#define B_SALUCOND	0
#define B_SFLAG		1

/*			Step operation					*/

#define B_STEPB		0
#define B_DIV1		1
#define B_DIV2		2
#define B_MUL		3

/*			Source of flag input				*/

#define B_FLALU		0
#define B_FLSHIFT	1
