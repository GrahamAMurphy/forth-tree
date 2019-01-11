/* Advanced Camera (Mongoose-V) custom TTY code */
/* Uses UART1 */

/* uses these interfaces: */
#include <camera.h>
/* defines this interface */
#include "io.h"

/* UART registers addresses, etc. used by TTY driver */
#define XMIT		U1_TX		/* transmit register */
#define RECV		U1_RX		/* receive register */
#define STATUS		PER_STAT	/* status register */
#define XMIT_MASK	(1<<20)		/* transmit mask for status */
#define RECV_MASK	(1<<21)		/* receive mask for status */

void
cmd_ioinit(void)
{
	*U1_BAUD  = 0x04e104e1;		/* 9600 baud (at 12MHz clock) */
	*PER_CMD |= 0x00003800;		/* enable tx and rx; RTS */
}

/* Rest of TTY driver appended here by make: */
