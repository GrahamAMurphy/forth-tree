/*************************************************************************
 * Initialize and start a program.
 * Note: this also attempts to localize compiler dependencies, i.e., what
 * is name of initial gp pointer, etc.
 */

#include <regdef.h>

/* TBD: test data */
	.data
	.align 2
testdata:.word 0

	.text
	.align	2

/* _start: start application */
	.globl _start
	.ent _start
_start:
	jal	__clrbss		/* erase unitialized data */

/* flush EDAC errors */
	la	t0, 0xb0000000
	la	t1, 0xb0200000
0:	lw	t2,(t0)
	nop
	sw	t2,(t0)
	addiu	t0, t0, 4
	bne	t0,t1,0b

/* enable EDAC */
	li	t0,0x01480041		/* setup SPEC2 */
	sw	t0,0xfffe010c
	sh	$0,0xbd18000c		/* disable SRAM EDAC */
	li	t0,0x800018e0		/* enable EDAC */
	sw	t0,0xfffe0180
	li	t0,0xffff		/* enable SRAM EDAC */
	sh	t0,0xbd18000c

/* generate EDAC error in a test location */
#if 0
	lw	t2,0xfffe0180		/* override EDAC generated parity */
	li	t1,0x40000000
	or	t0,t2,t1
	sw	t0,0xfffe0180
	li	t0,0xc2			/* parity bits */
	sw	t0,0xfffe0194
	li	t0,0x55555555		/* write test pattern */
	la	t1,testdata
	sw	t0,(t1)
	sw	t2,0xfffe0180		/* turn off EDAC override */
#endif

/* RestartApp: restart application */
	.globl RestartApp
RestartApp:
	.set noreorder
	mfc0	t0, C0_SR		/* mask and disable all interrupts */
	li	t1, 0xffff0000
	and	t0,t0,t1
	mtc0	t0, C0_SR
	.set reorder
	la	sp,__stacktop		/* init/empty stack */
	la	gp,_gp
#if 0 /* use PMON exception handlers */
	jal	__initexp		/* initialize exception handler */
	jal	FlushCache		/* flush instruction cache */
#endif
	jal	StartApp		/* start user code */
	/* should not get here, but catch if it does. */
0:	b	0b
	.end _start

/* __clrbss(): clear (i.e. zero) BSS section. */
	.ent __clrbss
__clrbss:
	la	t0,_fbss		/* start of BSS section */
	la	t1,_end			/* end of BSS section */
	b	1f			/* in case BSS is empty */
0:	sb	zero,(t0)		/* zero a byte at a time; BSS might */
	addiu	t0,t0,1			/*  not be word aligned */
1:	bne	t0,t1,0b
	jr	ra
	.end __clrbss
