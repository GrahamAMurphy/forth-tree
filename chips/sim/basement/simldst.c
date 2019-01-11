#include <stdio.h>
#include <a.out.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "simdefs.h"
#include "masks.h"
#include "globals.h"

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

forthword pstack[PSTACKSIZE];		/* parameter stack */
int ptop=PSTACKSIZE-1;
int pover;
int punder;

forthword rstack[RSTACKSIZE];		/* return stack */
int rtop=RSTACKSIZE-1;
int rover;
int runder;

forthword alua, alub;			/* ALU input registers */
forthword aluout;			/* ALU output register to abus */

union opstruct ir,			/* instruction register */
	       oldir;			/* previous ir for load, store */

forthword portaddr;			/* port address */
forthword portvalue;			/* port value */
int portio;				/* read/write operation */

int flag;				/* accumulator flag */

forthword abus,				/* bus A */
	  bbus,				/* bus B */
	  tbus;				/* bus T */

union pswformat psw;			/* processor status word */

forthword udr0, udr1, udr2, udr3;	/* user defined registers */
#define externalrsp udr0		/* udr0 is dedicated to the rstack */
#define externalpsp udr1		/* udr1 is dedicated to the pstack */

forthword pca, pcb;			/* program counter register pair */

int actualie;				/* actual interrupt enable, delayed
					   one instruction from ie in psw*/

forthword *mem;				/* memory array */

unsigned int maxaddr;			/* maximum memory address + 1 */

int cycle;				/* cycle number */
int phase;				/* phase number */

int quitflag;				/* set by terminate in fakeio */

static int stepmode = S_PHASE;		/* S_CYCLE: single cycle,
					   S_PHASE: single phase  */

static int intpending = FALSE;		/* set on external interrupt */
static int busrequest = FALSE;		/* bus request for DMA */

static int state = T_EXEC;		/* machine state */

static instrtypes[8];			/* count instruction types */
static loadusedmode;			/* load instruction with offset */
static storeusedmode;			/* store instruction with offset */
static cycletypes[CYCLETYPES];		/* count cycle types */

/*		Declarations of functions defined in this file */
extern void Loadobj();
extern void Cycle();
extern void Phase();
extern void Singlestep();
extern void Sim();
extern void Vsim();
extern void Resetregs();
extern void Startat();
extern void Stack();
extern void Waitcycle();
extern void Extint();
extern void Dmastart();
extern void Dmaend();
extern void Nextstate();
extern void Phia();
extern void Phib();
extern void Loadphia();
extern void Loadphib();
extern void Storephia();
extern void Storephib();
extern void Execphia();
extern void Execphib();
extern void Intphia();
extern void Intphib();
extern void Dmaphia();
extern void Dmaphib();
extern void Rsdecphia();
extern void Rsdecphib();
extern void Rsstorephia();
extern void Rsstorephib();
extern void Rsloadphia();
extern void Rsloadphib();
extern void Rsincphia();
extern void Rsincphib();
extern void Psdecphia();
extern void Psdecphib();
extern void Psstorephia();
extern void Psstorephib();
extern void Psloadphia();
extern void Psloadphib();
extern void Psincphia();
extern void Psincphib();
extern forthword Asrca();
extern forthword Bsrca();
extern forthword Tsrca();
extern forthword Readreg();
extern void Stackop();
extern forthword Alu();
extern forthword Microalu();
extern void Flag();
extern forthword Bsrcb();
extern void Bdest();
extern void Writereg();
extern void Clearstats();
extern void Summary();

void
Loadobj(fp)
FILE *fp;
{
	struct exec header;

	fread(&header,sizeof(header),1,fp);
	if(N_BADMAG(header)){		/* linear object code */
		struct stat buf;	/* file status buffer */

		fstat(fileno(fp),&buf);
		if((mem=(forthword *)malloc((int)buf.st_size)) == NULL){
			Cmdmsg("Malloc failed\n");
			return;
		}
		rewind(fp);
		maxaddr = (unsigned int)buf.st_size;
	} else {			/* otherwise it is in a.out format */
		if((mem=(forthword *)malloc(header.a_data)) == NULL){
			Cmdmsg("Malloc failed\n");
			return;
		}
		fseek(fp,(long)N_TXTOFF(header),0);
		maxaddr = (unsigned int)header.a_data;
	}
	fread(mem,maxaddr,1,fp);
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
	while((times!=0) & !quitflag){
		Phia(); Phib();
		if(pca == addr) times--;
	}
}

