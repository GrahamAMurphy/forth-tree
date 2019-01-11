
|	interrupt routines

	.globl _txA, _rcvA

_txA:	.word	0x48e7, 0xc0c0			| moveml #<d0,d1,a0,a1>,sp@-
	movl	#_chanA,sp@-
	jsr	_transint
	addql	#4,sp
	.word	0x4cdf, 0x0303			| moveml sp@+,<d0,d1,a0,a1>
	rte

_rcvA:	.word	0x48e7, 0xc0c0			| moveml #<d0,d1,a0,a1>,sp@-
	movl	#_chanA,sp@-
	jsr	_recint
	addql	#4,sp
	.word	0x4cdf, 0x0303			| moveml sp@+,<d0,d1,a0,a1>
	rte
