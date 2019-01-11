/* This file gathers descriptions of things external to the processor
 * or at the boundary between internal and external.
 */

/* external things */
#define PORT_RD		1		/* read port */
#define PORT_WR		2		/* write port */
#define PORT_RDBYTE	3		/* read byte (FRISC 4) */
#define PORT_WRBYTE	4		/* write byte (FRISC 4) */

#define TRAPBASE 0x40000000L		/* start of I/O trap space */
#define TRAPTOP  0x5fffffffL		/* end of I/O trap space */

extern int32 portaddr;			/* port address */
extern int32 portvalue;			/* value read/written to port */
extern int portio;			/* read/write operation */

extern int busidle;			/* bus idle; overrides portio */
