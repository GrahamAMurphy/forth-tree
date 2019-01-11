#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "globals.h"
#include "external.h"
#include "intrnl4c.h"
#include "frmts4c.h"
#include "masks4c.h"

#define BIT(n)	(1<<(n))

#define USAGE \
"usage: fsim4 [-s] [-c fl] [-t fl] [-v fl] [-b] [-r x] [objfile]\n"

#define MSB 0x80000000			/* useful bit patterns */
#define LSB 0x00000001

#define TRUNCATE(x) ((x)&0xfffffffc)	/* generate aligned address */
#define BYTESEL(x)  ((x)&0x3)		/* extract byte select bits */

/* extract compressed branch address */
#ifdef BITS16
#define BRADDR(x)   (((x).irbranch.destms<<18) | ((x).irbranch.destls<<2))
#else
#define BRADDR(x)   ((x).irbranch.dest<<2)
#endif

#define A_INSTR 0L			/* instruction access */
#define A_DATA  0x80000000L		/* data access */
#define A_LOAD  0L			/* load access */
#define A_STORE 0x40000000L		/* store access */

#define T_INT 0				/* interrupt state */
#define T_EXEC 1			/* execute state */
#define T_LOAD 2			/* load state */
#define T_STORE 3			/* store state */
#define T_SQUASH 4			/* squash incorrectly taken branch */
#define T_ROVER1 5			/* return stack full, cycle 1 */
#define T_ROVER2 6			/* return stack full, cycle 2 */
#define T_RUNDER1 7			/* return stack fill, cycle 1 */
#define T_RUNDER2 8			/* return stack fill, cycle 2 */
#define T_POVER1 9			/* parameter stack full, cycle 1 */
#define T_POVER2 10			/* parameter stack full, cycle 2 */
#define T_PUNDER1 11			/* parameter stack fill, cycle 1 */
#define T_PUNDER2 12			/* parameter stack fill, cycle 2 */
#define CYCLETYPES 13			/* total number of cycles types */

static char *cyclenames[] = {		/* cycle names, for summary */
	"interrupt", "execute", "load", "store", "squash",
	"roverdec", "roverstore", "runderload", "runderinc",
	"poverdec", "poverstore", "punderload", "punderinc" };

static char *instrnames[] = {		/* instruction names, for summary */
	"flow", "loadbyte", "storebyte", "micro",
	"load", "store", "lal", "lah" };

union pswformat psw;			/* processor status word */

int32 pstack[PSTACKSIZE];		/* parameter stack */
int ptop=PSTACKSIZE-1;
int pover;
int punder;

int32 rstack[RSTACKSIZE];		/* return stack */
int rtop=RSTACKSIZE-1;
int rover;
int runder;

int32 hold;				/* return stack to abus anticipator */

int32 aluout;				/* ALU output register */

int32 barrelout;			/* barrel shifter output */

int32 y;				/* Y shift register */

union opstruct ir,			/* instruction register */
	       oldir;			/* previous ir writeback */
union opstruct irhold;			/* instruction holding register */

static int read;			/* read/writenot */
static int bytemode;			/* TRUE: byte access */

int32 portaddr;				/* port address */
int32 portvalue;			/* port value */
int portio;				/* read/write operation */

int32 dhold;				/* load holding register */

int flag;				/* accumulator flag */
static int flinput;			/* new flag value */

int32	abus,				/* bus A */
	dbus,				/* bus D */
	bbus,				/* bus B */
	tbus,				/* bus T */
	wbus;				/* bus W */

union sswformat ssw;			/* stack cache status word */
int32 externalrsp,			/* external return stack overflow */
	  externalpsp;			/* external parameter stack overflow */

int actualie;				/* actual interrupt enable */

int32 udr0, udr1, udr2, udr3;		/* user defined registers */

int32 pc;				/* program counter register pair */

int32 cycle;				/* number of execution cycles */
int32 stallcycle;			/* number of stall cycles */

int quitflag;				/* set by terminate in fakeio */

int32 *torptr;				/* who has TOR? */

static void (*writeback)();		/* register writeback function */

static unsigned char *lowmem;		/* low memory array */
static uint32 lowtop = 0x40000;		/* maximum memory address (default) */

static unsigned char *highmem;		/* high memory array */
#define hightop 0xffffffff		/* maximum memory address */
static uint32 highbottom = hightop;	/* base address */

/* Bus interface unit */
static int externalaccess;
int busidle = TRUE;			/* bus idle; overrides portio */
int biustall;				/* stall signals */
int cachestall = FALSE; /* TBD */

/* Direct-mapped Cache */
#define NCACHECELLS 1024		/* number of cells in cache RAM */
#define NSUBBLOCKBITS 2			/* size of sub-block select field */
#define NTAGBITS 8			/* size of tag select field */
					/* hackin' */
#define NTAGS (NCACHECELLS/(1<<NSUBBLOCKBITS))
#define SUBMASK (((1<<NSUBBLOCKBITS)-1)<<2)
#define SETMASK (((1<<NTAGBITS)-1) << (NSUBBLOCKBITS+2))
#define TAGMASK (~(SETMASK | ((1<<(NSUBBLOCKBITS+2))-1)))

static uint32 tagram[NTAGS];		/* cache tags and valid bits */
static int32 dataram[NCACHECELLS];	/* cached data */

#define INDEX(addr)\
	(((addr)&SETMASK)>>(NSUBBLOCKBITS+2))
#define SUBINDEX(addr)\
	(((addr)&SUBMASK)>>2)
#define BLOCKHIT(addr)\
	((tagram[INDEX(addr)] & TAGMASK) == ((addr)&TAGMASK))
#define SUBBLOCKPRESENT(addr)\
	(tagram[INDEX(addr)] & (1<<SUBINDEX(addr)))
#define ALLOCBLOCK(addr)\
	tagram[INDEX(addr)] = (((addr) & TAGMASK) | (1<<SUBINDEX(addr)))
#define ADDTOBLOCK(addr)\
	tagram[INDEX(addr)] |= (1<<SUBINDEX(addr))
#define DEALLOCBLOCK(addr)\
	tagram[INDEX(addr)] = ((addr)&TAGMASK)
#define CACHEDATA(addr)\
	dataram[((addr)&(SETMASK|SUBMASK))>>2]

/* internal boot ROM */
/* BUG: assumes that this array will be allocated at aligned address */
static unsigned char ibrom[] = {	/* internal boot ROM image */
#include "ibrom4c.h"
};
#define ibrombottom (ROMPAGE<<28)	/* bottom and top ROM addresses */
#define ibromtop    (ibrombottom+0x0fffffff)

/* internal memory-mapped I/O */
#define INTERNALIOBASE	(IOPAGE<<28)
#define INTERNALIOTOP	(INTERNALIOBASE+0x07ffffff)
#define TAGBACKDOORBASE	(INTERNALIOBASE+0x08000000)
#define TAGBACKDOORTOP	(INTERNALIOBASE+0x0fffffff)
#define IOATTR	(INTERNALIOBASE+(R_ATTR<<2)) /* attribute RAM */
#define IORSP	(INTERNALIOBASE+(R_RSP<<2))  /* return stack overflow pointer */
#define IOPSP	(INTERNALIOBASE+(R_PSP<<2))  /* param stack overflow pointer */
#define IOSCC	(INTERNALIOBASE+(R_SCC<<2))  /* stack cache controller */
#define IOMASK	(INTERNALIOBASE+(R_MASK<<2)) /* interrupt mask */
#define IOMODE	(INTERNALIOBASE+(R_MODE<<2)) /* interrupt mode */
#define IOEDGE	(INTERNALIOBASE+(R_EDGE<<2)) /* interrupt edge detector */
#define IOLEVL	(INTERNALIOBASE+(R_LEVL<<2)) /* interrupt level */

