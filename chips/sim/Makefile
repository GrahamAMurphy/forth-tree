# this program must be compiled in the att universe (on Masscomp systems)
CFLAGS		= -O 
# target directory for compiled applications
BIN =		/home/users/john/bin/`mach`

##########################################################################
LINTFILES	= main.c parser.c aluops.c fakeio.c vectors.c save.c \
			fsim4c.c trace.c fkscrn4b.c

COMOBJS		= main.o parser.o aluopsc.o fakeio.o vectors.o save.o
FOBJS		= $(COMOBJS) fsim3.o trace.o screen3.o
NIFOBJS		= $(COMOBJS) fsim3.o trace.o fkscrn.o
F4OBJS		= $(COMOBJS) fsim4.o trace.o screen4.o
NIF4OBJS	= $(COMOBJS) fsim4.o trace.o fkscrn.o
F4PIPEOBJS	= $(COMOBJS) fsim4a.o trace.o screen4a.o
NIF4PIPEOBJS	= $(COMOBJS) fsim4a.o trace.o fkscrn.o
F4BYTEOBJS	= $(COMOBJS) fsim4b.o trace.o screen4b.o
NIF4BYTEOBJS	= $(COMOBJS) fsim4b.o trace.o fkscrn4b.o
F4SEAOBJS	= $(COMOBJS) fsim4c.o trace.o screen4c.o
NIF4SEAOBJS	= $(COMOBJS) fsim4c.o trace.o fkscrn4b.o
ALLOBJS		= *.o \
			fsim3 nifsim3 fsim4 nifsim4 \
			fsim4a nifsim4a fsim4b nifsim4b fsim4c nifsim4c
LDOBJS		= 

all:		fsim3 nifsim3 fsim4c nifsim4c

frisc3:		fsim3 nifsim3

frisc4:		fsim4c nifsim4c

fsim3:		$(FOBJS)
		$(CC) -o fsim3 $(CFLAGS) $(FOBJS) $(LDOBJS) -lcurses -ltermlib

nifsim3:	$(NIFOBJS)
		$(CC) -o nifsim3 $(CFLAGS) $(NIFOBJS) $(LDOBJS)

fsim4:		$(F4OBJS)
		$(CC) -o fsim4 $(CFLAGS) $(F4OBJS) -lcurses -ltermlib

nifsim4:	$(NIF4OBJS)
		$(CC) -o nifsim4 $(CFLAGS) $(NIF4OBJS)

fsim4a:		$(F4PIPEOBJS)
		$(CC) -o fsim4a $(CFLAGS) $(F4PIPEOBJS) -lcurses -ltermlib

nifsim4a:	$(NIF4PIPEOBJS)
		$(CC) -o nifsim4a $(CFLAGS) $(NIF4PIPEOBJS) -lcurses -ltermlib

fsim4b:		$(F4BYTEOBJS)
		$(CC) -o fsim4b $(CFLAGS) $(F4BYTEOBJS) -lcurses -ltermlib

nifsim4b:	$(NIF4BYTEOBJS)
		$(CC) -o nifsim4b $(CFLAGS) $(NIF4BYTEOBJS) -lcurses -ltermlib

fsim4c:		$(F4SEAOBJS)
		$(CC) -o fsim4c $(CFLAGS) $(F4SEAOBJS) $(LDOBJS) -lcurses -ltermlib

nifsim4c:	$(NIF4SEAOBJS)
		$(CC) -o nifsim4c $(CFLAGS) $(NIF4SEAOBJS) $(LDOBJS)

lint:
		lint -a $(LINTFILES)

install:	fsim3 nifsim3 fsim4c nifsim4c
		strip fsim3 nifsim3 fsim4c nifsim4c
		mv fsim3 nifsim3 fsim4c nifsim4c $(BIN)

parser.c:	scan.c parser.y
		yacc parser.y
		mv y.tab.c parser.c

scan.c:		scan.l
		lex scan.l
		mv lex.yy.c scan.c

main.o:		globals.h external.h
fsim3.o:	globals.h external.h intrnl3.h frmts3.h masks3.h
fsim4.o:	globals.h external.h intrnl4.h frmts4.h masks4.h \
		ibrom4.h
fsim4a.o:	globals.h external.h intrnl4a.h frmts4.h \
		masks4.h ibrom4.h
fsim4b.o:	globals.h external.h intrnl4a.h frmts4b.h \
		masks4b.h ibrom4b.h
fsim4c.o:	globals.h external.h intrnl4c.h frmts4c.h \
		masks4c.h ibrom4c.h
screen3.o:	globals.h external.h intrnl3.h frmts3.h
screen4.o:	globals.h external.h intrnl4.h frmts4.h
screen4a.o:	globals.h external.h intrnl4a.h frmts4.h
screen4b.o:	globals.h external.h intrnl4a.h frmts4b.h
screen4c.o:	globals.h external.h intrnl4c.h frmts4c.h
fkscrn.o:	globals.h external.h
fkscrn4b.o:	globals.h external.h
fakeio.o:	globals.h external.h
trace.o:	globals.h external.h
vectors.o:	globals.h external.h
save.o:		globals.h external.h
aluopsc.o:	globals.h			# portable version
aluopsa.o:
		$(CC) -E aluopsa.s >foo.s
		as -o aluopsa.o foo.s
		rm foo.s

clean:
		rm -f $(ALLOBJS)
