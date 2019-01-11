/* TIMED (Mongoose-V) custom TTY code */
/* Uses UART0 */

/* uses these interfaces: */
#include <timed.h>
/* defines this interface */
#include "io.h"

/* UART registers addresses, etc. used by TTY driver */
#define XMIT		U0_TX		/* transmit register */
#define RECV		U0_RX		/* receive register */
#define STATUS		PER_STAT	/* status register */
#define XMIT_MASK	(1<<14)		/* transmit mask for status */
#define RECV_MASK	(1<<15)		/* receive mask for status */

void
cmd_ioinit(void)
{
	*U0_BAUD  = 0x04e104e1;		/* 9600 baud (at 12MHz clock) */
	*PER_CMD |= 0x000000e0;		/* enable tx and rx; RTS */
}

/* Rest of TTY driver appended here by make: */
