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
extern int pover;
extern int punder;

extern forthword rstack[];		/* return stack */
extern int rtop;
extern int rover;
extern int runder;

extern forthword hold;			/* return stack to abus sync. reg. */

extern forthword aluout;		/* ALU output register */

extern forthword barrelout;		/* barrel shifter output */

extern forthword y;			/* Y shift register */

extern union opstruct irb, irb,		/* instruction register */
			oldir;		/* previous ir writeback */
extern union opstruct irholdb, irholda;	/* instruction holding register */

extern forthword portaddr;		/* port address */
extern forthword portvalue;		/* port value */
extern int portio;			/* read/write operation */

extern forthword dhold;			/* load holding register */

extern int flag;			/* accumulator flag */

extern forthword abus,			/* bus A */
		 bbus;			/* bus B */

extern union sswformat ssw;		/* stack cache status word */
extern forthword externalrsp,		/* external return stack overflow */
		externalpsp;		/* external parameter stack overflow */
extern int cpselect;			/* coprocessor select */

extern int actualie;			/* actual interrupt enable, delayed
 					   one instruction from ie in psw*/

extern forthword udr0, udr1, udr2, udr3;/* user defined registers */

extern forthword pca, pcb;		/* program counter register pair */
extern forthword pchold;		/* pc to bbus sync. register */

extern int cycle;			/* cycle number */
extern int phase;			/* phase number */

extern forthword *torptr;		/* who has TOR? */

extern unsigned int intedgedetected;	/* edge detect latches */
extern unsigned int intmask;		/* interrupt masks */
extern unsigned int intmode;		/* interrupt modes: 0=level 1=edge */
