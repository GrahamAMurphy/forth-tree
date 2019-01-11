/*
 *	character device drivers for Heurikon 68k V2F board
 *
 *	This version of the driver supports the following features:
 *		XON/XOFF flow control on output channel
 *		type ahead buffer on input channel
 */

#define VECTbase 0x02000000	/* base of on-card RAM */
#define VECTOR 0x40		/* MFP interrupt vector number */

#include "iohk68v2f.h"
#include "io.h"

/*
 *	These are the actual serial channel descriptors.
 */

#define MFP ((unsigned char *)0x00fec000)

struct channel chanA = {QUIET, MFP };

struct channel *chans[] = { &chanA };

/*
 *	This routine initializes the serial channels. It resets the UARTs,
 *	programs the UARTs' interrupt vectors, programs the UARTs, sets the
 *	baud rate to be 9600 for each channel, places pointers to the inter-
 *	rupt routines in the 68000's vector table, initializes the channel
 *	descriptor data structures, and enables interrupts.
 */

initchans()
{
	int rcvA();			/* assembly language int. routine */
	int txA();
	register int cnt, (**vectptr)();
	static int (*vecttable[])() = {
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, txA, 0,
		rcvA, 0, 0, 0
	};
	static struct mfpentry {
		unsigned char *reg;
		unsigned char val;
	} mfptable[] = {
		&MFP[tacr],	0x07,	/* timer A, delay mode, 200 prescale */
		&MFP[tadr],	192,	/* timer A, counter value */
		&MFP[vr],  VECTOR|0x08,	/* vector, in-service reg. enable */
		&MFP[iera],	0x14,	/* enable rcv, tx interrupt */
		&MFP[ierb],	0x00,
		&MFP[ipra],	0x00,	/* clear interrupt pending regs */
		&MFP[iprb],	0x00,
		&MFP[isra],	0x00,	/* clear interrupt in service regs */
		&MFP[isrb],	0x00,
		&MFP[imra],	0x34,	/* interrupt mask register */
		&MFP[tcdcr],	0x11,	/* timer C & D, delay mode,4 prescale*/
		&MFP[tcdr],	0x02,	/* timer C, count value (for 9600) */
		&MFP[tddr],	0x02,	/* timer D, count value (for 9600) */
		&MFP[ucr],	0x98,	/* 16 prescale,8 bits,no parity,2stop*/
		&MFP[rsr],	0x01,
		&MFP[tsr],	0x01	/* enable tx and rcv */
	};

	vectptr = (int (**)())(VECTbase + (VECTOR * 4));
	for(cnt = 0; cnt < sizeof(vecttable)/sizeof(int(**)()); cnt++)
		*vectptr++ = vecttable[cnt];

	for(cnt = 0; cnt < sizeof(mfptable)/sizeof(struct mfpentry); cnt++)
		*mfptable[cnt].reg = mfptable[cnt].val;

	for(cnt = 0; cnt < 10000; cnt++)/* baud rate settle time */
		;

	for(cnt=0; cnt < sizeof(chans)/sizeof(struct channel *); cnt++){
		chans[cnt]->state=QUIET;/* initial state in QUIET */
		chans[cnt]->outfifo.count =
		chans[cnt]->infifo.count = 0;
		chans[cnt]->outfifo.inptr =
		chans[cnt]->outfifo.outptr = chans[cnt]->outfifo.buffer;
		chans[cnt]->infifo.inptr =
		chans[cnt]->infifo.outptr = chans[cnt]->infifo.buffer;
	}

	INTLEVEL0;			/* enable interrupts */
}

/*
 *	These are the actual I/O routines. Output is a routine for buffering
 *	output characters in the output fifo.  Transint is the transmit 
 *	interrupt routine.  Input is a routine for getting the next character
 *	from the type ahead buffer.  Rcvint is the receive interrupt routine.
 */

/*
 *	STARTOUTPUT is a macro for starting output on channel x using fifo
 *	y.
 */

#define STARTOUTPUT(x,y) \
	x->portno[udr] = *y->outptr; \
	y->count--; \
	if(++y->outptr == &y->buffer[SIZE]){ \
		y->outptr = y->buffer; \
	} \
	x->state = OUTPUT

