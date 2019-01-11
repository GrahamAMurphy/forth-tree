/* Interface between Forth TTY and MIPSLIB TTY */

/* uses these interfaces: */
#include <ttyutil.h>
#include <tty.h>
/* defines this interface: */
#include "io.h"

/*------------------------------------------------------------------------
 * Public interface to tty.
 */

void
cmd_ioinit(void) {
	TtyStart();
}

void
cmd_putc(unsigned char c) {
	TtyPutc(c, &ttyio);
}

int
cmd_getc(void) {
	return TtyGetc(&ttyio);
}

/* Rest of TTY driver appended here by make: */