#define NATTRIBS (1<<NATTRIBBITS)
uint32 attributes[NATTRIBS];
#define WAITSPECIAL 0x3f
#define waits(addr)	(attrib(addr) & 0x3f)
#define cachable(addr)	(attrib(addr) & 0x40)
#define attrib(addr)	(attributes[((uint32)(addr))>>(32-NATTRIBBITS)])

/* interrupt controller */
static unsigned int intactive;		/* interrupt inputs */
       unsigned int intedgedetected;	/* edge detect latches */
       unsigned int intmask;		/* interrupt masks */
       unsigned int intmode;		/* interrupt modes: 0=level 1=edge */
       unsigned int intlevel;		/* interrupt level: 0=low 1=high */
static unsigned int intvector;		/* interrupt vector */

static int state, nextstate ;		/* machine state */

static instrtypes[8];			/* count instruction types */
static cycletypes[CYCLETYPES];		/* count execution cycle types */

static int tracing = FALSE;		/* no trace by default */
static int visual = FALSE;		/* no automatic screen update */

static uint32 trapaddr;			/* simulate until this is reached */
static int traptimes;			/* ... this many times */

/*		Declarations of functions defined in this file */
static void Nop();
static void Loadobj();
static void Loadbytepromobj();
static void Iodecode();
static void Biu();
static void Startexternal();
static void Endexternal();
static int32 Fetchexternal();
static void Storeexternal();
static int32 Internaliofetch();
static void Internaliostore();
static int Intsuppress();
static int Interruptpending();
static int Interruptvector();
static void Nextstate();
static void Phia();
static void Phib();
static void Dpphia();
static void Dpphib();
static void Loadphia();
static void Loadphib();
static void Loadwriteback();
static void Storephia();
static void Storephib();
static void Execphia();
static void Execphib();
static void Squashphia();
static void Squashphib();
static void Intphia();
static void Intphib();
static void Rsover1phia();
static void Rsover1phib();
static void Rsover2phia();
static void Rsover2phib();
static void Rsunder1phia();
static void Rsunder1phib();
static void Rsunder2phia();
static void Rsunder2phib();
static void Rsunderflowwriteback();
static void Psover1phia();
static void Psover1phib();
static void Psover2phia();
static void Psover2phib();
static void Psunder1phia();
static void Psunder1phib();
static void Psunder2phia();
static void Psunder2phib();
static void Psunderflowwriteback();
static int32 Asrc();
static int32 Bsrc();
static int32 Tsrc();
static void Stackop();
static int32 Readreg();
static int32 Alu();
static int32 Microalu();
static int32 Booth1();
static int32 Booth2();
static int32 Barrel();
static int32 Microbarrel();
static int Flag();
static void Writereg();
static void Execcleanup();
static void Aluwriteback();
static void Aluwritebacktor();
static void Aluwritebacktos();
static void Shiftwriteback();

static void
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
			sscanf(optarg,"%lx",&lowtop);
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
	if((fp=fopen(objcode,"rb")) == NULL){
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
	if((uint32)buf.st_size > lowtop){
		fprintf(stderr,"Too big; use -r to allocate more memory\n");
		exit(1);
	}
	if((lowmem=(unsigned char *)malloc((unsigned int)lowtop)) == NULL){
		fprintf(stderr,"Malloc failed (in Loadobj)\n");
		exit(1);
	}
	fread((void *)lowmem,buf.st_size,1,fp);
}

static void
Loadbytepromobj(fp)
FILE *fp;
{
	struct stat buf;	/* file status buffer */
	unsigned char *mem;
	int c;

	fstat(fileno(fp),&buf);
	if((uint32)buf.st_size > lowtop){
		fprintf(stderr,"Too big; use -r to allocate more memory\n");
		exit(1);
	}

	/* allocate high memory for external boot PROM */
	if((highmem=(unsigned char *)malloc((unsigned int)(4*buf.st_size)))
				== NULL){
		fprintf(stderr,"Malloc failed (in Loadbytepromobj.1)\n");
		exit(1);
	}
	highbottom = hightop + 1 - 4*buf.st_size;
	mem = highmem;
	while((c=getc(fp)) != EOF){
		*mem++ = (unsigned char)c;
		*mem++ = 0; *mem++ = 0; *mem++ = 0;
	}

	/* allocate low memory too */
	if((lowmem=(unsigned char *)malloc((unsigned int)lowtop)) == NULL){
		fprintf(stderr,"Malloc failed (Loadbytepromobj.2)\n");
		exit(1);
	}
}

void
Saveobj()
{
/* TBD should save in proper boot ROM format */
	Savecore(lowmem, lowtop+1);
}

/****************************************************************************
 *	Here is a model of the on-chip and off-chip I/O system.
 */

