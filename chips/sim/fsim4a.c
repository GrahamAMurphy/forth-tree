#include <stdio.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "globals.h"
#include "external.h"
#include "intrnl4a.h"
#include "frmts4.h"
#include "masks4.h"

#define BIT(n)	(1<<(n))

#define USAGE \
"usage: fsim4 [-s] [-c fl] [-t fl] [-v fl] [-b] [-r x] [objfile]\n"

#define MSB 0x80000000			/* useful bit patterns */
#define LSB 0x00000001

#define S_PHASE	0			/* single phase step */
#define S_CYCLE	1			/* single cycle step */

#define A_INSTR 0			/* instruction access */
#define A_DATA  0x80000000		/* data access */
#define A_LOAD  0			/* load access */
#define A_STORE 0x40000000		/* store access */

#define T_INT 0				/* interrupt state */
#define T_EXEC 1			/* execute state */
#define T_LOAD 2			/* load state */
#define T_STORE 3			/* store state */
#define T_GRANT 4			/* DMA state */
#define T_RSDEC 5			/* return stack full, cycle 1 */
#define T_RSSTORE 6			/* return stack full, cycle 2 */
#define T_RSLOAD 7			/* return stack fill, cycle 1 */
#define T_RSINC 8			/* return stack fill, cycle 2 */
#define T_PSDEC 9			/* parameter stack full, cycle 1 */
#define T_PSSTORE 10			/* parameter stack full, cycle 2 */
#define T_PSLOAD 11			/* parameter stack fill, cycle 1 */
#define T_PSINC 12			/* parameter stack fill, cycle 2 */
#define CYCLETYPES 13			/* total number of cycles types */

static char *cyclenames[] = {		/* cycle names, for summary */
	"interrupt", "execute", "load", "store", "grant",
	"roverdec", "roverstore", "runderload", "runderinc",
	"poverdec", "poverstore", "punderload", "punderinc" };

static char *instrnames[] = {		/* instruction names, for summary */
	"call", "branch", "?branch", "micro",
	"load", "store", "lal", "lah" };

union pswformat psw;			/* processor status word */

forthword pstack[PSTACKSIZE];		/* parameter stack */
int ptop=PSTACKSIZE-1;
int pover;
int punder;

forthword rstack[RSTACKSIZE];		/* return stack */
int rtop=RSTACKSIZE-1;
int rover;
int runder;

forthword hold;				/* return stack to abus sync. reg. */

forthword aluout;			/* ALU output register */

forthword barrelout;			/* barrel shifter output */

forthword y;				/* Y shift register */

union opstruct irb, ira,		/* instruction register */
	       oldir;			/* previous ir writeback */
union opstruct irholdb, irholda;	/* instruction holding register */

forthword portaddr;			/* port address */
forthword portvalue;			/* port value */
int portio;				/* read/write operation */

forthword dhold;			/* load holding register */

int flag;				/* accumulator flag */

forthword abus,				/* bus A */
	  bbus;				/* bus B */

union sswformat ssw;			/* stack cache status word */
forthword externalrsp,			/* external return stack overflow */
	  externalpsp;			/* external parameter stack overflow */
int cpselect;				/* coprocessor select */

int actualie;				/* actual interrupt enable, delayed
					   one instruction from ie in psw*/

forthword udr0, udr1, udr2, udr3;	/* user defined registers */

forthword pca, pcb;			/* program counter register pair */
forthword pchold;			/* pc to bbus sync. register */

int cycle;				/* cycle number */
int phase;				/* phase number */

int quitflag;				/* set by terminate in fakeio */

forthword *torptr;			/* who has TOR? */

static void (*writeback)();		/* register writeback function */

static forthword *lowmem;		/* low memory array */
static unsigned int lowtop = 0x10000;	/* maximum memory address (default) */

static forthword *highmem;		/* high memory array */
#define hightop 0x0fffffff		/* maximum memory address */
static unsigned int highbottom = hightop+1; /* base address */

static forthword ibrom[] = {		/* internal boot ROM image */
#include "ibrom4.h"
};
#define ibromtop    0x1fffffff
#define ibrombottom 0x10000000		/* bottom and top ROM addresses */

static int stepmode = S_CYCLE;		/* S_CYCLE: single cycle,
					   S_PHASE: single phase  */

static unsigned int intactive;		/* interrupt inputs */
       unsigned int intedgedetected;	/* edge detect latches */
       unsigned int intmask;		/* interrupt masks */
       unsigned int intmode;		/* interrupt modes: 0=level 1=edge */
static unsigned int intvector;		/* interrupt vector */

static int busrequest = FALSE;		/* bus request for DMA */

static int state = T_EXEC;		/* machine state */

static instrtypes[8];			/* count instruction types */
static cycletypes[CYCLETYPES];		/* count cycle types */

static int tracing = FALSE;		/* no trace by default */

/*		Declarations of functions defined in this file */
void Nop();
void Startup();
static void Loadobj();
static void Loadbytepromobj();
void Saveobj();
forthword Fetchexternal();
void Storeexternal();
void Cycle();
void Phase();
void Singlestep();
void Sim();
void Simcycles();
void Vsim();
void Resetregs();
void Startat();
void Stack();
void Waitcycle();
void Dmastart();
void Dmaend();
void Interrupton();
void Interruptoff();
int Interruptpending();
int Interruptvector();
void Nextstate();
void Phia();
void Phib();
void Loadphia();
void Loadphib();
void Loadwriteback();
void Storephia();
void Storephib();
void Execphia();
void Execphib();
void Intphia();
void Intphib();
void Dmaphia();
void Dmaphib();
void Rsover1phia();
void Rsover1phib();
void Rsover2phia();
void Rsover2phib();
void Rsunder1phia();
void Rsunder1phib();
void Rsunder2phia();
void Rsunder2phib();
void Rsunderflowwriteback();
void Psover1phia();
void Psover1phib();
void Psover2phia();
void Psover2phib();
void Psunder1phia();
void Psunder1phib();
void Psunder2phia();
void Psunder2phib();
void Psunderflowwriteback();
forthword Asrca();
void Stackop();
forthword Bsrcb();
forthword Asrcb();
forthword Readreg();
forthword Alu();
forthword Microalu();
forthword Booth1();
forthword Booth2();
forthword Barrel();
forthword Microbarrel();
void Writereg();
void Execcleanup();
void Aluwriteback();
void Aluwritebacktor();
void Shiftwriteback();
void Clearstats();
void Summary();
void Traceon();
void Traceoff();

