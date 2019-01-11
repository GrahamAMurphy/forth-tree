# Metacompiler Makefile
# Configurable parameters

CC =		gcc
CFLAGS =	-g

###########################################################################
SOURCEFILES =	dirs.c fc.c

###########################################################################
forth2m:	fc.o dirs.o
		$(CC) -o forth2m fc.o dirs.o

fc.o dirs.o:	fc.h

lint:
		lint -a -c -h $(SOURCEFILES)

clean:
		rm -f forth2m fc.o dirs.o