static void
Iodecode()
{
	static enum {local, newblock, newdata,
			replacedata, replacedatawait, invalidate}
		state = local, newstate = local;
	static int32 oldcachedata, wrcachemask;

	state = newstate;
	/* defaults: may be changed later in routine */
	cachestall = FALSE;
	externalaccess = FALSE;
	newstate = local;
	switch(state){
	   case local:
		if((abus<=lowtop) ||
		   ((abus>=highbottom) && (abus<=hightop))){
			if
			   /* If cache hit */
			   (psw.fields.cacheenabled && cachable(abus) &&
			   read && BLOCKHIT(abus) && SUBBLOCKPRESENT(abus)){
				Biu();
				dbus = CACHEDATA(abus);
#ifdef DEBUG
printf("Hit   %x %x tagram[%x] = %x",abus,dbus,INDEX(abus),tagram[INDEX(abus)]);
printf(" dataram[%x] = %x\n",((abus)&(SETMASK|SUBMASK))>>2,CACHEDATA(abus));
#endif
			} else if
			  /* if cache read miss: allocate a new block */
			  (psw.fields.cacheenabled && cachable(abus) &&
			   read && !BLOCKHIT(abus)){
				cachestall = TRUE;
				externalaccess = TRUE;
				Biu();
				dbus = portvalue;
				newstate = newblock;
			} else if
			  /* if cache read subblock miss: load subblock */
			  (psw.fields.cacheenabled && cachable(abus) &&
			   read && BLOCKHIT(abus) &&!SUBBLOCKPRESENT(abus)){
				cachestall = TRUE;
				externalaccess = TRUE;
				Biu();
				dbus = portvalue;
				newstate = newdata;
			} else if
			  /* if cache write hit: replace subblock */
			  (psw.fields.cacheenabled && cachable(abus) &&
			    !read&& BLOCKHIT(abus) && SUBBLOCKPRESENT(abus)){
				cachestall = TRUE;
				externalaccess = TRUE;
				Biu();
				oldcachedata = CACHEDATA(abus);
				wrcachemask = bytemode
					    ? 0xff<<(((~abus)&0x3)*8)
					    : ~0;
				if(biustall)	newstate = replacedatawait;
				else		newstate = replacedata;
			/* external access, write miss, etc. */
			} else {
				externalaccess = TRUE;
				Biu();
				if(read) dbus = portvalue;
			}
		} else if((abus >= TRAPBASE) && (abus <= TRAPTOP)){
			Biu();
			if(read) dbus = Fakeio((int32)abus - TRAPBASE);
			else Fakeioarg(dbus);
		} else if((abus >= INTERNALIOBASE) && (abus <= INTERNALIOTOP)){
			Biu();
			if(read) dbus = Internaliofetch(abus);
			else (void)Internaliostore(abus, dbus);
		} else if((abus >= TAGBACKDOORBASE)&&(abus <= TAGBACKDOORTOP)){
			cachestall = TRUE;
			Biu();
			newstate = invalidate;
			if(read) dbus = portvalue;
		} else if(psw.fields.ibrenable &&
		  	(abus >= ibrombottom) &&
		  	(abus <= ibromtop)){
#ifdef LITTLEENDIAN
			unsigned char *m = (unsigned char *)(ibrom +
							     TRUNCATE(abus) -
							     ibrombottom);
			Biu();
			if(read) dbus =	(int32)(*m)<<24 | (int32)(*(m+1))<<16 |
					(int32)(*(m+2))<<8 | (int32)(*(m+3));
#else
			Biu();
			if(read) dbus = *(int32 *)(ibrom + TRUNCATE(abus)
							 - ibrombottom);
#endif
		} else {
			Biu();
			if(read){
				Ioread(abus,1L);
				dbus = 1;
			} else {
				Iowrite(abus, dbus);
			}
		}
		break;

	   case newblock:	/* allocate a block: only happens on read miss*/
		externalaccess = TRUE;
		ALLOCBLOCK(abus);
		Biu();
		if(biustall)	newstate = newblock;
		else		newstate = local;
		CACHEDATA(abus) = dbus = portvalue;
#ifdef DEBUG
if(newstate==local)printf("Alloc %x %x tagram[%x] = %x",abus,dbus,INDEX(abus),tagram[INDEX(abus)]);
printf(" dataram[%x] = %x\n",((abus)&(SETMASK|SUBMASK))>>2,CACHEDATA(abus));
#endif
		break;

	   case newdata:
		externalaccess = TRUE;
		ADDTOBLOCK(abus);
		Biu();
		if(biustall)	newstate = newdata;
		else		newstate = local;
		CACHEDATA(abus) = dbus = portvalue;
#ifdef DEBUG
if(newstate==local)printf("Addto %x %x tagram[%x] = %x",abus,dbus,INDEX(abus),tagram[INDEX(abus)]);
printf(" dataram[%x] = %x\n",((abus)&(SETMASK|SUBMASK))>>2,CACHEDATA(abus));
#endif
		break;

	   case replacedata:
		ADDTOBLOCK(abus);
		Biu();
		CACHEDATA(abus) = (dbus & wrcachemask)
				 | (oldcachedata & ~wrcachemask);
		newstate = local;
		break;

	   case replacedatawait:
		externalaccess = TRUE;
		ADDTOBLOCK(abus);
		Biu();
		if(biustall)	newstate = replacedatawait;
		else		newstate = local;
		CACHEDATA(abus) = (dbus & wrcachemask)
				 | (oldcachedata & ~wrcachemask);
		break;

	   case invalidate:
		DEALLOCBLOCK(abus);
		Biu();
		newstate = local;
#ifdef DEBUG
if(newstate==local)printf("Deall %x %x tagram[%x] = %x",abus,dbus,INDEX(abus),tagram[INDEX(abus)]);
printf(" dataram[%x] = %x\n",((abus)&(SETMASK|SUBMASK))>>2,CACHEDATA(abus));
#endif
		break;
	}
}

static void
Biu()
{
	static enum {first, readcount, writecount, idle}
		state = idle, newstate = idle;
	static unsigned int count;

/*	switch(state){
	   case first: Cmdmsg("first\n"); break;
	   case readcount: Cmdmsg("readcount\n"); break;
	   case writecount: Cmdmsg("writecount\n"); break;
	   case idle: Cmdmsg("idle\n"); break;
	}*/
	state = newstate;
	/* defaults: may be changed later in routine */
	biustall = FALSE;
	switch(state){
	   case first:
		Startexternal();
		if(externalaccess){
			if(read){
				if(waits(abus) != WAITSPECIAL){
					count = waits(abus) + 1;
					biustall = TRUE;
					newstate = readcount;
				} else {
					Endexternal();
					newstate = first;
				}
			} else /* write */ {
				if(waits(abus) != WAITSPECIAL){
					count = waits(abus) + 1;
					newstate = writecount;
				} else {
					Endexternal();
					newstate = first;
				}
			}
		} else /* !externalaccess */ {
			newstate = idle;
		}
		break;

	   case readcount:
		if(--count){
			biustall = TRUE;
			newstate = readcount;
		} else {
			Endexternal();
			newstate = first;
		}
		break;

	   case writecount:
		if(--count){
			if(!externalaccess){
				newstate = writecount;
			} else {
				biustall = TRUE;
				newstate = writecount;
			}
		} else {
			if(externalaccess){
				biustall = TRUE;
			}
			Endexternal();
			newstate = first;
		}
		break;

	   case idle:
		busidle = TRUE;
		if(externalaccess){
			biustall = TRUE;
			newstate = first;
		} else {
			newstate = idle;
		}
		break;
	}
}

static void
Startexternal()
{
	portaddr = abus;
	busidle = FALSE;
	portio = read ? (bytemode ? PORT_RDBYTE : PORT_RD) :
			(bytemode ? PORT_WRBYTE : PORT_WR);
	if(!read) portvalue = dbus;
}

static void
Endexternal()
{
	if(portio == PORT_WR) Storeexternal(portaddr, portvalue);
	else if(portio == PORT_WRBYTE) Storeexternalbyte(portaddr, portvalue);
	else portvalue = Fetchexternal(portaddr);
}

static int32
Fetchexternal(addr)
uint32 addr;
{
	if(addr <= lowtop){
#ifdef LITTLEENDIAN			/* simulate big endian */
		unsigned char *m = (unsigned char *)(lowmem + TRUNCATE(addr));
		return( (int32)(*m)<<24 | (int32)(*(m+1))<<16 |
			(int32)(*(m+2))<<8 | (int32)(*(m+3)));
#else
		return( *(int32 *)(lowmem + TRUNCATE(addr)));
#endif
	} else if(psw.fields.ibrenable && /* redundant? */
		  (abus >= ibrombottom) &&
		  (abus <= ibromtop)){
#ifdef LITTLEENDIAN
		unsigned char *m = (unsigned char *)(ibrom +
							     TRUNCATE(abus) -
							     ibrombottom);
		return( (int32)(*m)<<24 | (int32)(*(m+1))<<16 |
			(int32)(*(m+2))<<8 | (int32)(*(m+3)));
#else
		return( *(int32 *)(ibrom + TRUNCATE(abus) - ibrombottom));
#endif
	} else if((addr >= highbottom) &&
		  (addr <= hightop)){
#ifdef LITTLEENDIAN
		unsigned char *m = (unsigned char *)(highmem + TRUNCATE(addr)
					- highbottom);
		return( (int32)(*m)<<24 | (int32)(*(m+1))<<16 |
			    (int32)(*(m+2))<<8 | (int32)(*(m+3)));
#else
		return( *(int32 *)(highmem + TRUNCATE(addr) - highbottom));
#endif
	} else {
		Cmdmsg("Illegal fetch\n");
		return(0);
	}
}

