#define NEXT asm("clrl d0")
#define JUMP asm("clrl d0")

typedef int forthword;

#define PSTSIZE	    64
#define RSTSIZE	    64

forthword pstack[PSTSIZE];
forthword rstack[RSTSIZE];

extern Alit, Abranch, Aqbranch, Apdo, Ai, Aj, Apleave;
extern Astore, Astoresp, Aplus, Aplusstore, Aminus, Aminusone;
extern Azero, Azeroless, Aone, Aoneplus, Aoneminus;
extern Atwo, Atwoplus, Atwominus, Atwostar, Atwoslash;

foo()
{

	register forthword *iar;
	register forthword *psp= &pstack[PSTSIZE];
	register forthword *rsp= &rstack[RSTSIZE];
	register forthword *w0;

/*		inner interpreter		*/

Apcolon:*--rsp = (forthword) iar;
	iar = w0;

lnext:	w0 = (forthword *) *iar++;
	{ register forthword *w1 = (forthword *) *w0++;
	  JUMP;
	}

Areturn:iar = (forthword *) *iar++;
	w0 = (forthword *) *iar++;
	{ register forthword *w1 = (forthword *) *w0++;
	  JUMP;
	}

Aexecute:
	w0 = (forthword *) *psp++;
	{ register forthword *w1 = (forthword *) *w0++;
	  JUMP;
	}

Alit:	*--psp = *iar++;
	NEXT;

Abranch:	iar = (forthword *) *iar;
	NEXT;

Aqbranch:
	if (*psp++ == 0) goto Abranch;
	iar++;
	NEXT;

Apdo:	*--rsp = *psp++ - (*--rsp = *(psp+1) + 0x80000000);
	psp++;
	NEXT;

/* ploop and pploop go here when I figure out how to implement them */

Ai:	*--psp = *rsp + *(rsp+1);
	NEXT;

Aj:	*--psp = *(rsp+2) + *(rsp+3);
	NEXT;

Apleave:	rsp += 2;
	iar = (forthword *) *iar;
	NEXT;

Astore:	{ register forthword *tmp = (forthword *) *psp++;
	* tmp = *psp++;
	}
	NEXT;

Astoresp:
	psp = (forthword *) *psp;
	NEXT;

Aplus:	*psp += *psp++;
	NEXT;

Aplusstore:
	{ register forthword *tmp = (forthword *) *psp++;
	* tmp += *psp++;
	}
	NEXT;

Aminus:	*psp -= *psp++;
	NEXT;

Aminusone:
	*--psp = -1;
	NEXT;

Azero:	*--psp = 0;
	NEXT;

Azeroless:
	*psp = (*psp == 0) ? -1 : 0;
	NEXT;

Aone:	*--psp = 1;
	NEXT;

Aoneplus:
	(*psp)++;
	NEXT;

Aoneminus:
	(*psp)--;
	NEXT;

Atwo:	*--psp = 2;
	NEXT;

Atwoplus:
	*psp += 2;
	NEXT;

Atwominus:
	*psp -= 2;
	NEXT;

Atwostar:
	*psp << 1;
	NEXT;

Atwoslash:
	*psp >> 1;
	NEXT;

Acat:	*psp = (forthword) *((unsigned char *) *psp);
	NEXT;
}

