/* This file has definitions of things internal to the SC32. */

#define PSTACKSIZE 16			/* size of parameter stack */
#define RSTACKSIZE 16			/* size of return stack */

union pswformat {			/* format of processor status word */
	forthword value;		/* for assigning to psw */
	struct {			/* for accessing individual fields */
#ifdef RIGHTBITS	/* some compilers allocate bits from right to left */
		unsigned cpselect:4,	/* coprocessor select */
			 ie:1,		/* interrupt enable */
			 ibrenable:1,	/* internal boot ROM enable */
			 :26;		/* unused */
#else			/* and others allocate from left to right */
		unsigned :26,		/* unused */
			 ibrenable:1,	/* internal boot ROM enable */
			 ie:1,		/* interrupt enable */
			 cpselect:4;	/* coprocessor select */
#endif
	}fields;
};

union sswformat {			/* format of stack cache status word */
	forthword value;		/* for assigning to ssw */
	struct {			/* for accessing individual fields */
#ifdef RIGHTBITS	/* some compilers allocate bits from right to left */
		unsigned rover:4,	/* rstack overflow marker */
			 rtop:4,	/* copy of rtop */
			 pover:4,	/* pstack overflow marker */
			 ptop:4,	/* copy of ptop */
			 rcache:1,	/* return stack cache enable */
			 pcache:1,	/* parameter stack cache enable */
			 :14;		/* unused */
#else			/* and others allocate from left to right */
		unsigned :14,		/* unused */
			 pcache:1,	/* parameter stack cache enable */
			 rcache:1,	/* return stack cache enable */
			 ptop:4,	/* copy of ptop */
			 pover:4,	/* pstack overflow marker */
			 rtop:4,	/* copy of rtop */
			 rover:4;	/* rstack overflow marker */
#endif
	}fields;
};

extern union pswformat psw;		/* processor status word */

extern forthword pstack[];		/* parameter stack */
extern int ptop;
extern int pover, punder;

extern forthword rstack[];		/* return stack */
extern int rtop;
extern int rover, runder;

extern forthword pca, pcb;		/* Program Counter latches */

extern forthword aluout;		/* ALU output latch */

extern forthword alua, alub;		/* ALU input registers */

extern forthword barrelout;		/* barrel shifter output */

extern forthword y;			/* y shift register */

extern union opstruct ir;		/* instruction register */

extern int flag;			/* accumulator flag */

extern forthword abus,			/* bus A */
		 bbus,			/* bus B */
		 tbus;			/* bus T */

extern union sswformat ssw;		/* stack cache status word */
extern forthword externalrsp,		/* external return stack overflow */
		 externalpsp;		/* external parameter stack overflow */
extern int cpselect;			/* coprocessor select */

extern forthword udr0, udr1, udr2, udr3;/* user defined registers */

extern unsigned int intedgedetected;	/* edge detect latches */
extern unsigned int intmask;		/* interrupt masks */
extern unsigned int intmode;		/* interrupt modes: 0=level 1=edge */