void
Simcycles(lastcycle)
forthword lastcycle;
{
	quitflag = FALSE;
	if(phase == 2) Phib();
	lastcycle += cycle;		/* from current cycle */
	while((cycle != lastcycle) & !quitflag){
		Phia(); Phib();
	}
}

void
Vsim(addr)
forthword addr;
{
	quitflag = FALSE;
	if(phase == 2) Phib();
	while((pca != addr) & !quitflag){
		Phia(); Phib();
		Updatescr();
	}
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
}

void
Startat(addr)
forthword addr;
{
	pca = portaddr = abus = addr;
	ir.irvalue = portvalue = mem[addr];
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
Extint()
{
	intpending = TRUE;
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

void
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

void
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
	if((unsigned)portaddr>0xffff0000){
		portvalue = Fakeio(portaddr);
	} else if((unsigned)portaddr>=maxaddr){
		IOread(portaddr,1);
		portvalue = 1;
	} else {
		portvalue = mem[portaddr];
	}
	bbus = portvalue;
	if(oldir.irload.offset != 0) loadusedmode++;
	Writereg(oldir.irload.reg2);
	Nextstate();
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	Storephia and Storephib handle the second cycle of load
 *	instructions.
 */

void
Storephia()
{
	portaddr = abus = aluout;
	portio = PORT_WR;
	portvalue = bbus = Readreg(oldir.irload.reg2);
	phase = 2;
}

void
Storephib()
{
	if((unsigned)portaddr>0xffff0000){
		Fakeioarg(portvalue);
	} else if((unsigned)portaddr>=maxaddr){
		IOwrite(portaddr,portvalue);
	} else {
		mem[portaddr] = portvalue;
	}
	if(oldir.irload.offset != 0) storeusedmode++;
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
	abus = Asrca();
	portio = PORT_RD;
	alub = bbus = Bsrca();
	alua = tbus = Tsrca();
	portaddr = abus;
	pca = abus;
	Stackop();
	phase = 2;
}

void
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
	ir.irvalue = portvalue = mem[portaddr];
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	Intphia and Intphib handle the interrupt acknowledge cycle.
 */

void
Intphia()
{
	portaddr = abus = 1;
	portio = PORT_RD;
	intpending = FALSE;
	psw.fields.ie = 0;
	phase = 2;
}

void
Intphib()
{
	Nextstate();
	ir.irvalue = portvalue = mem[portaddr];
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
	Nextstate();
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	The following functions handle the phase a and phase b hardware
 *	overflow and underflow operations
 */

void
Rsdecphia()
{
	portaddr = abus = pcb;
	portio = PORT_RD;
	alub = bbus = externalrsp;
	phase = 2;
}

void
Rsdecphib()
{
	if(++rover == RSTACKSIZE) rover=0;
	if(++runder == RSTACKSIZE) runder=0;
	externalrsp = bbus = aluout = alub-1;
	Nextstate();
	portvalue = mem[portaddr];
	phase = 1;
	cycle++;
}

void
Rsstorephia()
{
	portaddr = abus = externalrsp;
	portio = PORT_WR;
	portvalue = bbus = rstack[(rtop+1) & RSTACKSIZE-1];
	phase = 2;
}

void
Rsstorephib()
{
	mem[portaddr] = portvalue;
	Nextstate();
	phase = 1;
	cycle++;
}

void
Rsloadphia()
{
	portaddr = abus = externalrsp;
	portio = PORT_RD;
	phase = 2;
}

void
Rsloadphib()
{
	if(--rover == -1) rover=RSTACKSIZE-1;
	if(--runder == -1) runder=RSTACKSIZE-1;
	rstack[(rtop-4) & RSTACKSIZE-1] = bbus = portvalue = mem[portaddr];
	Nextstate();
	phase = 1;
	cycle++;
}

void
Rsincphia()
{
	portaddr = abus = pcb;
	portio = PORT_RD;
	alub = bbus = externalrsp;
	phase = 2;
}

void
Rsincphib()
{
	externalrsp = bbus = aluout = alub+1;
	Nextstate();
	portvalue = mem[portaddr];
	phase = 1;
	cycle++;
}

void
Psdecphia()
{
	portaddr = abus = pcb;
	portio = PORT_RD;
	alub = bbus = externalpsp;
	phase = 2;
}

void
Psdecphib()
{
	if(++pover == PSTACKSIZE) pover=0;
	if(++punder == PSTACKSIZE) punder=0;
	externalpsp = bbus = aluout = alub-1;
	Nextstate();
	portvalue = mem[portaddr];
	phase = 1;
	cycle++;
}

void
Psstorephia()
{
	portaddr = abus = externalpsp;
	portio = PORT_WR;
	portvalue = bbus = pstack[(ptop+1) & PSTACKSIZE-1];
	phase = 2;
}

void
Psstorephib()
{
	mem[portaddr] = portvalue;
	Nextstate();
	phase = 1;
	cycle++;
}

void
Psloadphia()
{
	portaddr = abus = externalpsp;
	portio = PORT_RD;
	phase = 2;
}

void
Psloadphib()
{
	if(--pover == -1) pover=PSTACKSIZE-1;
	if(--punder == -1) punder=PSTACKSIZE-1;
	pstack[(ptop-4) & PSTACKSIZE-1] = bbus = portvalue = mem[portaddr];
	Nextstate();
	phase = 1;
	cycle++;
}

void
Psincphia()
{
	portaddr = abus = pcb;
	portio = PORT_RD;
	alub = bbus = externalpsp;
	phase = 2;
}

void
Psincphib()
{
	externalpsp = bbus = aluout = alub+1;
	Nextstate();
	portvalue = mem[portaddr];
	phase = 1;
	cycle++;
}

/****************************************************************************
 *	The following functions handle everything that can happen on phase
 *	a in the EXEC state.
 */

forthword
Asrca()
{
	forthword result;

	switch(ir.irbranch.type){
	   case B_COLON:
	   case B_BRANCH:
		result = ir.irbranch.dest;
		break;
	   case B_QBRANCH:
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

forthword
Bsrca()
{
	forthword result;

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

forthword
Tsrca()
{
	forthword result;

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

forthword
Readreg(r)
int r;
{
	forthword tmp;
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

void
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

#ifdef MC68K
#define CIN(x)		(((x) & 1) << 4)
#else
#define CIN(x)		((x) & 1)
#endif

forthword
Alu()
{
	forthword result;

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

forthword
Microalu()
{
	int cin;			/* carry input */
	forthword result;

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
			result = result;
			break;
		   case B_DIV1:
			result = Addop(result,~alua,CIN(cin),&flags);
			break;
		   case B_DIV2:
			result = !flag ? Addop(alua,result,CIN(cin),&flags) :
					result;
			break;
		   case B_MUL:
			result = flag ? Addop(alua,result,CIN(cin),&flags) :
					result;
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

void
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

int
ALUcond()
{
	int result;

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

forthword
Bsrcb()
{
	forthword result;
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

void
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

void
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
	loadusedmode = storeusedmode = 0;
	for(i=0; i<CYCLETYPES; i++){
		cycletypes[i] = 0;
	}
}

void
Summary()
{
	int i;

	printf("instruction types:\n");
	for(i=0; i<4; i++){
		printf("\t%9d %s\n",instrtypes[i],instrnames[i]);
	}
	printf("\t%9d %s (%d with non-zero offset)\n",
		instrtypes[4],instrnames[4],loadusedmode);
	printf("\t%9d %s (%d with non-zero offset)\n",
		instrtypes[5],instrnames[5],storeusedmode);
	for(i=6; i<8; i++){
		printf("\t%9d %s\n",instrtypes[i],instrnames[i]);
	}
	printf("cycle types:\n");
	for(i=0; i<CYCLETYPES; i++){
		printf("\t%9d %s\n",cycletypes[i],cyclenames[i]);
	}
	printf("\t%9d total cycles\n",cycle);
}
