/* TurboRocket custom interrupt-driven TTY code */
/* Stand-alone startup code */

/* uses these interfaces: */
#include <camera.h>
#include <procutil.h>
#include <ttyutil.h>
/* defines this interface: */
#include "io.h"

/*------------------------------------------------------------------------
 * tty interface; uses UART 0
 */

/* UART registers */
#define UART_XMIT	U0_TX
#define UART_RECV	U0_RX
#define UART_BAUD	U0_BAUD

/* UART commands */
#define RTS		(1<<7)		/* request to send */
#define ENA_XMIT	(1<<6)		/* enable transmitter */
#define ENA_RECV	(1<<5)		/* enable receiver */

/* UART status, mask, or cause */
#define RDY_XMIT	(1<<14)		/* transmitter ready */
#define RDY_RECV	(1<<15)		/* receiver ready */

/* Create tty descriptor */
static struct tty tty1;

/*------------------------------------------------------------------------
 * Helper functions; registered with generic tty driver.
 */

/* write first char to UART */
static void UartOutFirst(unsigned char c)
{
	*PER_CMD |= ENA_XMIT;
	*UART_XMIT = c;
}

/* write char to UART */
static void UartOut(unsigned char c)
{
	*UART_XMIT = c;
}

/* chars unavailable for UART */
static void UartStop(void)
{
	*PER_CMD &= ~ENA_XMIT;
}

/* read char from UART */
static unsigned char UartIn(void)
{
	return(*UART_RECV);
}

/*------------------------------------------------------------------------
 * Interrupt routine.
 */

static void PerInterrupt(void)
{
	unsigned int status = *PER_CAUSE;
	if(status & RDY_XMIT) TtyTransInt(&tty1);
	if(status & RDY_RECV) TtyRcvInt(&tty1);
}

/*------------------------------------------------------------------------
 * Public interface to tty.
 */

void
cmd_ioinit(void) {
	/* initialize UART registers */
/*	*UART_BAUD  = 0x04e104e1;	/*  9600 baud (at 12MHz clock) */
	*UART_BAUD  = 0x02700270;	/* 19200 baud (at 12MHz clock) */
	*PER_CMD |= ENA_RECV|RTS;	/* enable rx, assert RTS */

	/* initialize tty structure and register helper functions. */
	TtyInit(&tty1, UartOutFirst, UartOut, UartStop, UartIn);

	/* Install and enable interrupt handler */
	RegisterInt(PER_INT, PerInterrupt);
	UnmaskInt(PER_INT);
	*PER_MASK |= RDY_XMIT|RDY_RECV;	/* unmask UART interrupts */
}

void
cmd_putc(unsigned char c) {
	TtyPutc(c, &tty1);
}

int
cmd_getc(void) {
	return TtyGetc(&tty1);
}

/* Rest of TTY driver appended here by make: */
