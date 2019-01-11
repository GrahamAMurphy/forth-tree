/* Stand-alone startup code */

#include "forth.h"			/* uses this interface */

void
go()					/* GNU C: use go instead of main */
{
	init_forth();
	do_forth(0,0);			/* dummy args */
}
