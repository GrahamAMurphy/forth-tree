#define INTLEVEL4  asm("movw #0x2400,sr")
#define INTLEVEL3  asm("movw #0x2300,sr")
#define INTLEVEL0  asm("movw #0x2000,sr")

/*
 *	This structure describes the input and output fifos on each line
 */

#define SIZE	256			/* size of fifos */

struct fifo {				/* I/O fifo */
	int count;			/* number of bytes in fifo */
	char *inptr, *outptr;		/* in and out pointer into buffer */
	char buffer[SIZE];		/* character buffer */
};

/*
 *	This structure describes one serial channel: the state of the channel,
 *		the address of the data port in the UART, and the input
 *		and output fifos.
 */

#define QUIET	  0			/* nothing being transmitted */
#define OUTPUT	  1			/* transmitting */
#define THROTTLED 2			/* XOFF received */

struct channel {			/* tty channel descriptor */
	int state;			/* QUIET, OUTPUT, or THROTTLED */
	volatile unsigned char *portno;	/* I/O address of data port */
	struct fifo outfifo,		/* transmit fifo */
		    infifo;		/* receive fifo */
};

extern struct channel *chans[];