void
Nop()
{}

void
Startup(argc,argv)
int argc;
char *argv[];
{
	extern char *optarg;		/* for getopt() */
	extern int optind;
	int c;
	char *objcode = "a.out";	/* default object code file */
	FILE *fp;
	int bytepromboot = FALSE;	/* TRUE loads obj file in byte-format */


	while((c=getopt(argc,argv,"bc:r:st:v:")) != EOF){
		switch(c){
		   case 'b':
			bytepromboot = TRUE;
			break;
		   case 'c':
			if(Saveopen(optarg)==NULL){
				fprintf(stderr,
				   "Can't open save file %s\n",optarg);
				exit(1);
			}
			break;
		   case 'r':
			sscanf(optarg,"%x",&lowtop);
			break;
		   case 's':
			summarize = TRUE;
			break;
		   case 't':
			if(Traceopen(optarg)==NULL){
				fprintf(stderr,
				   "Can't open trace file %s\n",optarg);
				exit(1);
			}
			break;
		   case 'v':
			genvectors = TRUE;
			if(Vecopen(optarg)==NULL){
				fprintf(stderr,
				   "Can't open vector file %s\n",optarg);
				exit(1);
			}
			break;
		   case '?':
			fprintf(stderr,USAGE);
			exit(1);
			break;
		}
	}
	if(optind<argc) objcode = argv[optind];
	if((fp=fopen(objcode,"r")) == NULL){
		fprintf(stderr,"Can't open object file\n");
		exit(1);
	}
	if(bytepromboot) Loadbytepromobj(fp);
	else		 Loadobj(fp);
	fclose(fp);
}

static void
Loadobj(fp)
FILE *fp;
{
	struct stat buf;	/* file status buffer */

	fstat(fileno(fp),&buf);
	if((unsigned int)buf.st_size > (lowtop * sizeof(forthword))){
		fprintf(stderr,"Too big; use -r to allocate more memory\n");
		exit(1);
	}
	if((lowmem=(forthword *)malloc(lowtop * sizeof(forthword))) == NULL){
		fprintf(stderr,"Malloc failed\n");
		exit(1);
	}
	fread(lowmem,(unsigned int)buf.st_size,1,fp);
}

static void
Loadbytepromobj(fp)
FILE *fp;
{
	struct stat buf;	/* file status buffer */
	forthword *mem;
	int c;

	fstat(fileno(fp),&buf);
	/* allocate high memory for external boot PROM */
	if((highmem=(forthword *)calloc((int)buf.st_size,sizeof(forthword)))
			== NULL){
		fprintf(stderr,"Malloc failed\n");
		return;
	}
	highbottom = hightop + 1 - buf.st_size;
	mem = highmem;
	while((c=getc(fp)) != EOF) *mem++ = (forthword)c | 0xfefefe00;

	/* allocate low memory too */
	if((lowmem=(forthword *)calloc((unsigned int)lowtop,sizeof(forthword)))
			== NULL){
		fprintf(stderr,"Malloc failed\n");
		return;
	}
}

void
Saveobj()
{
/* TBD should save in proper boot ROM format */
	Savecore(lowmem, lowtop+1);
}

forthword
Fetchexternal(addr)
forthword addr;
{
	if((unsigned)addr <= lowtop){
		return(lowmem[addr]);
	} else if((unsigned)addr > TRAPSPACE){
		return(Fakeio(addr));
	} else if(((unsigned)addr >= highbottom) &&
		  ((unsigned)addr <= hightop)){
		return(highmem[addr-highbottom]);
	} else if(psw.fields.ibrenable &&
		  ((unsigned)addr >= ibrombottom) &&
		  ((unsigned)addr <= ibromtop)){
		return(ibrom[addr-ibrombottom]);
	} else {
		Ioread(addr,1);
		return(1);
	}
}

void
Storeexternal(addr,value)
forthword addr;
forthword value;
{
	if((unsigned)addr <= lowtop){
		lowmem[addr] = value;
	} else if((unsigned)addr > TRAPSPACE){
		Fakeioarg(value);
	} else if(((unsigned)addr >= highbottom) &&
		  ((unsigned)addr <= hightop)){
		highmem[addr-highbottom] = value;
	} else if(psw.fields.ibrenable &&
		  ((unsigned)addr >= ibrombottom) &&
		  ((unsigned)addr <= ibromtop)){
		/* do nothing; it is a ROM */ ;
	} else {
		Iowrite(addr,value);
	}
}

void
Cycle()
{
	stepmode = S_CYCLE;
}

void
Phase()
{
	stepmode = S_PHASE;
}

void
Singlestep()
{
	if(stepmode == S_CYCLE){
		if(phase == 2) Phib();
		Phia(); Phib();
	} else {	/* S_PHASE */
		if(phase == 1) Phia();
		else	       Phib();
	}
}

void
Sim(addr,times)
forthword addr;
int times;
{
	quitflag = FALSE;
	if(phase == 2) Phib();
	while((times!=0) && !quitflag){
		Phia(); Phib();
		if(pca == addr) times--;
	}
}

void
Simcycles(cycles)
int cycles;
{
	quitflag = FALSE;
	if(phase == 2) Phib();
	while((--cycles >= 0) && !quitflag){
		Phia(); Phib();
	}
}

void
Vsim(addr)
forthword addr;
{
	quitflag = FALSE;
	if(phase == 2) Phib();
	while((pca != addr) && !quitflag){
		Phia(); Phib();
		Updatescr();
	}
}

