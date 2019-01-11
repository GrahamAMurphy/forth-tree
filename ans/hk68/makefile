CFLAGS = -O
CC = gcc
CPP = /lib/cpp $(CPPFLAGS)

OBJS =		inthk68v2f.o iohk68v2f.o forthio.o
sahk68v2fforth:	$(OBJS)
		ld -r -o ../allsaio.o $(OBJS)

#########################################################################
forthio.o:	io.h

iohk68v2f.o:	iohk68v2f.h io.h

clean:
		rm -f *.o
