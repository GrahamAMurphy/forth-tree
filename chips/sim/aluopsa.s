#ifdef sun	/* i.e. 68k or sparc */
#ifdef sparc	/* sparc */
	.seg	"text"
	.global	_Addop
! Addop(x,y,cin,flags)
! forthword x, y;  int cin;  short *flags;
_Addop:	addcc	%o2,-1,%g0			! set carry in
	addxcc	%o0,%o1,%o0			! do the add
	addx	%g0,%g0,%o1			! carry out
	bvs,a	0f
	add	%o1,2,%o1			! overflow
0:	bz,a	0f
	add	%o1,4,%o1			! zero
0:	bn,a	0f
	add	%o1,8,%o1			! negative
0:	retl
	sth	%o1,[%o3]			! return condition codes

#else		/* 68k */

	.globl _Addop

_Addop:	link	a6,#0
	movl	a6@(8),d1			| operand 1
	movl	a6@(12),d0			| operand 2
|	movl 	a6@(18),cc			| set op condition codes
	.long	0x44ee0012
	addxl	d1,d0				| do addition
	movl	a6@(20),a0			| address to return flags
	.word	0x42d0				| movw cc,a0@ 
	unlk	a6
	rts
#endif
#endif sun