void
Resetregs()
{
	state = T_EXEC;
	abus = bbus = 0;
	ssw.fields.ptop = ptop = PSTACKSIZE-1;
	ssw.fields.rtop = rtop = RSTACKSIZE-1;
	ssw.fields.pover = pover = 0;
	punder = (pover+4) & PSTACKSIZE-1;
	ssw.fields.rover = rover = 0;
	runder = (rover+4) & RSTACKSIZE-1;
	ssw.fields.rcache = ssw.fields.pcache = 0; /* disable caches */
	actualie = psw.fields.ie = 0;		/* disable interrupts */
	cpselect = psw.fields.cpselect = 0;	/* map in UDRs */
	psw.fields.ibrenable = 1;		/* enable internal boot ROM */
	intactive = 0;				/* no interrupt requests */
	busrequest = FALSE;
	irb.irvalue = ira.irvalue = irholdb.irvalue = irholda.irvalue = 0;
	y = 0;
	flag = 0;
	pcb = pca = portaddr = 0;
	portvalue = 0;
	portio = PORT_RD;
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Nop;
	cycle = 0;
	phase = 1;
	Startat(ibrombottom);
}

void
Startat(addr)
forthword addr;
{
	pca = portaddr = abus = addr;
	irb.irvalue = portvalue = Fetchexternal(addr);
	pcb = pca + 1;
	if(genvectors){
		Vecldphia();
		Vecldphib();
	}
}

void
Stack(num)
forthword num;
{
	if(++ptop==PSTACKSIZE) ptop=0;
	pstack[ptop] = num;
}

void
Waitcycle()
{
	cycle++;
}

void
Dmastart()
{
	busrequest = TRUE;
}

void
Dmaend()
{
	busrequest = FALSE;
}

/****************************************************************************
 *	Here is a model of the interrupt controller
 */

void
Interrupton(n)
int n;
{
	intedgedetected |= (~intactive) & BIT(n);
	intactive |= BIT(n);
}

void
Interruptoff(n)
int n;
{
	intactive &= ~BIT(n);
}

int
Interruptpending()
{
	unsigned int pending = intmask & ((~intmode & intactive) |
					  ( intmode & intedgedetected));
	if(pending){
		unsigned int mask;
		for(mask=1, intvector=0; (pending&mask)==0;
					mask<<=1, intvector++)
			;
	}
	return(pending != 0);
}

int
Interruptvector()
{
	intedgedetected &= ~BIT(intvector);
	return(intvector);
}

/****************************************************************************
 *	Here is a model of the state machine the controls the chip
 */

void
Nextstate()
{
	switch(state){
	   case T_INT:
		state = T_EXEC; break;
	   case T_GRANT:
		if(busrequest){
			break;
		} else if(actualie && Interruptpending()){
			state = T_INT;
		} else {
			state = T_EXEC;
		}
		break;
	   case T_EXEC:
		if(ira.irbranch.type == B_LOAD){
			state = T_LOAD;
		} else if(ira.irbranch.type == B_STORE){
			state = T_STORE;
		} else if(ssw.fields.rcache && rtop==rover) {
			state = T_RSDEC;
		} else if(ssw.fields.rcache && rtop==runder) {
			state = T_RSLOAD;
		} else if(ssw.fields.pcache && ptop==pover) {
			state = T_PSDEC;
		} else if(ssw.fields.pcache && ptop==punder) {
			state = T_PSLOAD;
		} else if(actualie && Interruptpending()) {
			state = T_INT;
		} else if(busrequest){
			state = T_GRANT;
		} else {
			state = T_EXEC;
		}
		break;
	   case T_RSSTORE:
	   case T_RSINC:
	   case T_PSSTORE:
	   case T_PSINC:
	   case T_LOAD:
	   case T_STORE:
		if(ssw.fields.rcache && rtop==rover) {
			state = T_RSDEC;
		} else if(ssw.fields.rcache && rtop==runder) {
			state = T_RSLOAD;
		} else if(ssw.fields.pcache && ptop==pover) {
			state = T_PSDEC;
		} else if(ssw.fields.pcache && ptop==punder) {
			state = T_PSLOAD;
		} else if(actualie && Interruptpending()) {
			state = T_INT;
		} else if(busrequest){
			state = T_GRANT;
		} else {
			state = T_EXEC;
		}
		break;
	   case T_RSDEC:
		state = T_RSSTORE; break;
	   case T_RSLOAD:
		state = T_RSINC; break;
	   case T_PSDEC:
		state = T_PSSTORE; break;
	   case T_PSLOAD:
		state = T_PSINC; break;
	}
}

/****************************************************************************
 *	Phia and Phib handle the phase a and phase b of the current
 *	instruction respectively.
 */

void
Phia()
{
	actualie = psw.fields.ie;	/* delayed ie */
	irholda = irholdb;		/* always clocked on phase A */
	ira = irb;
	pchold = pcb;
	(*writeback)();			/* write back result of previous cycle*/
	switch(state){
	   case T_INT:
		Intphia();
		if(genvectors) Vecintphia();
		break;
	   case T_GRANT:
		Dmaphia();
		if(genvectors) Vecdmaphia();
		break;
	   case T_EXEC:
		Execphia();
		if(tracing) Logliteral(irb);
		if(genvectors) Vecldphia();
		break;
	   case T_LOAD:
		Loadphia();
		if(genvectors) Vecldphia();
		break;
	   case T_STORE:
		Storephia();
		if(genvectors) Vecstphia();
		break;
	   case T_RSDEC:
		Rsover1phia();
		if(genvectors) Vecldphia();
		break;
	   case T_RSSTORE:
		Rsover2phia();
		if(genvectors) Vecstphia();
		break;
	   case T_RSLOAD:
		Rsunder1phia();
		if(genvectors) Vecldphia();
		break;
	   case T_RSINC:
		Rsunder2phia();
		if(genvectors) Vecldphia();
		break;
	   case T_PSDEC:
		Psover1phia();
		if(genvectors) Vecldphia();
		break;
	   case T_PSSTORE:
		Psover2phia();
		if(genvectors) Vecstphia();
		break;
	   case T_PSLOAD:
		Psunder1phia();
		if(genvectors) Vecldphia();
		break;
	   case T_PSINC:
		Psunder2phia();
		if(genvectors) Vecldphia();
		break;
	}
}

