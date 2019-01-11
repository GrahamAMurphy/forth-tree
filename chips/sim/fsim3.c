#include <stdio.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "globals.h"
#include "external.h"
#include "intrnl3.h"
#include "frmts3.h"
#include "masks3.h"

#define USAGE "usage: fsim3 [-s] [-c fl] [-t fl] [-v fl] [-r x] [objfile]\n"

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

int32 pstack[PSTACKSIZE];		/* parameter stack */
int ptop=PSTACKSIZE-1;
int pover;
int punder;

int32 rstack[RSTACKSIZE];		/* return stack */
int rtop=RSTACKSIZE-1;
int rover;
int runder;

int32 alua, alub;			/* ALU input registers */
int32 aluout;				/* ALU output register to abus */

union opstruct ir,			/* instruction register */
	       oldir;			/* previous ir for load, store */

int32 portaddr;				/* port address */
int32 portvalue;			/* port value */
int portio;				/* read/write operation */

/* This variable is not used; it is here for compatability with FRISC 4 */
int busidle = FALSE;			/* bus idle; overrides portio */

int flag;				/* accumulator flag */

int32 abus,				/* bus A */
	  bbus,				/* bus B */
	  tbus;				/* bus T */

union pswformat psw;			/* processor status word */

int32 udr0, udr1, udr2, udr3;		/* user defined registers */
#define externalrsp udr0		/* udr0 is dedicated to the rstack */
#define externalpsp udr1		/* udr1 is dedicated to the pstack */

uint32 pca, pcb;			/* program counter register pair */

int actualie;				/* actual interrupt enable, delayed
					   one instruction from ie in psw*/

int32 cycle;				/* cycle number */
int phase;				/* phase number */

int quitflag;				/* set by terminate in fakeio */

static int32 *mem;			/* memory array */
static uint32 maxaddr = 0x10000;	/* maximum memory address + 1 */

static int stepmode = S_PHASE;		/* S_CYCLE: single cycle,
					   S_PHASE: single phase  */

static int intpending = FALSE;		/* set on external interrupt */
static int busrequest = FALSE;		/* bus request for DMA */

static int state = T_EXEC;		/* machine state */

static instrtypes[8];			/* count instruction types */
static cycletypes[CYCLETYPES];		/* count cycle types */

static int branchtaken;
static int branchback;

static int tracing = FALSE;		/* no trace by default */
static int visual = FALSE;		/* no automatic screen update */

static uint32 trapaddr;			/* simulate until this is reached */
static int traptimes;			/* ... this many times */

