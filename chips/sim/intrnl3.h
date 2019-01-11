/* This file has definitions of things internal to the SC32. */

#define PSTACKSIZE 16			/* size of parameter stack */
#define RSTACKSIZE 16			/* size of return stack */

union pswformat {			/* format of processor status word */
	int32 value;			/* for assigning to psw */
	struct {			/* for accessing individual fields */
#ifdef RIGHTBITS	/* some compilers allocate bits from right to left */
		unsigned rover:4,	/* rstack overflow marker */
			 rtop:4,	/* copy of rtop */
			 pover:4,	/* pstack overflow marker */
			 ptop:4,	/* copy of ptop */
			 cache:1,	/* cache enable */
			 ie:1,		/* interrupt enable */
			 :14;		/* unused */
#else			/* and others allocate from left to right */
		unsigned :14,		/* unused */
			 ie:1,		/* interrupt enable */
			 cache:1,	/* cache enable */
			 ptop:4,	/* copy of ptop */
			 pover:4,	/* pstack overflow marker */
			 rtop:4,	/* copy of rtop */
			 rover:4;	/* rstack overflow marker */
#endif
	}fields;
};

extern int32 pstack[];			/* parameter stack */
extern int ptop;
extern int pover, punder;

extern int32 rstack[];			/* return stack */
extern int rtop;
extern int rover, runder;

extern uint32 pca, pcb;			/* Program Counter latches */

extern int32 aluout;			/* ALU output latch */

extern int32 alua, alub;		/* ALU input registers */

extern union opstruct ir;		/* instruction register */

extern int flag;			/* accumulator flag */

extern int32 abus,			/* bus A */
		 bbus,			/* bus B */
		 tbus;			/* bus T */

extern union pswformat psw;		/* processor status word */

extern int32 udr0, udr1, udr2, udr3;/* user defined registers */