void
Phib()
{
	switch(state){
	   case T_INT:
		Intphib();
		if(genvectors) Vecintphib();
		break;
	   case T_GRANT:
		Dmaphib();
		if(genvectors) Vecdmaphib();
		break;
	   case T_EXEC:
		Execphib();
		if(genvectors) Vecldphib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	   case T_LOAD:
		Loadphib();
		if(genvectors) Vecldphib();
		if(tracing) Trace(A_DATA | A_LOAD);
		break;
	   case T_STORE:
		Storephib();
		if(genvectors) Vecstphib();
		if(tracing) Trace(A_DATA | A_STORE);
		break;
	   case T_RSDEC:
		Rsover1phib();
		if(genvectors) Vecldphib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	   case T_RSSTORE:
		Rsover2phib();
		if(genvectors) Vecstphib();
		if(tracing) Trace(A_DATA | A_STORE);
		break;
	   case T_RSLOAD:
		Rsunder1phib();
		if(genvectors) Vecldphib();
		if(tracing) Trace(A_DATA | A_LOAD);
		break;
	   case T_RSINC:
		Rsunder2phib();
		if(genvectors) Vecldphib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	   case T_PSDEC:
		Psover1phib();
		if(genvectors) Vecldphib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	   case T_PSSTORE:
		Psover2phib();
		if(genvectors) Vecstphib();
		if(tracing) Trace(A_DATA | A_STORE);
		break;
	   case T_PSLOAD:
		Psunder1phib();
		if(genvectors) Vecldphib();
		if(tracing) Trace(A_DATA | A_LOAD);
		break;
	   case T_PSINC:
		Psunder2phib();
		if(genvectors) Vecldphib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	}
	cycletypes[state]++;
}

/****************************************************************************
 *	Loadphia and Loadphib handle the second cycle of load
 *	instructions.
 */

void
Loadphia()
{
	portaddr = abus = aluout;
	portio = PORT_RD;
	phase = 2;
}

void
Loadphib()
{
	dhold = portvalue = Fetchexternal(portaddr);
	writeback = Loadwriteback;
	hold = rstack[rtop];
	torptr = ira.irload.reg2 == B_R0R2 ? &dhold : &hold;
	irb = irholda;
	pcb = pca + 1;
	Nextstate();
	phase = 1;
	cycle++;
}

void
Loadwriteback()
{
	bbus = dhold;
	Writereg(oldir.irload.reg2);
}

/****************************************************************************
 *	Storephia and Storephib handle the second cycle of store
 *	instructions.
 */

void
Storephia()
{
	portaddr = abus = aluout;
	portio = PORT_WR;
	phase = 2;
}

