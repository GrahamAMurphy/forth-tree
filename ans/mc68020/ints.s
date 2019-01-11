/* (c) 1992 Johns Hopkins University / Applied Physics Laboratory */
#include "../macro.h"

.globl	Alock, Aunlock, Astartclock, Arti

/* lock		( -- oldmask ) */
Alock:	movl	#0,_errno		/* reset error number */
	jbsr	_lock			/* use wrapper for sigblock(2) */
	movl	d0,psp@-		/* return old interrupt mask */
	next

/* unlock	( oldmask -- ) */
Aunlock:movl	#0,_errno		/* reset error number */
	movl	psp@+,sp@-		/* original interrupt mask */
	jbsr	_unlock			/* use wrapper for sigsetmask(2) */
	addql	#4,sp			/* discard arguments */
	next

/* start-clock	( -- ) */
Astartclock:
	movl	#0,_errno		/* reset error number */
	jbsr	_startclock		/* user wrapper for signal(3) */
	next

/****************************************************************************/
/* Interrupt handler */

	.data
	.=.+256				/* parameter stack */
TOPPS:

	.text
	.globl	_alarmhandler
_alarmhandler:
	moveml	a1-a6/d1-d7,sp@-	/* save most registers */
	movl	#1,sp@-			/* one second */
	jbsr	_alarm			/* reset alarm clock: alarm(3) */
	addql	#4,sp			/* discard arguments */
        movl    #TOPPS,psp              /* stack pointer */
        movl    #ALARMHANDLER,iar       /* point to handler in Forth */
        movl    #lnext,addrnext         /* save inner interp. entry point */
        next

/* rti		( -- ) */
Arti:	moveml	sp@+,a1-a6/d1-d7	/* restore registers */
	rts				/* return from interrupt */

