#define NFIELDS		18		/* number of fields in all 
					   variants of opcode */

#define S_TYPE		3
#define S_DEST		29
#define S_RETURN	1
#define S_REG1		4
#define S_REG2		4
#define S_PSTACK	2
#define S_RSTACK	2

#define S_OFFSET	16

#define S_ALUMODE	1
#define S_BSRC		1
#define S_ALUCOND	4
#define S_CIN		2
#define S_FLAG		1

#define S_ALUOP		7

#define S_SHIFT		2
#define S_SIN		1
#define S_STEP		2
#define S_FLAGIN	1

typedef struct {
#ifdef RIGHTBITS	/* some compilers allocate bits from right to left */
	unsigned unused:1;
	unsigned flagin: S_FLAGIN;		/* flag load source */
	unsigned step: S_STEP;			/* multiply or divide step */
	unsigned sin: S_SIN;			/* shift source */
	unsigned shift: S_SHIFT;		/* shift */
	unsigned flag: S_FLAG;			/* load flag */
	unsigned cin: S_CIN;			/* carry in */
	unsigned alucond: S_ALUCOND;		/* ALU condition */
	unsigned bsrc: S_BSRC;			/* FL or DL */
	unsigned alumode: S_ALUMODE;		/* alu or shift */
	unsigned rstack: S_RSTACK;		/* return stack */
	unsigned pstack: S_PSTACK;		/* parameter stack */
	unsigned reg2: S_REG2;			/* register 2 */
	unsigned reg1: S_REG1;			/* register 1 */
	unsigned ret: S_RETURN;			/* return from call */
	unsigned type: S_TYPE;			/* instruction type? */
#else			/* and others allocate bits from left to right */
	unsigned type: S_TYPE;			/* instruction type? */
	unsigned ret: S_RETURN;			/* return from call */
	unsigned reg1: S_REG1;			/* register 1 */
	unsigned reg2: S_REG2;			/* register 2 */
	unsigned pstack: S_PSTACK;		/* parameter stack */
	unsigned rstack: S_RSTACK;		/* return stack */
	unsigned alumode: S_ALUMODE;		/* alu or shift */
	unsigned bsrc: S_BSRC;			/* FL or DL */
	unsigned alucond: S_ALUCOND;		/* ALU condition */
	unsigned cin: S_CIN;			/* carry in */
	unsigned flag: S_FLAG;			/* load flag */
	unsigned shift: S_SHIFT;		/* shift */
	unsigned sin: S_SIN;			/* shift source */
	unsigned step: S_STEP;			/* multiply or divide step */
	unsigned flagin: S_FLAGIN;		/* flag load source */
	unsigned unused:1;
#endif
} shiftmicro;

typedef struct {
#ifdef RIGHTBITS	/* some compilers allocate bits from right to left */
	unsigned aluop:S_ALUOP;			/* ALU */
	unsigned flag: S_FLAG;			/* flag */
	unsigned cin: S_CIN;			/* carry in */
	unsigned alucond: S_ALUCOND;		/* ALU condition */
	unsigned bsrc: S_BSRC;			/* FL or DL */
	unsigned alumode: S_ALUMODE;		/* alu or shift */
	unsigned rstack: S_RSTACK;		/* return stack */
	unsigned pstack: S_PSTACK;		/* parameter stack */
	unsigned reg2: S_REG2;			/* register 2 */
	unsigned reg1: S_REG1;			/* register 1 */
	unsigned ret: S_RETURN;			/* return from call */
	unsigned type: S_TYPE;			/* instruction type? */
#else			/* and others allocate bits from left to right */
	unsigned type: S_TYPE;			/* instruction type? */
	unsigned ret: S_RETURN;			/* return from call */
	unsigned reg1: S_REG1;			/* register 1 */
	unsigned reg2: S_REG2;			/* register 2 */
	unsigned pstack: S_PSTACK;		/* parameter stack */
	unsigned rstack: S_RSTACK;		/* return stack */
	unsigned alumode: S_ALUMODE;		/* alu or shift */
	unsigned bsrc: S_BSRC;			/* FL or DL */
	unsigned alucond: S_ALUCOND;		/* ALU condition */
	unsigned cin: S_CIN;			/* carry in */
	unsigned flag: S_FLAG;			/* flag */
	unsigned aluop:S_ALUOP;			/* ALU */
#endif
} alumicro;

typedef struct {
#ifdef RIGHTBITS	/* some compilers allocate bits from right to left */
	unsigned offset: S_OFFSET;		/* offset */
	unsigned rstack: S_RSTACK;		/* return stack */
	unsigned pstack: S_PSTACK;		/* parameter stack */
	unsigned reg2: S_REG2;			/* register 2 */
	unsigned reg1: S_REG1;			/* register 1 */
	unsigned ret: S_RETURN;			/* return from call */
	unsigned type: S_TYPE;			/* instruction type? */
#else			/* and others allocate bits from left to right */
	unsigned type: S_TYPE;			/* instruction type? */
	unsigned ret: S_RETURN;			/* return from call */
	unsigned reg1: S_REG1;			/* register 1 */
	unsigned reg2: S_REG2;			/* register 2 */
	unsigned pstack: S_PSTACK;		/* parameter stack */
	unsigned rstack: S_RSTACK;		/* return stack */
	unsigned offset: S_OFFSET;		/* offset */
#endif
} loadstore;

typedef struct {
#ifdef RIGHTBITS	/* some compilers allocate bits from right to left */
	unsigned dest: S_DEST;			/* target address */
	unsigned type: S_TYPE;			/* branch type */
#else			/* and others allocate bits from left to right */
	unsigned type: S_TYPE;			/* branch type */
	unsigned dest: S_DEST;			/* target address */
#endif
} branch;

union opstruct {				/* instruction register */
	int32 irvalue;				/* treat as value */
	alumicro iralu;				/* treat as instruction */
	shiftmicro irshift;			/* treat as instruction */
	loadstore irload;			/* treat as load/store */
	branch irbranch;			/* treat as branch */
};