void
Storephib()
{
	portvalue = bbus = Readreg(ira.irload.reg2);
	Storeexternal(portaddr, portvalue);
	writeback = Nop;
	hold = rstack[rtop];
	torptr = &hold;
	irb = irholda;
	pcb = pca + 1;
	Nextstate();
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	Execphia and Execphib handle the first cycle of all instructions.
 */

void
Execphia()
{
	pca = portaddr = abus = Asrca();
	portio = PORT_RD;
	phase = 2;
}

void
Execphib()
{
	instrtypes[ira.irbranch.type]++;	/* statistics */
	bbus = Bsrcb();
	abus = Asrcb();
	Stackop();
	hold = rstack[rtop];
	aluout = Alu();
	barrelout = Barrel();
	oldir = ira;				/* for simulation only */
	irholdb.irvalue = portvalue = Fetchexternal(portaddr);
	Execcleanup();
	pcb = pca + 1;
	Nextstate();
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	Intphia and Intphib handle the interrupt acknowledge cycle.
 */

void
Intphia()
{
	portaddr = abus = Interruptvector();
	portio = PORT_RD;
	psw.fields.ie = 0;
	phase = 2;
}

void
Intphib()
{
	writeback = Nop;
	hold = rstack[rtop];
	torptr = &hold;
	pcb = pca;
	Nextstate();
	irb.irvalue = portvalue = Fetchexternal(portaddr);
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	Dmaphia and Dmaphib handle DMA cycles.
 */

void
Dmaphia()
{
	phase = 2;
}

void
Dmaphib()
{
	pcb = pca + 1;
	Nextstate();
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	The following functions handle the phase a and phase b hardware
 *	overflow and underflow operations
 */

void
Rsover1phia()
{
	portaddr = abus = pcb;
	portio = PORT_RD;
	phase = 2;
}

void
Rsover1phib()
{
	if(++rover == RSTACKSIZE) rover=0;
	if(++runder == RSTACKSIZE) runder=0;
	bbus = externalrsp;
	aluout = bbus-1;
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Nop;
	irb = irholda;
	pcb = pca + 1;
	Nextstate();
	portvalue = Fetchexternal(portaddr);
	phase = 1;
	cycle++;
}

void
Rsover2phia()
{
	portaddr = abus = aluout;
	externalrsp = bbus = aluout;
	portio = PORT_WR;
	phase = 2;
}

void
Rsover2phib()
{
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Nop;
	irb = irholda;
	portvalue = bbus = rstack[(rtop+1) & RSTACKSIZE-1];
	Storeexternal(portaddr, portvalue);
	pcb = pca + 1;
	Nextstate();
	phase = 1;
	cycle++;
}

void
Rsunder1phia()
{
	portaddr = abus = pcb;
	portio = PORT_RD;
	phase = 2;
}

void
Rsunder1phib()
{
	if(--rover == -1) rover=RSTACKSIZE-1;
	if(--runder == -1) runder=RSTACKSIZE-1;
	hold = externalrsp;
	bbus = externalrsp;
	aluout = bbus+1;
	writeback = Nop;
	irb = irholda;
	pcb = pca + 1;
	Nextstate();
	portvalue = Fetchexternal(portaddr);
	phase = 1;
	cycle++;
}

void
Rsunder2phia()
{
	portaddr = abus = hold;
	portio = PORT_RD;
	externalrsp = bbus = aluout;
	phase = 2;
}

void
Rsunder2phib()
{
	dhold = portvalue = Fetchexternal(portaddr);
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Rsunderflowwriteback;
	pcb = pca + 1;
	irb = irholda;
	Nextstate();
	phase = 1;
	cycle++;
}

void
Rsunderflowwriteback()
{
	rstack[(rtop-4) & RSTACKSIZE-1] = bbus = dhold;
}

void
Psover1phia()
{
	portaddr = abus = pcb;
	portio = PORT_RD;
	phase = 2;
}

void
Psover1phib()
{
	if(++pover == PSTACKSIZE) pover=0;
	if(++punder == PSTACKSIZE) punder=0;
	bbus = externalpsp;
	aluout = bbus-1;
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Nop;
	irb = irholda;
	pcb = pca + 1;
	Nextstate();
	portvalue = Fetchexternal(portaddr);
	phase = 1;
	cycle++;
}

void
Psover2phia()
{
	portaddr = abus = aluout;
	externalpsp = bbus = aluout;
	portio = PORT_WR;
	phase = 2;
}

void
Psover2phib()
{
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Nop;
	irb = irholda;
	portvalue = bbus = pstack[(ptop+1) & PSTACKSIZE-1];
	Storeexternal(portaddr, portvalue);
	pcb = pca + 1;
	Nextstate();
	phase = 1;
	cycle++;
}

void
Psunder1phia()
{
	portaddr = abus = pcb;
	portio = PORT_RD;
	phase = 2;
}

void
Psunder1phib()
{
	if(--pover == -1) pover=PSTACKSIZE-1;
	if(--punder == -1) punder=PSTACKSIZE-1;
	hold = externalpsp;
	bbus = externalpsp;
	aluout = bbus+1;
	writeback = Nop;
	irb = irholda;
	pcb = pca + 1;
	Nextstate();
	portvalue = Fetchexternal(portaddr);
	phase = 1;
	cycle++;
}

void
Psunder2phia()
{
	portaddr = abus = hold;
	portio = PORT_RD;
	externalpsp = bbus = aluout;
	phase = 2;
}

void
Psunder2phib()
{
	dhold = portvalue = Fetchexternal(portaddr);
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Psunderflowwriteback;
	pcb = pca + 1;
	irb = irholda;
	Nextstate();
	phase = 1;
	cycle++;
}

void
Psunderflowwriteback()
{
	pstack[(ptop-4) & PSTACKSIZE-1] = bbus = dhold;
}

/****************************************************************************
 *	The following functions handle everything that can happen on phase
 *	a in the EXEC state.
 */

forthword
Asrca()
{
	forthword result;

	switch(irb.irbranch.type){
	   case B_COLON:
	   case B_BRANCH:
		result = irb.irbranch.dest;
		break;
	   case B_QBRANCH:
		result = flag ? pcb : irb.irbranch.dest;
		break;
	   case B_LAL:
	   case B_LAH:
	   case B_LOAD:
	   case B_STORE:
	   case B_MICRO:
		result = (irb.irload.ret == B_RETURN) ? *torptr : pcb;
		break;
	   default:
		Cmdmsg("Uknown instruction type");
		break;
	}
	return(result);
}

/****************************************************************************
 *	The following functions handle everything that can happen on phase
 *	b in the EXEC state.
 */

forthword
Bsrcb()
{
	forthword result;

	switch(ira.irbranch.type){
	   case B_COLON:
	   case B_BRANCH:			/* DEFAULT */
	   case B_QBRANCH:			/* DEFAULT */
		result = pchold;
		break;
	   case B_LAL:
	   case B_LAH:
	   case B_LOAD:
	   case B_STORE:
	   case B_MICRO:
		result = Readreg(ira.irload.reg1);
		break;
	   default:
		Cmdmsg("Unknown instruction type");
		break;
	}
	return(result);
}

forthword
Asrcb()
{
	forthword result;

	switch(ira.irbranch.type){
	   case B_LAL:
	   case B_LOAD:
	   case B_STORE:
		result = ira.irload.offset;
		break;
	   case B_LAH:
		result = ira.irload.offset<<16;
		break;
	   case B_COLON:			/* DEFAULT */
	   case B_BRANCH:			/* DEFAULT */
	   case B_QBRANCH:			/* DEFAULT TBD */
	   case B_MICRO:
		if((ira.iralu.class == B_SHIFT) &&
		   (ira.irshift.imm == B_IMM)){
			result = ira.irshift.literal;
		} else {
			result = pstack[ptop];
		}
		break;
	   default:
		Cmdmsg("Unknown instruction type");
		break;
	}
	return(result);
}

void
Stackop()
{
	switch(ira.irbranch.type){
	   case B_COLON:
		rtop++; rtop &= RSTACKSIZE-1; break;
	   case B_BRANCH:
	   case B_QBRANCH:
		break;
	   case B_LAL:
	   case B_LAH:
	   case B_LOAD:
	   case B_STORE:
	   case B_MICRO:
		switch(ira.irload.pstack){
		   case B_NOPPS:	break;
		   case B_PUSHPS:	ptop++; ptop &= PSTACKSIZE-1; break;
		   case B_POPPS:	ptop--; ptop &= PSTACKSIZE-1; break;
		   default:		Cmdmsg("Unknown stack op\n"); break;
		}
	
		switch(ira.irload.rstack){
		   case B_NOPRS:	break;
		   case B_PUSHRS:	rtop++; rtop &= RSTACKSIZE-1; break;
		   case B_POPRS:	rtop--; rtop &= RSTACKSIZE-1; break;
		   default:		Cmdmsg("Unknown stack op\n"); break;
		}
		break;
	   default:
		Cmdmsg("Unknown instruction type"); break;
	}
}

forthword
Readreg(r)
int r;
{
	forthword tmp;
	/* WARNING: this code assumes that the encoding of the r1 and
	   r2 fields is identical! */
	switch(r){
	   case B_S0R1:		tmp = pstack[(ptop)   & PSTACKSIZE-1]; break;
	   case B_S1R1:		tmp = pstack[(ptop-1) & PSTACKSIZE-1]; break;
	   case B_S2R1:		tmp = pstack[(ptop-2) & PSTACKSIZE-1]; break;
	   case B_S3R1:		tmp = pstack[(ptop-3) & PSTACKSIZE-1]; break;
	   case B_R0R1:		tmp = rstack[(rtop)   & RSTACKSIZE-1]; break;
	   case B_R1R1:		tmp = rstack[(rtop-1) & RSTACKSIZE-1]; break;
	   case B_R2R1:		tmp = rstack[(rtop-2) & RSTACKSIZE-1]; break;
	   case B_R3R1:		tmp = rstack[(rtop-3) & RSTACKSIZE-1]; break;
	   case B_IARR1:	tmp = pchold; break;
	   case B_PSWR1:	psw.fields.cpselect = cpselect;
				tmp = psw.value; break;
	   case B_ZEROR1:	tmp = 0; break;
	   case B_YR1:		tmp = y; break;
	   default:
		switch(cpselect){
		   case 0:
			switch(r){
	   		   case B_U0R1: tmp = udr0; break;
	   		   case B_U1R1: tmp = udr1; break;
	   		   case B_U2R1: tmp = udr2; break;
	   		   case B_U3R1: tmp = udr3; break;
		   	   default:	Cmdmsg("Unknown cp0 register\n"); break;
			}
			break;
		   case 1:
			switch(r){
	   		   case B_U0R1:	tmp = externalrsp; break;
	   		   case B_U1R1:	tmp = externalpsp; break;
	   		   case B_U2R1: ssw.fields.ptop = ptop;
					ssw.fields.pover = pover;
					ssw.fields.rtop = rtop;
					ssw.fields.rover = rover;
					tmp = ssw.value;
					break;
		   	   default:	Cmdmsg("Unknown cp1 register\n"); break;
			}
			break;
		   case 2:
			switch(r){
			   case B_U0R1:	tmp = intmask; break;
			   case B_U1R1:	tmp = intmode; break;
			   case B_U2R1:	tmp = intedgedetected; break;
		   	   default:	Cmdmsg("Unknown cp2 register\n"); break;
			}
			break;
		   default:	Cmdmsg("Unknown coprocessor\n"); break;
		}
	}
	return(tmp);
}

int alucond;				/* selected ALU condition */

#define COUT(x)		((x))
#define OVFLOW(x)	((x)>>1)
#define EQUAL(x)	((x)>>2)
#define NEG(x)		((x)>>3)

#define CIN(x)		(((x) & 1) << 4) /* TBD right for sparc? */

forthword
Alu()
{
	forthword result;

	switch(ira.irbranch.type){
	   case B_COLON:
	   case B_BRANCH:
	   case B_QBRANCH:
		result = bbus;
		break;
	   case B_LAL:
	   case B_LAH:
	   case B_LOAD:
	   case B_STORE:
		result = abus + bbus;
		break;
	   case B_MICRO:
		result = Microalu();
		break;
	   default:
		Cmdmsg("Unknown instruction type");
		break;
	}
	return(result);
}

forthword
Microalu()
{
	int result;
	short flags = 0;		/* XNZVC flags */
	int flinput;			/* flag input source */

	switch(ira.iralu.class){
	   case B_ALU:
	   case B_TEST: {
		int cin;		/* carry input */

		switch(ira.iralu.cin){
		   case B_ZERO:		cin = 0; break;
		   case B_ONE:		cin = 1; break;
		   case B_CFLAG:	cin = flag; break;
		   case B_CFLAGBAR:	cin = ~flag; break;
		   default:		Cmdmsg("Unknown carry in\n"); break;
		}
		switch(ira.iralu.aluop){
		   case B_ONES:		result = ~0; break;
		   case B_ZEROS:	result = 0; break;
		   case B_NOPA:		result = abus; break;
		   case B_NOPB:		result = bbus; break;
		   case B_NOTA:		result = ~abus; break;
		   case B_NOTB:		result = ~bbus; break;
		   case B_AANDB:	result = abus & bbus; break;
		   case B_ABARANDB:	result = ~abus & bbus; break;
		   case B_AANDBBAR:	result = abus & ~bbus; break;
		   case B_ANANDB:	result = ~(abus & bbus); break;
		   case B_AORB:		result = abus | bbus; break;
		   case B_ABARORB:	result = ~abus | bbus; break;
		   case B_AORBBAR:	result = abus | ~bbus; break;
		   case B_ANORB:	result = ~(abus | bbus); break;
		   case B_AXORB:	result = abus ^ bbus; break;
		   case B_ANXORB:	result = ~(abus ^ bbus); break;
		   case B_APLUSB:	result = Addop(abus,bbus,CIN(cin),
						&flags);
					break;
		   case B_AMINUSB:	result = Addop(abus,~bbus,CIN(cin),
						&flags);
					break;
		   case B_BMINUSA:	result = Addop(bbus,~abus,CIN(cin),
						&flags);
					break;
		   case B_NEGA:		result = Addop(~abus,0,CIN(cin),
						&flags);
					break;
		   case B_NEGB:		result = Addop(0,~bbus,CIN(cin),&flags);
					break;
		   case B_INCA:		result = Addop(abus,0,CIN(cin),&flags);
					break;
		   case B_INCB:		result = Addop(0,bbus,CIN(cin),&flags);
					break;
		   case B_DECA:		result = Addop(abus,-1,CIN(cin),&flags);
					break;
		   case B_DECB:		result = Addop(-1,bbus,CIN(cin),&flags);
					break;
		   case B_ABARPLUSBBAR:	result = Addop(~abus,~bbus,CIN(cin),
						&flags);
					break;
		   default:		Cmdmsg("Unknown ALU operation\n");
					quitflag = TRUE;
					break;
		}
		if(result<0) flags |= 0x8;
		if(result==0) flags |= 0x4;
		flinput = alucond = ALUcond(flags);
		if(ira.iralu.class == B_TEST)
			result = alucond ? -1 : 0;
/* TBD alucond still needed?*/
		break;
	   }
	case B_STEP: {
		int shiftleftresult;	/* shifted B input */
		int shiftleftout;	/* output of left shifter */
		int shiftrightout;	/* output of right shifter */
		int boothshiftout;	/* output of internal ALU right shift*/

		if(ira.irstep.left == B_LEFT){
			shiftleftout = (bbus & MSB) ? 1 : 0;
			shiftleftresult = bbus<<1;
			if(y & MSB) shiftleftresult |= LSB;
		} else {
			shiftleftresult = bbus;
		}
		switch(ira.irstep.step){
		   case B_STEPB:
		   	result = Addop(0,shiftleftresult,CIN(0),&flags);
			break;
		   case B_CADD:
			result = Addop((!flag?abus:0),shiftleftresult,CIN(0),
						&flags);
			break;
		   case B_DIV:
			result = flag ? Addop(~abus,shiftleftresult,CIN(1),
						&flags)
				      : Addop( abus,shiftleftresult,CIN(0),
						&flags);
			break;
		   case B_MUL:
			result = Booth1(abus,shiftleftresult,
						&flags,&boothshiftout);
			break;
		   default:
			Cmdmsg("Unknown step operation");
			break;
		}
		if(result<0) flags |= 0x8;
		if(result==0) flags |= 0x4;
		alucond = ALUcond(flags);
		if(ira.irstep.step == B_MUL)
			result = Booth2(result,alucond,&boothshiftout);
		if(ira.irstep.right == B_RIGHT){
			shiftrightout = result & LSB;
			result = (unsigned)result >> 1;
			if((ira.irstep.rsrc == B_SFLAG && flag) ||
			   (ira.irstep.rsrc == B_SALUCOND && alucond))
				result |= MSB;
			else
				result &= ~MSB;
		}
		switch(ira.irstep.fsrc){
		   case B_FLALUCOND:	flinput = alucond; break;
		   case B_FLY1:		flinput = (y>>1)&LSB; break;
		   case B_FLDIVHELP:	flinput = flag
					? (shiftleftout | (COUT(flags)&0x1))
					: (shiftleftout & (COUT(flags)&0x1));
					 break;
		   case B_FLRIGHTSHIFT:	flinput = shiftrightout; break;
		}
		switch(ira.irstep.ycont){
		   case B_YNOP:
			break;
		   case B_YLEFT:
			y <<= 1;
			if(flag) y |= LSB;
			break;
		   case B_YRIGHT:
			y = (unsigned)y>>2;
			if(boothshiftout) y |= (unsigned)MSB>>1;
			if(shiftrightout) y |= MSB;
			break;
		   default:
			Cmdmsg("Unknown Y operation\n");
			break;
		}
		break;
	   }
	   case B_SHIFT: {
		result = bbus;
		if(result<0) flags |= 0x8;
		if(result==0) flags |= 0x4;
		flinput = alucond = ALUcond(flags);
		break;
	   }
	}
	if(ira.iralu.flag == B_FLAG) flag = flinput;
	return(result);
}

int
ALUcond(flags)
short flags;
{
	int result;

	switch(ira.iralu.alucond){
	   case B_CLEAR:	result = 0; break;
	   case B_EQUAL:	result = EQUAL(flags); break;
	   case B_NEG:		result = NEG(flags); break;
	   case B_COUT:		result = COUT(flags); break;
	   case B_OVFLOW:	result = OVFLOW(flags); break;
	   case B_LESS:		result = NEG(flags) ^ OVFLOW(flags); break;
	   case B_ULESSEQ:	result = ~COUT(flags) | EQUAL(flags); break;
	   case B_LESSEQ:	result = (NEG(flags) ^ OVFLOW(flags))
					 | EQUAL(flags); break;
	   case B_SET:		result = 1; break;
	   case B_NEQUAL:	result = ~EQUAL(flags); break;
	   case B_NBAR:		result = ~NEG(flags); break;
	   case B_CBAR:		result = ~COUT(flags); break;
	   case B_VBAR:		result = ~OVFLOW(flags); break;
	   case B_GREATEREQ:	result = ~(NEG(flags) ^ OVFLOW(flags)); break;
	   case B_UGREATER:	result = ~(~COUT(flags) | EQUAL(flags)); break;
	   case B_GREATER:	result = ~((NEG(flags) ^ OVFLOW(flags))
					 | EQUAL(flags)); break;
	   default:		Cmdmsg("Unknown flag operation\n"); break;
	}
	return(result & LSB);			/* only lsb is flag */
}

forthword
Booth1(a, b, flagptr, shiftoutptr)
unsigned a, b;
short *flagptr;
int *shiftoutptr;
{
#define ASR(x) (((x)&MSB) ? ((x)>>1)|MSB : ((x)>>1))
	unsigned result;

	switch(((y&0x3)<<1) | flag){
	   case 0:
		*shiftoutptr = a & LSB; a = ASR(a);
		result = a;
		break;
	   case 1:
	   case 2:
		result = Addop(a,b,CIN(0),flagptr);
		break;
	   case 3:
		*shiftoutptr = a & LSB; a = ASR(a);
		result = Addop(a,b,CIN(0),flagptr);
		break;
	   case 4:
		*shiftoutptr = a & LSB; a = ASR(a);
		result = Addop(a,~b,CIN(1),flagptr);
		break;
	   case 5:
	   case 6:
		result = Addop(a,~b,CIN(1),flagptr);
		break;
	   case 7:
		*shiftoutptr = a & LSB; a = ASR(a);
		result = a;
		break;
	}
	return((forthword)result);
}

forthword
Booth2(booth1result, alucond, shiftoutptr)
unsigned booth1result;
int alucond;
int *shiftoutptr;
{
	unsigned result;

	switch(((y&0x3)<<1) | flag){
	   case 1:
	   case 2:
	   case 5:
	   case 6:
		*shiftoutptr = booth1result & LSB;
		result = booth1result >> 1;
		if(alucond) result |= MSB;
		break;
	   default:
		result = booth1result;
		break;
	}
	return((forthword)result);
}


forthword
Barrel()
{
	forthword result;

	switch(ira.irbranch.type){
	   case B_COLON:			/* DEFAULT */
	   case B_BRANCH:
	   case B_QBRANCH:
	   case B_LAL:
	   case B_LAH:
	   case B_LOAD:
	   case B_STORE:
		result = bbus;
		break;
	   case B_MICRO:
		result = Microbarrel();
		break;
	   default:
		Cmdmsg("Unknown instruction type");
		break;
	}
	return(result);
}

forthword
Microbarrel()
{
	unsigned result;

	if(ira.iralu.class == B_SHIFT){
		if(ira.irshift.pri == B_PRI){
			unsigned x;
			for(result=0, x=(unsigned)bbus;
						result<32; result++, x<<=1)
				if(x&MSB) break;
		} else {
			result = (unsigned)bbus;
		}
		{ int count = abus & 0x1f;
		  if(count){
			switch(ira.irshift.mode){
			   case B_ASR: {
				int signed = bbus & MSB;
				result = result >> count;
				if(signed) result |= ((1<<count)-1)
							<< (32-count);
				break;
			   }
			   case B_LSR:
				result = result >> count; break;
			   case B_LSL:
				result = result << count; break;
			   case B_ROL: {
				result = (result << count)
				       | (result >> (32-count));
				break;
			   }
			}
		  }
		}
	} else {
		result = (unsigned)bbus;	/* DEFAULT */
	}
	return((forthword)result);
}

void
Writereg(r)
int r;
{
	switch(r){
	   case B_S0R2:   pstack[(ptop)   & PSTACKSIZE-1] = bbus; break;
	   case B_S1R2:   pstack[(ptop-1) & PSTACKSIZE-1] = bbus; break;
	   case B_S2R2:   pstack[(ptop-2) & PSTACKSIZE-1] = bbus; break;
	   case B_S3R2:   pstack[(ptop-3) & PSTACKSIZE-1] = bbus; break;
	   case B_R0R2:   rstack[(rtop)   & RSTACKSIZE-1] = bbus; break;
	   case B_R1R2:   rstack[(rtop-1) & RSTACKSIZE-1] = bbus; break;
	   case B_R2R2:   rstack[(rtop-2) & RSTACKSIZE-1] = bbus; break;
	   case B_R3R2:   rstack[(rtop-3) & RSTACKSIZE-1] = bbus; break;
	   case B_IARR2:  break;		/* data sink */
	   case B_PSWR2:  psw.value = bbus & 0x3f;
			  cpselect = psw.fields.cpselect;
			  break;
	   case B_ZEROR2: break;		/* data sink */
	   case B_YR2:	  y = bbus; break;
	   default:
		switch(cpselect){
		   case 0:
			switch(r){
	   		   case B_U0R1: udr0 = bbus; break;
	   		   case B_U1R1: udr1 = bbus; break;
	   		   case B_U2R1: udr2 = bbus; break;
	   		   case B_U3R1: udr3 = bbus; break;
		   	   default:	Cmdmsg("Unknown cp0 register\n"); break;
			}
			break;
		   case 1:
			switch(r){
	   		   case B_U0R1:	externalrsp = bbus; break;
	   		   case B_U1R1:	externalpsp = bbus; break;
	   		   case B_U2R1: ssw.value = bbus;
					rtop = ssw.fields.rtop;
					rover = ssw.fields.rover;
					runder = (rover+4) & RSTACKSIZE-1;
					ptop = ssw.fields.ptop;
					pover = ssw.fields.pover;
					punder = (pover+4) & PSTACKSIZE-1;
					break;
		   	   default:	Cmdmsg("Unknown cp1 register\n"); break;
			}
			break;
		   case 2:
			switch(r){
			   case B_U0R1:	intmask = bbus; break;
			   case B_U1R1:	intmode = bbus; break;
			   case B_U2R1:	intedgedetected = bbus; break;
		   	   default:	Cmdmsg("Unknown cp2 register\n"); break;
			}
			break;
		   default:	Cmdmsg("Unknown coprocessor\n"); break;
		}
	}
}

void
Execcleanup()
{
	switch(ira.irbranch.type){
	   case B_COLON:
		irb.irvalue = portvalue;
		torptr = &aluout;
		writeback = Aluwritebacktor;
		break;
	   case B_BRANCH:
	   case B_QBRANCH:
		irb.irvalue = portvalue;
		torptr = &hold;
		writeback = Nop;
		break;
	   case B_LOAD:
	   case B_STORE:
		torptr = &hold;
		writeback = Nop;
		break;
	   case B_LAL:
	   case B_LAH:
		irb.irvalue = portvalue;
		torptr = (ira.irload.reg2 == B_R0R2) ? &aluout : &hold;
		writeback = Aluwriteback;
		break;
	   case B_MICRO:
		irb.irvalue = portvalue;
		if(ira.iralu.class == B_SHIFT){
			torptr = (ira.iralu.reg2 == B_R0R2) ? &barrelout :&hold;
			writeback = Shiftwriteback;
		} else {
			torptr = (ira.iralu.reg2 == B_R0R2) ? &aluout : &hold;
			writeback = Aluwriteback;
		}
		break;
	   default:
		Cmdmsg("Unknown instruction type");
		break;
	}
}

void
Aluwriteback()
{
	bbus = aluout;
	Writereg(oldir.irload.reg2);
}

void
Aluwritebacktor()
{
	rstack[rtop] = bbus = aluout;
}

void
Shiftwriteback()
{
	bbus = barrelout;
	Writereg(oldir.irload.reg2);
}


void
Clearstats()
{
	int i;

	cycle = 0;
	for(i=0; i<8; i++){
		instrtypes[i] = 0;
	}
	for(i=0; i<CYCLETYPES; i++){
		cycletypes[i] = 0;
	}
}

void
Summary()
{
	int i;

	printf("instruction types:\n");
	for(i=0; i<8; i++){
		printf("\t%9d %s\n",instrtypes[i],instrnames[i]);
	}
	printf("cycle types:\n");
	for(i=0; i<CYCLETYPES; i++){
		printf("\t%9d %s\n",cycletypes[i],cyclenames[i]);
	}
	printf("\t%9d total cycles\n",cycle);
}

void
Traceon()
{
	tracing = TRUE;
	Clearstats();
}

void
Traceoff()
{
	tracing = FALSE;
}