/*		Declarations of functions defined in this file */
static void Loadobj();
static void Nextstate();
static void Phia();
static void Phib();
static void Loadphia();
static void Loadphib();
static void Storephia();
static void Storephib();
static void Execphia();
static void Execphib();
static void Intphia();
static void Intphib();
static void Dmaphia();
static void Dmaphib();
static void Rsdecphia();
static void Rsdecphib();
static void Rsstorephia();
static void Rsstorephib();
static void Rsloadphia();
static void Rsloadphib();
static void Rsincphia();
static void Rsincphib();
static void Psdecphia();
static void Psdecphib();
static void Psstorephia();
static void Psstorephib();
static void Psloadphia();
static void Psloadphib();
static void Psincphia();
static void Psincphib();
static int32 Asrca();
static int32 Bsrca();
static int32 Tsrca();
static int32 Readreg();
static void Stackop();
static int32 Alu();
static int32 Microalu();
static void Flag();
static int32 Bsrcb();
static void Bdest();
static void Writereg();

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

	while((c=getopt(argc,argv,"c:r:st:v:")) != EOF){
		switch(c){
		   case 'c':
			if(Saveopen(optarg)==NULL){
				fprintf(stderr,
				   "Can't open save file %s\n",optarg);
				exit(1);
			}
			break;
		   case 'r':
			sscanf(optarg,"%lx",&maxaddr);
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
	Loadobj(fp);
	fclose(fp);
}

static void
Loadobj(fp)
FILE *fp;
{
	struct stat buf;	/* file status buffer */

	fstat(fileno(fp),&buf);
	if((uint32)buf.st_size > (maxaddr * sizeof(int32))){
		fprintf(stderr,"Too big; use -r to allocate more memory\n");
		exit(1);
	}
	if((mem=(int32 *)malloc(maxaddr*sizeof(int32))) == NULL){
		fprintf(stderr,"Malloc failed\n");
		exit(1);
	}
	fread(mem,(unsigned int)buf.st_size,1,fp);
}

void
Saveobj()
{
	Savecore(mem,maxaddr*sizeof(int32));
}

int32
Fetchexternal(addr)
uint32 addr;
{
	if(addr < maxaddr){
		return(mem[addr]);
	} else if((addr >= TRAPBASE) && (addr <= TRAPTOP)){
		return(Fakeio(addr-TRAPBASE));
	} else {
		Ioread(addr,1);
		return(1);
	}
}

int32
Fetchexternalbyte(addr)
uint32 addr;
{
	return(Fetchexternal(addr));		/* alias */
}

void
Storeexternal(addr,value)
uint32 addr;
int32 value;
{
	if(addr < maxaddr){
		mem[addr] = value;
	} else if((addr >= TRAPBASE) && (addr <= TRAPTOP)){
		Fakeioarg(value);
	} else {
		Iowrite(addr,value);
	}
}

void
Storeexternalbyte(addr,value)
uint32 addr;
int32 value;
{
	Storeexternal(addr,value);	/* alias */
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
	Updatescr();			/* always update; ignore visual */
}

void
Settrap(addr,times)
uint32 addr;
int times;
{
	trapaddr = addr;
	traptimes = times;
}

void
Sim(addr,times)
uint32 addr;
int times;
{
	Settrap(addr,times);
	quitflag = FALSE;
	if(phase == 2) Phib();
	while((traptimes!=0) && !quitflag){
		Phia(); Phib();
		if(visual) Updatescr();
		if(pca == trapaddr) traptimes--;
	}
	Updatescr();
}

void
Simcycles(cycles)
int cycles;
{
	quitflag = FALSE;
	if(phase == 2) Phib();
	while((--cycles >= 0) && !quitflag){
		Phia(); Phib();
		if(visual) Updatescr();
	}
	Updatescr();
}

void
Resetregs()
{
	state = T_EXEC;
	tbus = abus = bbus = 0;
	psw.fields.ptop = ptop = PSTACKSIZE-1;
	psw.fields.rtop = rtop = RSTACKSIZE-1;
	psw.fields.pover = pover = 0;
	punder = (pover+4) & PSTACKSIZE-1;
	psw.fields.rover = rover = 0;
	runder = (rover+4) & RSTACKSIZE-1;
	psw.fields.cache = 0;			/* disable cache */
	actualie = psw.fields.ie = 0;		/* disable interrupts */
	intpending = FALSE;
	busrequest = FALSE;
	ir.irvalue = 0;
	flag = 0;
	pcb = pca = portaddr = 0;
	portvalue = 0;
	portio = PORT_RD;
	cycle = 0;
	phase = 1;
	Startat(0);
}

void
Startat(addr)
uint32 addr;
{
	pca = portaddr = abus = addr;
	ir.irvalue = portvalue = Fetchexternal(addr);
	pcb = pca + 1;
	if(genvectors){
		Vecldphia();
		Vecldphib();
	}
	Updatescr();
}

void
Stack(num)
int32 num;
{
	if(++ptop==PSTACKSIZE) ptop=0;
	pstack[ptop] = num;
	Updatescr();
}

void
Waitcycle()
{
	cycle++;
}

void
Interrupton(n)
int n;					/* ignore: there is only one interrupt*/
{
	intpending = TRUE;
}

void
Interruptoff(n)
int n;					/* ignore: there is only one interrupt*/
{
	intpending = FALSE;
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
 *	Here is a model of the state machine the controls the chip
 */

static void
Nextstate()
{
	switch(state){
	   case T_INT:
		state = T_EXEC; break;
	   case T_GRANT:
		if(busrequest){
			break;
		} else if(intpending && actualie){
			state = T_INT;
		} else {
			state = T_EXEC;
		}
		break;
	   case T_EXEC:
		if(ir.irbranch.type == B_LOAD){
			state = T_LOAD;
		} else if(ir.irbranch.type == B_STORE){
			state = T_STORE;
		} else if(psw.fields.cache && rtop==rover) {
			state = T_RSDEC;
		} else if(psw.fields.cache && rtop==runder) {
			state = T_RSLOAD;
		} else if(psw.fields.cache && ptop==pover) {
			state = T_PSDEC;
		} else if(psw.fields.cache && ptop==punder) {
			state = T_PSLOAD;
		} else if(intpending && actualie) {
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
		if(psw.fields.cache && rtop==rover) {
			state = T_RSDEC;
		} else if(psw.fields.cache && rtop==runder) {
			state = T_RSLOAD;
		} else if(psw.fields.cache && ptop==pover) {
			state = T_PSDEC;
		} else if(psw.fields.cache && ptop==punder) {
			state = T_PSLOAD;
		} else if(intpending && actualie) {
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

static void
Phia()
{
	actualie = psw.fields.ie;			/* delayed ie */
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
		if(tracing) Logliteral(ir);
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
		Rsdecphia();
		if(genvectors) Vecldphia();
		break;
	   case T_RSSTORE:
		Rsstorephia();
		if(genvectors) Vecstphia();
		break;
	   case T_RSLOAD:
		Rsloadphia();
		if(genvectors) Vecldphia();
		break;
	   case T_RSINC:
		Rsincphia();
		if(genvectors) Vecldphia();
		break;
	   case T_PSDEC:
		Psdecphia();
		if(genvectors) Vecldphia();
		break;
	   case T_PSSTORE:
		Psstorephia();
		if(genvectors) Vecstphia();
		break;
	   case T_PSLOAD:
		Psloadphia();
		if(genvectors) Vecldphia();
		break;
	   case T_PSINC:
		Psincphia();
		if(genvectors) Vecldphia();
		break;
	}
}

static void
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
		Rsdecphib();
		if(genvectors) Vecldphib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	   case T_RSSTORE:
		Rsstorephib();
		if(genvectors) Vecstphib();
		if(tracing) Trace(A_DATA | A_STORE);
		break;
	   case T_RSLOAD:
		Rsloadphib();
		if(genvectors) Vecldphib();
		if(tracing) Trace(A_DATA | A_LOAD);
		break;
	   case T_RSINC:
		Rsincphib();
		if(genvectors) Vecldphib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	   case T_PSDEC:
		Psdecphib();
		if(genvectors) Vecldphib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	   case T_PSSTORE:
		Psstorephib();
		if(genvectors) Vecstphib();
		if(tracing) Trace(A_DATA | A_STORE);
		break;
	   case T_PSLOAD:
		Psloadphib();
		if(genvectors) Vecldphib();
		if(tracing) Trace(A_DATA | A_LOAD);
		break;
	   case T_PSINC:
		Psincphib();
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

static void
Loadphia()
{
	portaddr = abus = aluout;
	portio = PORT_RD;
	phase = 2;
}

static void
Loadphib()
{
	portvalue = Fetchexternal(portaddr);
	bbus = portvalue;
	Writereg(oldir.irload.reg2);
	Nextstate();
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	Storephia and Storephib handle the second cycle of load
 *	instructions.
 */

static void
Storephia()
{
	portaddr = abus = aluout;
	portio = PORT_WR;
	portvalue = bbus = Readreg(oldir.irload.reg2);
	phase = 2;
}

static void
Storephib()
{
	Storeexternal(portaddr, portvalue);
	Nextstate();
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	Execphia and Execphib handle the first cycle of all instructions.
 */

static void
Execphia()
{
	abus = Asrca();
	portio = PORT_RD;
	alub = bbus = Bsrca();
	alua = tbus = Tsrca();
	portaddr = abus;
	pca = abus;
	Stackop();
	phase = 2;
}

static void
Execphib()
{
	aluout = Alu();
	Flag();
	bbus = Bsrcb();
	Bdest();
	pcb = pca + 1;
	Nextstate();
	instrtypes[ir.irbranch.type]++;		/* statistics */
	oldir = ir;
	ir.irvalue = portvalue = Fetchexternal(portaddr);
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	Intphia and Intphib handle the interrupt acknowledge cycle.
 */

static void
Intphia()
{
	portaddr = abus = 1;
	portio = PORT_RD;
	intpending = FALSE;
	psw.fields.ie = 0;
	phase = 2;
}

static void
Intphib()
{
	Nextstate();
	ir.irvalue = portvalue = Fetchexternal(portaddr);
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	Dmaphia and Dmaphib handle DMA cycles.
 */

static void
Dmaphia()
{
	phase = 2;
}

static void
Dmaphib()
{
	Nextstate();
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	The following functions handle the phase a and phase b hardware
 *	overflow and underflow operations
 */

static void
Rsdecphia()
{
	portaddr = abus = pcb;
	portio = PORT_RD;
	alub = bbus = externalrsp;
	phase = 2;
}

static void
Rsdecphib()
{
	if(++rover == RSTACKSIZE) rover=0;
	if(++runder == RSTACKSIZE) runder=0;
	externalrsp = bbus = aluout = alub-1;
	Nextstate();
	portvalue = Fetchexternal(portaddr);
	phase = 1;
	cycle++;
}

static void
Rsstorephia()
{
	portaddr = abus = externalrsp;
	portio = PORT_WR;
	portvalue = bbus = rstack[(rtop+1) & RSTACKSIZE-1];
	phase = 2;
}

static void
Rsstorephib()
{
	Storeexternal(portaddr, portvalue);
	Nextstate();
	phase = 1;
	cycle++;
}

static void
Rsloadphia()
{
	portaddr = abus = externalrsp;
	portio = PORT_RD;
	phase = 2;
}

static void
Rsloadphib()
{
	if(--rover == -1) rover=RSTACKSIZE-1;
	if(--runder == -1) runder=RSTACKSIZE-1;
	rstack[(rtop-4) & RSTACKSIZE-1] = bbus = portvalue
		= Fetchexternal(portaddr);
	Nextstate();
	phase = 1;
	cycle++;
}

static void
Rsincphia()
{
	portaddr = abus = pcb;
	portio = PORT_RD;
	alub = bbus = externalrsp;
	phase = 2;
}

static void
Rsincphib()
{
	externalrsp = bbus = aluout = alub+1;
	Nextstate();
	portvalue = Fetchexternal(portaddr);
	phase = 1;
	cycle++;
}

static void
Psdecphia()
{
	portaddr = abus = pcb;
	portio = PORT_RD;
	alub = bbus = externalpsp;
	phase = 2;
}

static void
Psdecphib()
{
	if(++pover == PSTACKSIZE) pover=0;
	if(++punder == PSTACKSIZE) punder=0;
	externalpsp = bbus = aluout = alub-1;
	Nextstate();
	portvalue = Fetchexternal(portaddr);
	phase = 1;
	cycle++;
}

static void
Psstorephia()
{
	portaddr = abus = externalpsp;
	portio = PORT_WR;
	portvalue = bbus = pstack[(ptop+1) & PSTACKSIZE-1];
	phase = 2;
}

static void
Psstorephib()
{
	Storeexternal(portaddr, portvalue);
	Nextstate();
	phase = 1;
	cycle++;
}

static void
Psloadphia()
{
	portaddr = abus = externalpsp;
	portio = PORT_RD;
	phase = 2;
}

static void
Psloadphib()
{
	if(--pover == -1) pover=PSTACKSIZE-1;
	if(--punder == -1) punder=PSTACKSIZE-1;
	pstack[(ptop-4) & PSTACKSIZE-1] = bbus = portvalue
		= Fetchexternal(portaddr);
	Nextstate();
	phase = 1;
	cycle++;
}

static void
Psincphia()
{
	portaddr = abus = pcb;
	portio = PORT_RD;
	alub = bbus = externalpsp;
	phase = 2;
}

static void
Psincphib()
{
	externalpsp = bbus = aluout = alub+1;
	Nextstate();
	portvalue = Fetchexternal(portaddr);
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	The following functions handle everything that can happen on phase
 *	a in the EXEC state.
 */

static int32
Asrca()
{
	int32 result;

	switch(ir.irbranch.type){
	   case B_COLON:
	   case B_BRANCH:
		result = ir.irbranch.dest;
		break;
	   case B_QBRANCH:
		if(flag==0) branchtaken++;
		if(ir.irbranch.dest < pcb) branchback++;
		result = flag ? pcb : ir.irbranch.dest;
		break;
	   case B_LAL:
	   case B_LAH:
	   case B_LOAD:
	   case B_STORE:
	   case B_MICRO:
		result = (ir.irload.ret == B_RETURN) ? rstack[rtop] : pcb;
		break;
	   default:
		Cmdmsg("Uknown instruction type");
		break;
	}
	return(result);
}

static int32
Bsrca()
{
	int32 result;

	switch(ir.irbranch.type){
	   case B_COLON:
	   case B_BRANCH:			/* DEFAULT */
	   case B_QBRANCH:			/* DEFAULT */
		result = pcb;
		break;
	   case B_LAL:
	   case B_LAH:
	   case B_LOAD:
	   case B_STORE:
	   case B_MICRO:
		result = Readreg(ir.irload.reg1);
		break;
	   default:
		Cmdmsg("Unknown instruction type");
		break;
	}
	return(result);
}

static int32
Tsrca()
{
	int32 result;

	switch(ir.irbranch.type){
	   case B_LAL:
	   case B_LOAD:
	   case B_STORE:
		result = ir.irload.offset;
		break;
	   case B_LAH:
		result = ir.irload.offset<<16;
		break;
	   case B_COLON:			/* DEFAULT */
	   case B_BRANCH:			/* DEFAULT */
	   case B_QBRANCH:			/* DEFAULT */
	   case B_MICRO:
		result = pstack[ptop];
		break;
	   default:
		Cmdmsg("Unknown instruction type");
		break;
	}
	return(result);
}

static int32
Readreg(r)
int r;
{
	int32 tmp;
	/* WARNING: this code assumes that the encoding of the r1 and
	   r2 fields is identical! */
	switch(r){
	   case B_TOSR1:	tmp = pstack[(ptop)   & PSTACKSIZE-1]; break;
	   case B_SOSR1:	tmp = pstack[(ptop-1) & PSTACKSIZE-1]; break;
	   case B_ROSR1:	tmp = pstack[(ptop-2) & PSTACKSIZE-1]; break;
	   case B_FOSR1:	tmp = pstack[(ptop-3) & PSTACKSIZE-1]; break;
	   case B_TORR1:	tmp = rstack[(rtop)   & RSTACKSIZE-1]; break;
	   case B_SORR1:	tmp = rstack[(rtop-1) & RSTACKSIZE-1]; break;
	   case B_RORR1:	tmp = rstack[(rtop-2) & RSTACKSIZE-1]; break;
	   case B_FORR1:	tmp = rstack[(rtop-3) & RSTACKSIZE-1]; break;
	   case B_UDR0R1:	tmp = udr0; break;
	   case B_UDR1R1:	tmp = udr1; break;
	   case B_UDR2R1:	tmp = udr2; break;
	   case B_UDR3R1:	tmp = udr3; break;
	   case B_IARR1:	tmp = pcb; break;
	   case B_PSWR1:	psw.fields.ptop = ptop;
				psw.fields.pover = pover;
				psw.fields.rtop = rtop;
				psw.fields.rover = rover;
				tmp = psw.value; break;
	   case B_ZEROR1:	tmp = 0; break;
	   default:		Cmdmsg("Unknown B source\n"); break;
	}
	return(tmp);
}

static void
Stackop()
{
	switch(ir.irbranch.type){
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
		switch(ir.irload.pstack){
		   case B_NOPPS:	break;
		   case B_PUSHPS:	ptop++; ptop &= PSTACKSIZE-1; break;
		   case B_POPPS:	ptop--; ptop &= PSTACKSIZE-1; break;
		   default:		Cmdmsg("Unknown stack op\n"); break;
		}
	
		switch(ir.irload.rstack){
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

/****************************************************************************
 *	The following functions handle everything that can happen on phase
 *	b.
 */

int cout;				/* carry out after shift */
short flags;				/* XNZVC flags */
int shiftleftout;			/* output of left shifter */
int shiftrightout;			/* output of right shifter */

#define COUT(x)		((x))
#define OVFLOW(x)	((x)>>1)
#define EQUAL(x)	((x)>>2)
#define NEG(x)		((x)>>3)

#define CIN(x)		(((x) & 1) << 4)

static int32
Alu()
{
	int32 result;

	switch(ir.irbranch.type){
	   case B_COLON:
	   case B_BRANCH:
	   case B_QBRANCH:
		result = alub;
		break;
	   case B_LAL:
	   case B_LAH:
	   case B_LOAD:
	   case B_STORE:
		result = alua + alub;
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

static int32
Microalu()
{
	int cin;			/* carry input */
	int32 result;

	switch(ir.iralu.cin){
	   case B_ZERO:		cin = 0; break;
	   case B_ONE:		cin = 1; break;
	   case B_CFLAG:	cin = flag; break;
	   case B_CFLAGBAR:	cin = ~flag; break;
	   Default:		Cmdmsg("Unknown carry in\n"); break;
	}
	flags = 0;
	if(ir.iralu.alumode == B_ALU){
		switch(ir.iralu.aluop){
		   case B_ZEROS:	result = 0; break;
		   case B_ONES:		result = -1; break;
		   case B_NOTA:		result = ~alua; break;
		   case B_NOTB:		result = ~alub; break;
		   case B_NOPA:		result = alua; break;
		   case B_NOPB:		result = alub; break;
		   case B_ANANDB:	result = ~(alua & alub); break;
		   case B_ANORB:	result = ~(alua | alub); break;
		   case B_ANXORB:	result = ~(alua ^ alub); break;
		   case B_AANDB:	result = alua & alub; break;
		   case B_AORB:		result = alua | alub; break;
		   case B_AXORB:	result = alua ^ alub; break;
		   case B_AANDBBAR:	result = alua & ~alub; break;
		   case B_ABARANDB:	result = ~alua & alub; break;
		   case B_INCA:		result = Addop(alua,0,CIN(cin),&flags);
					break;
		   case B_INCB:		result = Addop(0,alub,CIN(cin),&flags);
					break;
		   case B_DECA:		result = Addop(alua,-1,CIN(cin),&flags);
					break;
		   case B_DECB:		result = Addop(-1,alub,CIN(cin),&flags);
					break;
		   case B_AMINUSB:	result = Addop(alua,~alub,CIN(cin),
						&flags);
					break;
		   case B_BMINUSA:	result = Addop(alub,~alua,CIN(cin),
						&flags);
					break;
		   case B_NEGA:		result = Addop(~alua,0,CIN(cin),
						&flags);
					break;
		   case B_NEGB:		result = Addop(0,~alub,CIN(cin),&flags);
					break;
		   case B_APLUSB:	result = Addop(alua,alub,CIN(cin),
						&flags);
					break;
		   default:		Cmdmsg("Unknown ALU operation\n");
					quitflag = TRUE;
					break;
		}
		if(result<0) flags |= 0x8;
		if(result==0) flags |= 0x4;
	} else {
		if(ir.irshift.shift == B_LEFT){
			shiftleftout = alub & 0x80000000;
			result = alub<<1;
			if(flag) result |= 0x1;
		} else {
			result = alub;
		}
		switch(ir.irshift.step){
		   case B_STEPB:
		   	result = Addop(result,0,CIN(cin),&flags);
			break;
		   case B_DIV1:
			result = Addop(result,~alua,CIN(cin),&flags);
			break;
		   case B_DIV2:
			result = Addop((!flag?alua:0),result,CIN(cin),&flags);
			break;
		   case B_MUL:
			result = Addop((flag?alua:0),result,CIN(cin),&flags);
			break;
		   default:
			Cmdmsg("Unknown step operation");
			break;
		}
		if(result<0) flags |= 0x8;
		if(result==0) flags |= 0x4;
		if(ir.irshift.shift == B_RIGHT){
			shiftrightout = result & 0x1;
			result >>= 1;
			if((ir.irshift.sin == B_SFLAG && flag) ||
			   (ir.irshift.sin == B_SALUCOND && ALUcond()))
				result |= 0x80000000;
			else
				result &= 0x7fffffff;
		}
	}
	return(result);
}

static void
Flag()
{
	if(ir.irbranch.type == B_MICRO &&
	   ir.iralu.flag){
		if(ir.iralu.alumode == B_SHIFT &&
		   ir.irshift.flagin == B_FLSHIFT){
			if(ir.irshift.shift == B_LEFT){
				flag = shiftleftout ? 1 : 0;
			} else {
				flag = shiftrightout ? 1 : 0;
			}
		} else {
			flag = ALUcond();
		}
	}
}

static int
ALUcond()
{
	int32 result;

	switch(ir.iralu.alucond){
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
	return(result & 0x1);			/* only lsb is flag */
}

static int32
Bsrcb()
{
	int32 result;
	if(ir.irbranch.type != B_MICRO){
		result = aluout;
	} else {
		if(ir.iralu.bsrc == B_DL){
			result = aluout;
		} else {
			result = flag ? -1 : 0;
		}
	}
	return(result);
}

static void
Bdest()
{
	switch(ir.irbranch.type){
	   case B_COLON:
		rstack[rtop] = bbus;
		break;
	   case B_BRANCH:
	   case B_QBRANCH:
	   case B_LOAD:
	   case B_STORE:
		break;
	   case B_LAL:
	   case B_LAH:
	   case B_MICRO:
		Writereg(ir.irload.reg2);
		break;
	   default:
		Cmdmsg("Unknown instruction type");
		break;
	}
}

static void
Writereg(r)
int r;
{
	switch(r){
	   case B_TOSR2:  pstack[(ptop)   & PSTACKSIZE-1] = bbus; break;
	   case B_SOSR2:  pstack[(ptop-1) & PSTACKSIZE-1] = bbus; break;
	   case B_ROSR2:  pstack[(ptop-2) & PSTACKSIZE-1] = bbus; break;
	   case B_FOSR2:  pstack[(ptop-3) & PSTACKSIZE-1] = bbus; break;
	   case B_TORR2:  rstack[(rtop)   & RSTACKSIZE-1] = bbus; break;
	   case B_SORR2:  rstack[(rtop-1) & RSTACKSIZE-1] = bbus; break;
	   case B_RORR2:  rstack[(rtop-2) & RSTACKSIZE-1] = bbus; break;
	   case B_FORR2:  rstack[(rtop-3) & RSTACKSIZE-1] = bbus; break;
	   case B_UDR0R2: udr0 = bbus; break;
	   case B_UDR1R2: udr1 = bbus; break;
	   case B_UDR2R2: udr2 = bbus; break;
	   case B_UDR3R2: udr3 = bbus; break;
	   case B_PSWR2:  psw.value = bbus & 0x3ffff;
			  rtop = psw.fields.rtop;
			  rover = psw.fields.rover;
			  runder = (rover+4) & RSTACKSIZE-1;
			  ptop = psw.fields.ptop;
			  pover = psw.fields.pover;
			  punder = (pover+4) & PSTACKSIZE-1;
			  break;
	   case B_NONER2: break;
	   default:	  Cmdmsg("Unknown B dest\n"); break;
	}
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
	branchtaken = branchback = 0;
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
	printf("\t%9d cond. branches taken\n", branchtaken);
	printf("\t%9d cond. branches backward\n", branchback);
}

void
Tracemode(enable)
int enable;
{
	tracing = enable;
	if(enable) Clearstats();
}

void
Vectormode(enable)
int enable;
{
	genvectors = enable;
}

void
Visualmode(enable)
int enable;
{
	visual = enable;
}
