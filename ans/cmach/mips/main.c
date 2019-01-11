/* Startup Code */

/* uses these interfaces: */
#include <procutil.h>
#include "../../forth.h"

void
StartApp()
{
	int i;

	for(i=1; i<16; i++) RegisterExp(i, RestartApp);
	init_forth();
	EnableInts();			/* enable interrupts */
	do_forth(0,0);			/* dummy args */
}