int32
Fetchexternalbyte(addr)
uint32 addr;
{
	if(addr <= lowtop){
		return(*(unsigned char *)(lowmem + addr));
	} else if((addr >= highbottom) &&
		  (addr <= hightop)){
		return(*(unsigned char *)(highmem + addr - highbottom));
	} else if(psw.fields.ibrenable && /* redundant? */
		  (addr >= ibrombottom) &&
		  (addr <= ibromtop)){
		return(*(unsigned char *)(ibrom + addr - ibrombottom));
	} else {
		Cmdmsg("Illegal byte fetch\n");
		return(0);
	}
}

static void
Storeexternal(addr,value)
uint32 addr;
int32 value;
{
	if(addr <= lowtop){
#ifdef LITTLEENDIAN
		unsigned char *m = (unsigned char *)(lowmem + TRUNCATE(addr));
		*m =	 (unsigned char)(value>>24);
		*(m+1) = (unsigned char)(value>>16);
		*(m+2) = (unsigned char)(value>>8);
		*(m+3) = (unsigned char)(value);
#else
		*(int32 *)(lowmem + TRUNCATE(addr)) = value;
#endif
	} else if((addr >= highbottom) &&
		  (addr <= hightop)){
#ifdef LITTLEENDIAN
		unsigned char *m = (unsigned char *)(highmem +
						     TRUNCATE(addr)
						     - highbottom);
		*m =	 (unsigned char)(value>>24);
		*(m+1) = (unsigned char)(value>>16);
		*(m+2) = (unsigned char)(value>>8);
		*(m+3) = (unsigned char)(value);
#else
		*(int32 *)(highmem + TRUNCATE(addr) -
				     highbottom) = value;
#endif
	} else {
		Cmdmsg("Illegal store\n");
	}
}

void
Storeexternalbyte(addr,value)
uint32 addr;
int32 value;
{
	if(addr <= lowtop){
		*(unsigned char *)(lowmem + addr) = (unsigned char)value;
	} else if((addr >= highbottom) &&
		  (addr <= hightop)){
		*(unsigned char *)(highmem + addr - highbottom) =
			(unsigned char)value;
	} else {
		Cmdmsg("Illegal byte store\n");
	}
}

static int32
Internaliofetch(addr)
uint32 addr;
{
	int32 tmp;

	if((addr>=IOATTR) && (addr<(IOATTR+(NATTRIBS<<2)))){
		tmp = *(attributes + (TRUNCATE(addr) - IOATTR)/sizeof(int32));
	} else {
		switch(addr){
		   case IORSP:	tmp = externalrsp; break;
		   case IOPSP:	tmp = externalpsp; break;
		   case IOSCC:	ssw.fields.ptop = ptop;
				ssw.fields.pover = pover;
				ssw.fields.rtop = rtop;
				ssw.fields.rover = rover;
				tmp = ssw.value;
				break;
		   case IOMASK:	tmp = intmask; break;
		   case IOMODE:	tmp = intmode; break;
		   case IOEDGE:	tmp = intedgedetected; break;
		   case IOLEVL:	tmp = intlevel; break;
		   default:	Cmdmsg("Unknown I/O device\n"); break;
		}
	}
	return(tmp);
}

static void
Internaliostore(addr, value)
uint32 addr;
int32 value;
{
	if((addr>=IOATTR) && (addr<(IOATTR+(NATTRIBS<<2)))){
		*(attributes + (TRUNCATE(addr) - IOATTR)/sizeof(int32)) = value;
	} else {
		switch(addr){
		   case IORSP:	externalrsp = value; break;
		   case IOPSP:	externalpsp = value; break;
		   case IOSCC:	ssw.value = value;
				rtop = ssw.fields.rtop;
				rover = ssw.fields.rover;
				runder = (rover+4) & RSTACKSIZE-1;
				ptop = ssw.fields.ptop;
				pover = ssw.fields.pover;
				punder = (pover+4) & PSTACKSIZE-1;
				break;
		   case IOMASK:	intmask = value; break;
		   case IOMODE:	intmode = value; break;
		   case IOEDGE:	intedgedetected = value; break;
		   case IOLEVL:	intlevel = value; break;
		   default:	Cmdmsg("Unknown I/O device\n"); break;
		}
	}
}

void
Cycle()
{
	/* Always in single cycle mode */
}

void
Phase()
{
	/* Obsolete */
}

void
Singlestep()
{
	Phia(); Phib();
	Updatescr();			/* update screen, ignore visual */
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
	while((traptimes!=0) && !quitflag){
		Phia(); Phib();
		if(visual) Updatescr();
		if(pc == trapaddr+4) traptimes--;
	}
	Updatescr();
}

void
Simcycles(cycles)
int cycles;
{
	quitflag = FALSE;
	while((--cycles >= 0) && !quitflag){
		Phia(); Phib();
		if(visual) Updatescr();
	}
	Updatescr();
}

void
Resetregs()
{
	state = nextstate = T_EXEC;
	dbus = abus = bbus = tbus = wbus = 0;
	ssw.fields.ptop = ptop = PSTACKSIZE-1;
	ssw.fields.rtop = rtop = RSTACKSIZE-1;
	ssw.fields.pover = pover = 0;
	punder = (pover+4) & PSTACKSIZE-1;
	ssw.fields.rover = rover = 0;
	runder = (rover+4) & RSTACKSIZE-1;
	ssw.fields.rcache = ssw.fields.pcache = 0; /* disable caches */
	actualie = psw.fields.ie = 0;		/* disable interrupts */
	psw.fields.ibrenable = 1;		/* enable internal boot ROM */
	psw.fields.cacheenabled = 0;		/* disable cache */
	intactive = 0;				/* no interrupt requests */
	ir.irvalue = irhold.irvalue = 0;
	y = 0;
	flag = 0;
	pc = portaddr = 0;
	portvalue = 0;
	portio = PORT_RD;
	read = TRUE; bytemode = FALSE;
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Nop;
	cycle = 0; stallcycle = 0;
	Startat((uint32)ibrombottom);
}

