/* cache utilities */

/* #include "/project/iem/R3000/UX/PMON/include/regdef.h" */
#include <regdef.h>

	.text
	.align 2

	.globl CacheFetch
	.ent CacheFetch
CacheFetch:
	.set noreorder
	mfc0	t0,C0_SR
	nop
	or	t1,t0,(1<<16)		/* remember original and modified */
	mtc0	t1,C0_SR		/* isolate caches */
	nop
	nop
	nop
	nop
	.set reorder

	lw	v0,(a0)			/* fetch from cache */

	mtc0	t0,C0_SR		/* unisolate caches */
	jr	ra
	.end CacheFetch

	.globl CacheStore
	.ent CacheStore
CacheStore:
	.set noreorder
	mfc0	t0,C0_SR
	nop
	or	t1,t0,(1<<16)		/* remember original and modified */
	mtc0	t1,C0_SR		/* isolate caches */
	nop
	nop
	nop
	nop
	.set reorder

	sw	a0,(a1)			/* store to cache */

	mtc0	t0,C0_SR		/* unisolate caches */
	jr	ra
	.end CacheStore
