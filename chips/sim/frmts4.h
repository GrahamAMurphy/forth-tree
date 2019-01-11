#define S_TYPE		3
#define S_DEST		29
#define S_RETURN	1
#define S_REG1		4
#define S_REG2		4
#define S_PSTACK	2
#define S_RSTACK	2

#define S_OFFSET	16

#define S_CLASS		2
#define S_ALUCOND	4
#define S_FLAG		1
#define S_CIN		2

#define S_ALUOP		7

#define S_STEP		2
#define S_LEFT		1
#define S_RIGHT		1
#define S_RSRC		1
#define S_FSRC		2
#define S_YCONT		2

#define S_MODE		2
#define S_PRI		1
#define S_IMM		1
#define S_LITERAL	5

typedef struct {
#ifdef RIGHTBITS	/* some compilers allocate bits from right to left */
	unsigned dest: S_DEST;			/* target address */
	unsigned type: S_TYPE;			/* branch type */
#else			/* and others allocate bits from left to right */
	unsigned type: S_TYPE;			/* branch type */
	unsigned dest: S_DEST;			/* target address */
#endif
} branch;

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
	unsigned aluop:S_ALUOP;			/* ALU */
	unsigned cin: S_CIN;			/* carry in */
	unsigned flag: S_FLAG;			/* flag */
	unsigned alucond: S_ALUCOND;		/* ALU condition */
	unsigned class: S_CLASS;		/* computation class */
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
	unsigned class: S_CLASS;		/* computation class */
	unsigned alucond: S_ALUCOND;		/* ALU condition */
	unsigned flag: S_FLAG;			/* flag */
	unsigned cin: S_CIN;			/* carry in */
	unsigned aluop:S_ALUOP;			/* ALU */
#endif
} alumicro;

typedef struct {
#ifdef RIGHTBITS	/* some compilers allocate bits from right to left */
	unsigned ycont: S_YCONT;		/* Y control */
	unsigned fsrc: S_FSRC;			/* flag source */
	unsigned rsrc: S_RSRC;			/* right shift input */
	unsigned right: S_RIGHT;		/* right shift */
	unsigned left: S_LEFT;			/* left shift */
	unsigned step: S_STEP;			/* multiply or divide step */
	unsigned flag: S_FLAG;			/* load flag */
	unsigned alucond: S_ALUCOND;		/* ALU condition */
	unsigned class: S_CLASS;		/* computation class */
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
	unsigned class: S_CLASS;		/* computation class */
	unsigned alucond: S_ALUCOND;		/* ALU condition */
	unsigned flag: S_FLAG;			/* load flag */
	unsigned step: S_STEP;			/* multiply or divide step */
	unsigned left: S_LEFT;			/* left shift */
	unsigned right: S_RIGHT;		/* right shift */
	unsigned rsrc: S_RSRC;			/* right shift input */
	unsigned fsrc: S_FSRC;			/* flag source */
	unsigned ycont: S_YCONT;		/* Y control */
#endif
} stepmicro;

typedef struct {
#ifdef RIGHTBITS	/* some compilers allocate bits from right to left */
	unsigned literal: S_LITERAL;		/* the operand */
	unsigned imm: S_IMM;			/* immediate operand */
	unsigned pri: S_PRI;			/* priority encoder */
	unsigned mode: S_MODE;			/* shift mode */
	unsigned flag: S_FLAG;			/* load flag */
	unsigned alucond: S_ALUCOND;		/* ALU condition */
	unsigned class: S_CLASS;		/* computation class */
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
	unsigned class: S_CLASS;		/* computation class */
	unsigned alucond: S_ALUCOND;		/* ALU condition */
	unsigned flag: S_FLAG;			/* load flag */
	unsigned mode: S_MODE;			/* shift mode */
	unsigned pri: S_PRI;			/* priority encoder */
	unsigned imm: S_IMM;			/* immediate operand */
	unsigned literal: S_LITERAL;		/* the operand */
#endif
} shiftmicro;

union opstruct {				/* instruction register */
	forthword irvalue;			/* treat as value */
	branch irbranch;			/* treat as branch */
	loadstore irload;			/* treat as load/store */
	alumicro iralu;				/* treat as instruction */
	stepmicro irstep;			/* treat as instruction */
	shiftmicro irshift;			/* treat as instruction */
};