void
Startat(addr)
uint32 addr;
{
	pc = abus = addr;
	Iodecode();
#if 0
	ir.irvalue = dbus = portvalue = Fetchexternal(addr);
	pc = abus + 4;
	if(genvectors){
		/*Vecldphia();*/
		Vecldphib();
	}
#endif
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
Dmastart()
{
	/* not supported */
}

void
Dmaend()
{
	/* not supported */
}

/****************************************************************************
 *	Here is a model of the interrupt controller
 */

/* Interrupt line goes low.  In default active-low mode, this interrupts */
void
Interrupton(n)
int n;
{
	unsigned int mask = BIT(n);

	if((intlevel & mask) == 0){	/* if active low mode */
					intedgedetected |= (~intactive) & mask;
					intactive |=  mask;
	} else {
					intactive &= ~mask;
	}
	if(genvectors) Vecinton(n);
}

/* Interrupt line goes high.  In default active-low mode, this stops interrupt*/
void
Interruptoff(n)
int n;
{
	unsigned int mask = BIT(n);

	if((intlevel & mask) != 0){	/* if active high mode */
					intedgedetected |= (~intactive) & mask;
					intactive |=  mask;
	} else {
					intactive &= ~mask;
	}
	if(genvectors) Vecintoff(n);
}

static int
Intsuppress()
{
	return(((state==T_EXEC) && ( (ir.irbranch.type==B_MICRO)
				  || (ir.irbranch.type==B_LAH)
				  || (ir.irbranch.type==B_LAL))
			 && (ir.irload.reg2==B_PSW))
	||	((state==T_LOAD) && (oldir.irload.reg2==B_PSW)));
}

static int
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

static int
Interruptvector()
{
	intedgedetected &= ~BIT(intvector);
	return(intvector);
}

/****************************************************************************
 *	Here is a model of the state machine the controls the chip
 */

static void
Nextstate()
{
	switch(state){
	   case T_EXEC:
		if(ir.irbranch.type == B_LOAD){
			nextstate = T_LOAD;
		} else if(ir.irbranch.type == B_LOADBYTE){
			nextstate = T_LOAD;
		} else if(ir.irbranch.type == B_STORE){
			nextstate = T_STORE;
		} else if(ir.irbranch.type == B_STOREBYTE){
			nextstate = T_STORE;
		} else if((ir.irbranch.type == B_FLOW) &&
			  (ir.irbranch.brtype == B_QBRANCH) &&
			  flag) {
			nextstate = T_SQUASH;
		} else if(ssw.fields.rcache && rtop==rover) {
			nextstate = T_ROVER1;
		} else if(ssw.fields.rcache && rtop==runder) {
			nextstate = T_RUNDER1;
		} else if(ssw.fields.pcache && ptop==pover) {
			nextstate = T_POVER1;
		} else if(ssw.fields.pcache && ptop==punder) {
			nextstate = T_PUNDER1;
		} else if(actualie && !Intsuppress() && Interruptpending()) {
			nextstate = T_INT;
		} else {
			nextstate = T_EXEC;
		}
		break;
	   case T_INT:
	   case T_ROVER2:
	   case T_RUNDER2:
	   case T_POVER2:
	   case T_PUNDER2:
	   case T_LOAD:
	   case T_STORE:
	   case T_SQUASH:
		if(ssw.fields.rcache && rtop==rover) {
			nextstate = T_ROVER1;
		} else if(ssw.fields.rcache && rtop==runder) {
			nextstate = T_RUNDER1;
		} else if(ssw.fields.pcache && ptop==pover) {
			nextstate = T_POVER1;
		} else if(ssw.fields.pcache && ptop==punder) {
			nextstate = T_PUNDER1;
		} else if(actualie && !Intsuppress() && Interruptpending()) {
			nextstate = T_INT;
		} else {
			nextstate = T_EXEC;
		}
		break;
	   case T_ROVER1:
		nextstate = T_ROVER2; break;
	   case T_RUNDER1:
		nextstate = T_RUNDER2; break;
	   case T_POVER1:
		nextstate = T_POVER2; break;
	   case T_PUNDER1:
		nextstate = T_PUNDER2; break;
	}
}

/****************************************************************************
 *	Phia and Phib handle the phase a and phase b of the current
 *	instruction respectively.
 */

static void
Phia()
{
	if(!(biustall || cachestall)) Dpphia();
	Iodecode();
}

static void
Phib()
{
	if(!(biustall || cachestall)) Dpphib();
	else stallcycle++;
	if(genvectors){
		if((portio==PORT_RD) || (portio==PORT_RDBYTE)) Vecldphib();
		else	Vecstphib();
	}
}

static void
Dpphia()
{
	state = nextstate;
	(*writeback)();			/* write back result of previous cycle*/
	switch(state){
	   case T_INT:
		Intphia();
		break;
	   case T_EXEC:
		Execphia();
		if(tracing) Logliteral(ir);
		break;
	   case T_LOAD:
		Loadphia();
		break;
	   case T_STORE:
		Storephia();
		break;
	   case T_SQUASH:
		Squashphia();
		break;
	   case T_ROVER1:
		Rsover1phia();
		break;
	   case T_ROVER2:
		Rsover2phia();
		break;
	   case T_RUNDER1:
		Rsunder1phia();
		break;
	   case T_RUNDER2:
		Rsunder2phia();
		break;
	   case T_POVER1:
		Psover1phia();
		break;
	   case T_POVER2:
		Psover2phia();
		break;
	   case T_PUNDER1:
		Psunder1phia();
		break;
	   case T_PUNDER2:
		Psunder2phia();
		break;
	}
	Nextstate();
}

static void
Dpphib()
{
	switch(state){
	   case T_INT:
		Intphib();
		break;
	   case T_EXEC:
		Execphib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	   case T_LOAD:
		Loadphib();
		if(tracing) Trace(A_DATA | A_LOAD);
		break;
	   case T_STORE:
		Storephib();
		if(tracing) Trace(A_DATA | A_STORE);
		break;
	   case T_SQUASH:
		Squashphib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	   case T_ROVER1:
		Rsover1phib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	   case T_ROVER2:
		Rsover2phib();
		if(tracing) Trace(A_DATA | A_STORE);
		break;
	   case T_RUNDER1:
		Rsunder1phib();
		if(tracing) Trace(A_DATA | A_LOAD); /* TBD INSTR? */
		break;
	   case T_RUNDER2:
		Rsunder2phib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	   case T_POVER1:
		Psover1phib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	   case T_POVER2:
		Psover2phib();
		if(tracing) Trace(A_DATA | A_STORE);
		break;
	   case T_PUNDER1:
		Psunder1phib();
		if(tracing) Trace(A_DATA | A_LOAD);
		break;
	   case T_PUNDER2:
		Psunder2phib();
		if(tracing) Trace(A_INSTR | A_LOAD);
		break;
	}
	cycle++;
	cycletypes[state]++;
}

/****************************************************************************
 *	Loadphia and Loadphib handle the second cycle of load
 *	instructions.
 */

static void
Loadphia()
{
	abus = aluout;
	read = TRUE;
	bytemode = oldir.irload.type == B_LOADBYTE;
}

static void
Loadphib()
{
	if(oldir.irload.type == B_LOADBYTE){
		switch(BYTESEL(abus)){	/* big-endian */
		   case 0:	dhold = (dbus>>24) & 0xff; break;
		   case 1:	dhold = (dbus>>16) & 0xff; break;
		   case 2:	dhold = (dbus>>8)  & 0xff; break;
		   case 3:	dhold = (dbus)     & 0xff; break;
		}
	} else {
		dhold = dbus;
	}
	writeback = Loadwriteback;
	hold = rstack[rtop];
	torptr = oldir.irload.reg2 == B_R0 ? &dhold : &hold;
}

static void
Loadwriteback()
{
	wbus = dhold;
	Writereg(oldir.irload.reg2);
}

/****************************************************************************
 *	Storephia and Storephib handle the second cycle of store
 *	instructions.
 */

static void
Storephia()
{
	abus = aluout;
	read = FALSE;
	bytemode = oldir.irload.type == B_STOREBYTE;
	bbus = Readreg(oldir.irload.reg2);
	if(oldir.irload.type == B_STOREBYTE){
		dbus = (bbus&0xff) << 24 /* spread bytes out */
		     | (bbus&0xff) << 16
		     | (bbus&0xff) << 8
		     | (bbus&0xff);
	} else {
		dbus = bbus;
	}
}

static void
Storephib()
{
	writeback = Nop;
	hold = rstack[rtop];
	torptr = &hold;
	dhold = dbus;
}

/****************************************************************************
 *	Execphia and Execphib handle the first cycle of all instructions.
 */

static void
Execphia()
{
	abus = Asrc();
	read = TRUE; bytemode = FALSE;
	bbus = Bsrc();
	tbus = Tsrc();
	Stackop();
}

static void
Execphib()
{
	instrtypes[ir.irbranch.type]++;		/* statistics */
	hold = rstack[rtop];
	aluout = Alu();
	barrelout = Barrel();
	flag = Flag();
	pc = abus + 4;
	Execcleanup();
	oldir = ir;				/* for simulation only */
	ir.irvalue = irhold.irvalue = dbus;
}

/****************************************************************************
 *	Squashphia and Squashphib handle an extra cycle for conditional
 *	branches that were mistakenly taken.
 */

static void
Squashphia()
{
	abus = aluout;
	read = TRUE; bytemode = FALSE;
}

static void
Squashphib()
{
	pc = abus + 4;
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Nop;
	ir.irvalue = irhold.irvalue = dbus;
}

/****************************************************************************
 *	Intphia and Intphib handle the interrupt acknowledge cycle.
 */

static void
Intphia()
{
	abus = Interruptvector()<<2;
	read = TRUE; bytemode = FALSE;
	bbus = pc;
	tbus = 4;
	actualie = psw.fields.ie = 0;
	rtop++; rtop &= RSTACKSIZE-1;
}

static void
Intphib()
{
	pc = abus + 4;
	aluout = bbus-tbus;
	torptr = &aluout;
	writeback = Aluwritebacktor;
	hold = rstack[rtop];
	ir.irvalue = dbus;
}

/****************************************************************************
 *	The following functions handle the phase a and phase b hardware
 *	overflow and underflow operations
 */

static void
Rsover1phia()
{
	abus = pc;
	read = TRUE; bytemode = FALSE;
	bbus = externalrsp;
	tbus = 4;
}

static void
Rsover1phib()
{
	if(++rover == RSTACKSIZE) rover=0;
	if(++runder == RSTACKSIZE) runder=0;
	aluout = bbus-tbus;
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Nop;
	dhold = dbus;
}

static void
Rsover2phia()
{
	externalrsp = wbus = aluout;
	abus = aluout;
	read = FALSE; bytemode = FALSE;
	dbus = bbus = rstack[(rtop+1) & RSTACKSIZE-1];
}

static void
Rsover2phib()
{
	dhold = dbus;
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Nop;
}

static void
Rsunder1phia()
{
	abus = pc;
	read = TRUE; bytemode = FALSE;
	bbus = externalrsp;
	tbus = 4;
}

static void
Rsunder1phib()
{
	if(--rover == -1) rover=RSTACKSIZE-1;
	if(--runder == -1) runder=RSTACKSIZE-1;
	hold = externalrsp;
	torptr = &hold; /* TBD */
	writeback = Nop;
	aluout = bbus+tbus;
	dhold = dbus;
}

static void
Rsunder2phia()
{
	abus = hold;
	read = TRUE; bytemode = FALSE;
	externalrsp = wbus = aluout;
}

static void
Rsunder2phib()
{
	dhold = dbus;
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Rsunderflowwriteback;
}

static void
Rsunderflowwriteback()
{
	rstack[(rtop-4) & RSTACKSIZE-1] = wbus = dhold;
}

static void
Psover1phia()
{
	abus = pc;
	read = TRUE; bytemode = FALSE;
	bbus = externalpsp;
	tbus = 4;
}

static void
Psover1phib()
{
	if(++pover == PSTACKSIZE) pover=0;
	if(++punder == PSTACKSIZE) punder=0;
	aluout = bbus-tbus;
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Nop;
	dhold = dbus;
}

static void
Psover2phia()
{
	externalpsp = wbus = aluout;
	abus = aluout;
	read = FALSE; bytemode = FALSE;
	dbus = bbus = pstack[(ptop+1) & PSTACKSIZE-1];
}

static void
Psover2phib()
{
	dhold = dbus;
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Nop;
}

static void
Psunder1phia()
{
	abus = pc;
	read = TRUE; bytemode = FALSE;
	bbus = externalpsp;
	tbus = 4;
}

static void
Psunder1phib()
{
	if(--pover == -1) pover=PSTACKSIZE-1;
	if(--punder == -1) punder=PSTACKSIZE-1;
	hold = externalpsp;
	torptr = &hold; /* TBD */
	writeback = Nop;
	aluout = bbus+tbus;
	dhold = dbus;
}

static void
Psunder2phia()
{
	abus = hold;
	read = TRUE; bytemode = FALSE;
	externalpsp = wbus = aluout;
}

static void
Psunder2phib()
{
	dhold = dbus;
	hold = rstack[rtop];
	torptr = &hold;
	writeback = Psunderflowwriteback;
}

static void
Psunderflowwriteback()
{
	pstack[(ptop-4) & PSTACKSIZE-1] = wbus = dhold;
}

/****************************************************************************
 *	The following functions handle everything that can happen on phase
 *	a in the EXEC state.
 */

static int32
Asrc()
{
	int32 result;

	switch(ir.irbranch.type){
	   case B_FLOW:
		switch(ir.irbranch.brtype){
		   case B_COLON:
		   case B_DOES:
		   case B_BRANCH:
		   case B_QBRANCH:
			result = BRADDR(ir);
			break;
		   default:
			Cmdmsg("Unknown branch type");
			break;
		}
		break;
	   case B_LAL:
	   case B_LAH:
	   case B_LOAD:
	   case B_STORE:
	   case B_LOADBYTE:
	   case B_STOREBYTE:
	   case B_MICRO:
		result = (ir.irload.ret == B_RETURN) ? *torptr : pc;
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

static int32
Bsrc()
{
	int32 result;

	switch(ir.irbranch.type){
	   case B_FLOW:				/* DEFAULT for all but call */
		result = pc;
		break;
	   case B_LAL:
	   case B_LAH:
	   case B_LOAD:
	   case B_STORE:
	   case B_LOADBYTE:
	   case B_STOREBYTE:
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
Tsrc()
{
	int32 result;

	switch(ir.irbranch.type){
	   case B_LAL:
	   case B_LOAD:
	   case B_STORE:
	   case B_LOADBYTE:
	   case B_STOREBYTE:
		result = ir.irload.offset;
		break;
	   case B_LAH:
		result = (int32)(ir.irload.offset)<<16;
		break;
	   case B_FLOW:				/* DEFAULT */
	   case B_MICRO:
		if((ir.iralu.class == B_SHIFT) &&
		   (ir.irshift.imm == B_IMM)){
			result = ir.irshift.literal;
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

static void
Stackop()
{
	switch(ir.irbranch.type){
	   case B_FLOW:
		if(ir.irbranch.brtype == B_COLON){
			rtop++; rtop &= RSTACKSIZE-1;
		} else if(ir.irbranch.brtype == B_DOES){
			ptop++; ptop &= PSTACKSIZE-1;
		}
		break;
	   case B_LAL:
	   case B_LAH:
	   case B_LOAD:
	   case B_STORE:
	   case B_LOADBYTE:
	   case B_STOREBYTE:
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

static int32
Readreg(r)
unsigned int r;
{
	int32 tmp;
	switch(r){
	   case B_S0:	tmp = pstack[(ptop)   & PSTACKSIZE-1]; break;
	   case B_S1:	tmp = pstack[(ptop-1) & PSTACKSIZE-1]; break;
	   case B_S2:	tmp = pstack[(ptop-2) & PSTACKSIZE-1]; break;
	   case B_S3:	tmp = pstack[(ptop-3) & PSTACKSIZE-1]; break;
	   case B_R0:	tmp = rstack[(rtop)   & RSTACKSIZE-1]; break;
	   case B_R1:	tmp = rstack[(rtop-1) & RSTACKSIZE-1]; break;
	   case B_R2:	tmp = rstack[(rtop-2) & RSTACKSIZE-1]; break;
	   case B_R3:	tmp = rstack[(rtop-3) & RSTACKSIZE-1]; break;
	   case B_IAR:	tmp = pc; break;
	   case B_PSW:	tmp = psw.value; break;
	   case B_ZERO:	tmp = 0; break;
	   case B_Y:	tmp = y; break;
	   case B_U0:	tmp = udr0; break;
	   case B_U1:	tmp = udr1; break;
	   case B_U2:	tmp = udr2; break;
	   case B_U3:	tmp = udr3; break;
	   default:
		   Cmdmsg("Unknown register\n"); break;
	}
	return(tmp);
}

int alucond;				/* selected ALU condition */

#define COUT(x)		((x))
#define OVFLOW(x)	((x)>>1)
#define EQUAL(x)	((x)>>2)
#define NEG(x)		((x)>>3)

#define CIN(x)		(((x) & 1) << 4) /* TBD right for sparc? */

static int32
Alu()
{
	int32 result;

	switch(ir.irbranch.type){
	   case B_FLOW:
		result = bbus;
		break;
	   case B_LAL:
	   case B_LAH:
	   case B_LOAD:
	   case B_STORE:
	   case B_LOADBYTE:
	   case B_STOREBYTE:
		result = tbus + bbus;
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
	int32 result;
	short flags = 0;		/* XNZVC flags */

	switch(ir.iralu.class){
	   case B_ALU:
	   case B_TEST: {
		int cin;		/* carry input */

		switch(ir.iralu.cin){
		   case B_CZERO:	cin = 0; break;
		   case B_CONE:		cin = 1; break;
		   case B_CFLAG:	cin = flag; break;
		   case B_CFLAGBAR:	cin = ~flag; break;
		   default:		Cmdmsg("Unknown carry in\n"); break;
		}
		switch(ir.iralu.aluop){
		   case B_ONES:		result = ~0; break;
		   case B_ZEROS:	result = 0; break;
		   case B_NOPA:		result = tbus; break;
		   case B_NOPB:		result = bbus; break;
		   case B_NOTA:		result = ~tbus; break;
		   case B_NOTB:		result = ~bbus; break;
		   case B_AANDB:	result = tbus & bbus; break;
		   case B_ABARANDB:	result = ~tbus & bbus; break;
		   case B_AANDBBAR:	result = tbus & ~bbus; break;
		   case B_ANANDB:	result = ~(tbus & bbus); break;
		   case B_AORB:		result = tbus | bbus; break;
		   case B_ABARORB:	result = ~tbus | bbus; break;
		   case B_AORBBAR:	result = tbus | ~bbus; break;
		   case B_ANORB:	result = ~(tbus | bbus); break;
		   case B_AXORB:	result = tbus ^ bbus; break;
		   case B_ANXORB:	result = ~(tbus ^ bbus); break;
		   case B_APLUSB:	result = Addop(tbus,bbus,CIN(cin),
						&flags);
					break;
		   case B_AMINUSB:	result = Addop(tbus,~bbus,CIN(cin),
						&flags);
					break;
		   case B_BMINUSA:	result = Addop(bbus,~tbus,CIN(cin),
						&flags);
					break;
		   case B_NEGB:		result = Addop(0L,~bbus,CIN(cin),
						&flags);
					break;
		   case B_INCB:		result = Addop(0L,bbus,CIN(cin),&flags);
					break;
		   case B_DECB:		result = Addop(-1L,bbus,CIN(cin),
						&flags);
					break;
		   case B_BPLUSB:	result = Addop(bbus,bbus,CIN(cin),
						&flags);
					break;
		   default:		Cmdmsg("Unknown ALU operation\n");
					quitflag = TRUE;
					break;
		}
		if(result<0) flags |= 0x8;
		if(result==0) flags |= 0x4;
		flinput = alucond = ALUcond(flags);
		if(ir.iralu.class == B_TEST)
			result = alucond ? -1 : 0;
/* TBD alucond still needed?*/
		break;
	   }
	case B_STEP: {
		int32 shiftleftresult;	/* shifted B input */
		int shiftleftout;	/* output of left shifter */
		int shiftrightout;	/* output of right shifter */
		int boothshiftout;	/* output of internal ALU right shift*/

		if(ir.irstep.left == B_LEFT){
			shiftleftout = (bbus & MSB) ? 1 : 0;
			shiftleftresult = bbus<<1;
			if(y & MSB) shiftleftresult |= LSB;
		} else {
			shiftleftresult = bbus;
		}
		switch(ir.irstep.step){
		   case B_STEPB: {
		   	result = Addop(0L,shiftleftresult,CIN(0),&flags);
			if(result<0) flags |= 0x8;
			if(result==0) flags |= 0x4;
			alucond = ALUcond(flags);
			break; }
		   case B_CADD: {
			result = Addop((!flag?tbus:0L),shiftleftresult,CIN(0),
						&flags);
			if(result<0) flags |= 0x8;
			if(result==0) flags |= 0x4;
			alucond = ALUcond(flags);
			break; }
		   case B_DIV: {
			result = flag ? Addop(~tbus,shiftleftresult,CIN(1),
						&flags)
				      : Addop( tbus,shiftleftresult,CIN(0),
						&flags);
			if(result<0) flags |= 0x8;
			if(result==0) flags |= 0x4;
			alucond = ALUcond(flags);
			break; }
		   case B_MUL: {
			int rightsin;
			result = Booth1(tbus,shiftleftresult,
						&flags,&boothshiftout);
			if(result<0) flags |= 0x8;
			if(result==0) flags |= 0x4;
			alucond = ALUcond(flags);
			rightsin = (NEG(flags)^OVFLOW(flags)) & LSB;
			result = Booth2(result, rightsin, &boothshiftout);
			shiftrightout = result & LSB;
			result = (uint32)result >> 1;
			if(rightsin)	result |= MSB;
			else		result &= ~MSB;
			break; }
		   default: {
			Cmdmsg("Unknown step operation");
			break; }
		}
		switch(ir.irstep.fsrc){
		   case B_FLALUCOND:	flinput = alucond; break;
		   case B_FLY1:		flinput = (y>>1)&LSB; break;
		   case B_FLDIVHELP:	flinput = flag
					? (shiftleftout | (COUT(flags)&0x1))
					: (shiftleftout & (COUT(flags)&0x1));
					 break;
		   default: Cmdmsg("Unknown flag source"); break;
		}
		switch(ir.irstep.ycont){
		   case B_YNOP:
			break;
		   case B_YLEFT:
			y <<= 1;
			if(flag) y |= LSB;
			break;
		   case B_YRIGHT:
			y = (uint32)y>>2;
			if(boothshiftout) y |= (uint32)MSB>>1;
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
		alucond = ALUcond(flags);
		flinput = bbus & LSB;
		break;
	   }
	}
	return(result);
}

static int
ALUcond(flags)
short flags;
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
	return(result & LSB);			/* only lsb is flag */
}

static int32
Booth1(a, b, flagptr, shiftoutptr)
int32 a, b;
short *flagptr;
int *shiftoutptr;
{
#define ASR(x) (((x)&MSB) ? ((x)>>1)|MSB : ((x)>>1)&~MSB)
	uint32 result;

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
	return((int32)result);
}

static int32
Booth2(booth1result, rightsin, shiftoutptr)
int32 booth1result;
int rightsin;
int *shiftoutptr;
{
	uint32 result;

	switch(((y&0x3)<<1) | flag){
	   case 1:
	   case 2:
	   case 5:
	   case 6:
		*shiftoutptr = booth1result & LSB;
		result = booth1result >> 1;
		if(rightsin)	result |= MSB;
		else		result &= ~MSB;
		break;
	   default:
		result = booth1result;
		break;
	}
	return((int32)result);
}


static int32
Barrel()
{
	int32 result;

	switch(ir.irbranch.type){
	   case B_FLOW:				/* DEFAULT */
	   case B_LAL:
	   case B_LAH:
	   case B_LOAD:
	   case B_STORE:
	   case B_LOADBYTE:
	   case B_STOREBYTE:
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

static int32
Microbarrel()
{
	uint32 result;

	if(ir.iralu.class == B_SHIFT){
		int count = tbus & 0x1f;
		switch(ir.irshift.mode){
		   case B_ASR: {
			int32 signset = bbus & MSB;
			if(count){
				result = (uint32)bbus >> count;
				if(signset) result |= ((1<<count)-1)
							<< (32-count);
			} else {
				result = (uint32)bbus;
			}
			break;
		   }
		   case B_LSR:
			result = (uint32)bbus >> count; break;
		   case B_LSL:
			result = (uint32)bbus << count; break;
		   case B_ROL: {
			if(count){
				result = ((uint32)bbus << count)
				       | ((uint32)bbus >> (32-count));
			} else {
				result = (uint32)bbus;
			}
			break;
		   }
		   case B_SRFLAG:
			result = (uint32)bbus >> count;
			if(flag) result |= ((1<<count)-1) << (32-count);
			break;
		   case B_SRFLAGBAR:
			result = (uint32)bbus >> count;
			if(!flag) result |= ((1<<count)-1) << (32-count);
			break;
		   case B_PRI: {
			uint32 x;
			for(result=0, x=(uint32)bbus;
						result<32; result++, x<<=1)
				if(x&MSB) break;
			break;
		   }
		   default: Cmdmsg("Unknown shift mode\n"); break;
		}
	} else {
		result = (uint32)bbus;		/* DEFAULT */
	}
	return((int32)result);
}

static int
Flag()
{
	return(((ir.irbranch.type == B_MICRO) && (ir.iralu.flag == B_FLAG))
			? flinput : flag);
}

static void
Writereg(r)
unsigned int r;
{
	switch(r){
	   case B_S0:   pstack[(ptop)   & PSTACKSIZE-1] = wbus; break;
	   case B_S1:   pstack[(ptop-1) & PSTACKSIZE-1] = wbus; break;
	   case B_S2:   pstack[(ptop-2) & PSTACKSIZE-1] = wbus; break;
	   case B_S3:   pstack[(ptop-3) & PSTACKSIZE-1] = wbus; break;
	   case B_R0:   rstack[(rtop)   & RSTACKSIZE-1] = wbus; break;
	   case B_R1:   rstack[(rtop-1) & RSTACKSIZE-1] = wbus; break;
	   case B_R2:   rstack[(rtop-2) & RSTACKSIZE-1] = wbus; break;
	   case B_R3:   rstack[(rtop-3) & RSTACKSIZE-1] = wbus; break;
	   case B_IAR:  break;		/* data sink */
	   case B_PSW:  psw.value = wbus & 0x3f;
			  actualie = psw.fields.ie;
			  break;
	   case B_ZERO: break;		/* data sink */
	   case B_Y:	y = wbus; break;
	   case B_U0:	udr0 = wbus; break;
	   case B_U1:	udr1 = wbus; break;
	   case B_U2:	udr2 = wbus; break;
	   case B_U3:	udr3 = wbus; break;
	   default:
		   Cmdmsg("Unknown cp0 register\n"); break;
	}
}

static void
Execcleanup()
{
	switch(ir.irbranch.type){
	   case B_FLOW:
		switch(ir.irbranch.brtype){
		   case B_COLON:
			torptr = &aluout;
			writeback = Aluwritebacktor;
			break;
		   case B_DOES:
			torptr = &hold;
			writeback = Aluwritebacktos;
			break;
		   case B_BRANCH:
		   case B_QBRANCH:
			torptr = &hold;
			writeback = Nop;
			break;
		   default:
			Cmdmsg("Unknown branch type");
			break;
		}
		break;
	   case B_LOAD:
	   case B_STORE:
	   case B_LOADBYTE:
	   case B_STOREBYTE:
		torptr = &hold;
		writeback = Nop;
		break;
	   case B_LAL:
	   case B_LAH:
		torptr = (ir.irload.reg2 == B_R0) ? &aluout : &hold;
		writeback = Aluwriteback;
		break;
	   case B_MICRO:
		if(ir.iralu.class == B_SHIFT){
			torptr = (ir.iralu.reg2 == B_R0) ? &barrelout :&hold;
			writeback = Shiftwriteback;
		} else {
			torptr = (ir.iralu.reg2 == B_R0) ? &aluout : &hold;
			writeback = Aluwriteback;
		}
		break;
	   default:
		Cmdmsg("Unknown instruction type");
		break;
	}
}

static void
Aluwriteback()
{
	wbus = aluout;
	Writereg(oldir.irload.reg2);
}

static void
Aluwritebacktor()
{
	rstack[rtop] = wbus = aluout;
}

static void
Aluwritebacktos()
{
	pstack[ptop] = wbus = aluout;
}

static void
Shiftwriteback()
{
	wbus = barrelout;
	Writereg(oldir.irload.reg2);
}


void
Clearstats()
{
	int i;

	cycle = 0; stallcycle = 0;
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
	printf("\t%9d total execution cycles\n", cycle);
	printf("\t%9d stall cycles\n", stallcycle);
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