/*
 *	Output places the input character in the output fifo.  If the fifo is
 *	full, it does a busy wait until there is room.  Interrupts are
 *	disabled during the time it is modifying the channel descriptor.
 */

output(chan,c)
register struct channel *chan;
char c;
{
	register struct fifo *ofifo = &chan->outfifo;

	while(ofifo->count == SIZE)	/* busy wait until there is room */
		;
	INTLEVEL4;			/* disable interrupts */
	*ofifo->inptr=c;		/* put character in fifo */
	ofifo->count++;
	if(++ofifo->inptr == &ofifo->buffer[SIZE]){
					/* step pointer and wrap around */
		ofifo->inptr = ofifo->buffer;
	}
	if(chan->state == QUIET){	/* if first character */
		STARTOUTPUT(chan,ofifo);/* start sending characters */
	}
	INTLEVEL0;			/* enable interrupts */
}

/*
 *	Transmit interrupt.  The channel will not be in the QUIET state
 *	when this interrupt is received.  If the state is not THROTTLED,
 *	the next character in the fifo is given to the UART unless the
 *	fifo is empty.  If this is the case, the state is set to quiet
 *	and the UART is prevented from issuing any further transmit int-
 *	errupts.  If the state is THROTTLED, the UART is also prevented
 *	from generating further interrupts.
 */

transint(chan)
register struct channel *chan;
{
	register struct fifo *ofifo = &chan->outfifo;

	if(chan->state == OUTPUT){	/* if outputting */
		if(ofifo->count == 0){	/* if fifo empty */
			chan->state=QUIET;
			chan->portno[ipra] = 0xfb;
			chan->portno[isra] = 0xfb;
			return;
		}
		chan->portno[udr] = *ofifo->outptr;
		ofifo->count--;
		if(++ofifo->outptr == &ofifo->buffer[SIZE])
			ofifo->outptr = ofifo->buffer;
	} else {			/* state must be THROTTLED */
					/* disable further interrupts */
		chan->portno[ipra] = 0xfb;
	}
	chan->portno[isra] = 0xfb;
	return;
}

/*
 *	Read next character from type ahead buffer.  If the fifo is
 *	empty, busy wait until there is data.  Interrupts are disabled
 *	during the time the channel descriptor is being modified.
 */

input(chan)
register struct channel *chan;
{
	register unsigned char c;	/* next character */
	register struct fifo *ififo = &chan->infifo;

	while(ififo->count == 0)	/* while nothing in fifo */
		;
	INTLEVEL4;			/* disable interrupts */
	c = *ififo->outptr;		/* get character from buffer */
	ififo->count--;
	if(++ififo->outptr == &ififo->buffer[SIZE])
		ififo->outptr = ififo->buffer;
	INTLEVEL0;			/* restore interrupts */
	return(c);
}

#define XOFF  '\023'
#define XON   '\021'

/*
 *	Interrupt routine to receive a character from the UART.  If an
 *	XOFF is received, the channel state is set to THROTTLED.  If
 *	an XON is received, the state is THROTTLED, and there are
 *	characters in the output fifo, the output is started up and
 *	the state is set to OUTPUT.  Otherwise the character is placed
 *	in the type ahead buffer.  If the buffer is full, the character
 *	is thrown away without a second thought.
 */

recint(chan)
register struct channel *chan;
{
	register struct fifo *ififo = &chan->infifo;
	register unsigned char c = chan->portno[udr];

	switch(c){
	   case XOFF: chan->state = THROTTLED;
		      break;		/* throttle output */
	   case XON:  if(chan->state == THROTTLED){
			register struct fifo *ofifo = &chan->outfifo;
			if(ofifo->count){
				STARTOUTPUT(chan,ofifo);
			} else {
				chan->state = QUIET;
			}
		      }
		      break;
	   default:   if(ififo->count != SIZE){
			*ififo->inptr = c;
			ififo->count++;
			if(++ififo->inptr == &ififo->buffer[SIZE])
				ififo->inptr = ififo->buffer;
		      }
		      break;
	}
	chan->portno[isra] = 0xef;
}

