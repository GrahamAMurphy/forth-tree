/* This file has definitions of things internal to the SC32. */

#define PSTACKSIZE 16			/* size of parameter stack */
#define RSTACKSIZE 16			/* size of return stack */

union pswformat {			/* format of processor status word */
	int32 value;			/* for assigning to psw */
	struct {			/* for accessing individual fields */
#ifdef RIGHTBITS	/* some compilers allocate bits from right to left */
		unsigned :3,		/* unused */
			 cacheenabled:1,/* cache enabled */
			 ie:1,		/* interrupt enable */
			 ibrenable:1,	/* internal boot ROM enable */
			 :10,		/* unused (IBM PC compiler hack) */
			 :16;
#else			/* and others allocate from left to right */
		unsigned :16,		/* unused */
			 :10,
			 ibrenable:1,	/* internal boot ROM enable */
			 ie:1,		/* interrupt enable */
			 cacheenabled:1,/* cache enabled */
			 :3;		/* unused */
#endif
	}fields;
};

union sswformat {			/* format of stack cache status word */
	int32 value;			/* for assigning to ssw */
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

extern int32 pstack[];			/* parameter stack */
extern int ptop;
extern int pover;
extern int punder;

extern int32 rstack[];			/* return stack */
extern int rtop;
extern int rover;
extern int runder;

extern int32 hold;			/* return stack to abus sync. reg. */

extern int32 aluout;			/* ALU output register */

extern int32 barrelout;			/* barrel shifter output */

extern int32 y;				/* Y shift register */

extern union opstruct ir,		/* instruction register */
		      oldir;		/* previous ir writeback */

extern int32 portaddr;			/* port address */
extern int32 portvalue;			/* port value */
extern int portio;			/* read/write operation */

extern int32 dhold;			/* load holding register */

extern int flag;			/* accumulator flag */

extern int32	abus,			/* bus A */
		dbus,			/* bus D */
		bbus,			/* bus B */
		tbus,			/* bus T */
		wbus;			/* bus W */

extern union sswformat ssw;		/* stack cache status word */
extern int32 externalrsp,		/* external return stack overflow */
		externalpsp;		/* external parameter stack overflow */

extern int actualie;			/* actual interrupt enable, delayed
 					   one instruction from ie in psw*/

extern int32 udr0, udr1, udr2, udr3;/* user defined registers */

extern int32 pc;			/* program counter register pair */

extern int32 cycle;			/* number of execution cycles */
extern int32 stallcycle;		/* number of stall cycles */

extern int32 *torptr;			/* who has TOR? */

extern int biustall;			/* stall signals */
extern int cachestall;

extern unsigned int intedgedetected;	/* edge detect latches */
extern unsigned int intmask;		/* interrupt masks */
extern unsigned int intmode;		/* interrupt modes: 0=level 1=edge */
extern unsigned int intlevel;		/* interrupt level: 0=low 1=high */
