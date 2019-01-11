# ANS Forth system Makefile
# Note: the Makefile is automatically constructed from Makefile.tem
# using the config shell script.

# default: compile forth
default:	forth

########################################################################
# TBD

ARCH =		cmach

# Flags for forth2m (Forth compiler)
# -u can be added to generate uppercase dictionary
INSTALLAT =	/usr/local/bin/forth
mc68020flags =	-m -o pch
sparcflags =	-m -o pch
cmachflags =	-o ptdihc
F2MFLAGS =	$(cmachflags) -w -i $(INSTALLAT)

############################################################################
# Unix specific code.

unixfsrcs =	$(ARCH)/macro.fr $(ARCH)/prims.fr \
		$(ARCH)/strings.fr $(ARCH)/file.fr $(ARCH)/unixmem.fr \
		$(ARCH)/unixtty.fr \
		$(ARCH)/lostart.fr \
		os.fr file-include.fr unix-start.fr \
		$(ARCH)/dict.fr
unixlobj =	jhforthc.o filewrap.o
unixobj =	$(unixlobj) main.o
unixlink =	$(unixobj)

filewrap.o:	filewrap.h
main.o:		forth.h io.h

############################################################################
# TurboRocket specific code.

turbofsrcs =	$(ARCH)/macro.fr $(ARCH)/prims.fr \
		$(ARCH)/satty.fr \
		$(ARCH)/lostart.fr \
		os.fr ../chips/silent.fr sa-start.fr \
		$(ARCH)/dict.fr
turbolobj =	jhforthc.o mips-tty.o
turboobj =	$(turbolobj) cmach/mips/main.o
turbolink =	-T $(MIPSLIB)/elf32ebmip.x \
			-Ttext 0x80040000 -defsym __stacktop=0x80800000 \
			-oformat srec \
			-u _start $(MIPSLIB)/startup-g.a \
			$(turboobj) $(MIPSLIB)/turbo-g.a

turbotty =	cmach/mips/tty.c devices/forthio.c

cmach/mips/main.o:forth.h

############################################################################
# IDT 79S381 evaluation board specific code.

79s381fsrcs =	$(ARCH)/macro.fr $(ARCH)/prims.fr \
		$(ARCH)/satty.fr \
		$(ARCH)/lostart.fr \
		os.fr ../chips/silent.fr sa-start.fr \
		$(ARCH)/dict.fr
79s381lobj =	jhforthc.o mips-tty.o
79s381obj =	$(79s381lobj) cmach/mips/main.o
79s381link =	-T $(MIPSLIB)/elf32ebmip.x \
			-Ttext 0x80000100 -defsym __stacktop=0x84040000 \
			-oformat srec \
			-u _ramboot $(MIPSLIB)/startup-g.a \
			$(79s381obj) $(MIPSLIB)/79s381-g.a

79s381tty =	cmach/mips/tty.c devices/forthio.c

cmach/mips/main.o:forth.h

############################################################################
# TIMED (Mongoose processor) specific code.

timedfsrcs =	$(ARCH)/macro.fr $(ARCH)/prims.fr \
		$(ARCH)/satty.fr \
		$(ARCH)/lostart.fr \
		os.fr ../chips/silent.fr sa-start.fr \
		$(ARCH)/dict.fr
timedlobj =	jhforthc.o mips-tty.o
timedobj =	$(timedlobj) cmach/mips/main.o
timedlink =	-T $(MIPSLIB)/elf32ebmip.x \
			-Ttext 90040000 -defsym __stacktop=0x90200000 \
			-oformat srec \
			-u _ramboot $(MIPSLIB)/startup-g.a \
			$(timedobj) $(MIPSLIB)/timed-g.a

timedtty =	cmach/mips/timed/mvpoll.c devices/polltty.c devices/forthio.c

cmach/mips/main.o:forth.h

############################################################################
# Advanced Camera (Mongoose processor) specific code.

camerafsrcs =	$(ARCH)/macro.fr $(ARCH)/prims.fr \
		$(ARCH)/satty.fr \
		$(ARCH)/mips/go.fr \
		$(ARCH)/lostart.fr \
		os.fr ../chips/silent.fr ../chips/lib/motor2.fr sa-start.fr \
		$(ARCH)/mips/camera/dict.fr
cameralobj =	jhforthc.o mips-tty.o
cameraobj =	$(cameralobj) cmach/mips/main.o
cameralink =	-T $(MIPSLIB)/elf32ebmip.x \
			-Ttext 90010000 -defsym __stacktop=0x90010000 \
			-oformat srec \
			-u _ramboot $(MIPSLIB)/startup-g.a \
			$(cameraobj) $(MIPSLIB)/camera-g.a

#cameratty =	cmach/mips/camera/mvpoll.c devices/polltty.c devices/forthio.c
cameratty =	cmach/mips/tty.c devices/forthio.c

cmach/mips/main.o:forth.h

############################################################################
# CRISP tracker (Mongoose processor) specific code.

trackerfsrcs =	$(ARCH)/macro.fr $(ARCH)/prims.fr \
		$(ARCH)/satty.fr \
		$(ARCH)/mips/go.fr \
		$(ARCH)/lostart.fr \
		os.fr ../chips/silent.fr ../chips/lib/motor2.fr sa-start.fr \
		$(ARCH)/mips/camera/dict.fr
# Uses camera dictionary sizing ...
trackerlobj =	jhforthc.o mips-tty.o
trackerobj =	$(trackerlobj) cmach/mips/main.o
trackerlink =	-T $(MIPSLIB)/elf32ebmip.x \
			-Ttext 90010000 -defsym __stacktop=0x90010000 \
			-oformat srec \
			-u _ramboot $(MIPSLIB)/startup-g.a \
			$(trackerobj) $(MIPSLIB)/tracker-g.a

#trackertty =	cmach/mips/camera/mvpoll.c devices/polltty.c devices/forthio.c
trackertty =	cmach/mips/tty.c devices/forthio.c

cmach/mips/main.o:forth.h

############################################################################
# UNANO (Mongoose processor) specific code.

unanofsrcs =	$(ARCH)/macro.fr $(ARCH)/prims.fr \
		$(ARCH)/satty.fr \
		$(ARCH)/mips/go.fr \
		$(ARCH)/lostart.fr \
		os.fr ../chips/silent.fr ../chips/lib/motor2.fr sa-start.fr \
		$(ARCH)/mips/camera/dict.fr
# Uses camera dictionary sizing ...
unanolobj =	jhforthc.o mips-tty.o
unanoobj =	$(unanolobj) cmach/mips/main.o
unanolink =	-T $(MIPSLIB)/elf32ebmip.x \
			-Ttext 90010000 -defsym __stacktop=0x90010000 \
			-oformat srec \
			-u _ramboot $(MIPSLIB)/startup-g.a \
			$(unanoobj) $(MIPSLIB)/unano-g.a

unanotty =	cmach/mips/tty.c devices/forthio.c

cmach/mips/main.o:forth.h

########################################################################
# Compile stand-alone tty

mips-tty.o:	io.h
mips-tty.c:	$(unixtty)
		cat $(unixtty) >mips-tty.c

########################################################################
# Compile Forth sources into CPP macros and C or assembly language.

FORTHSRCS =	$(unixfsrcs)

jhforthc.o:	defs.h io.h forth.h # TBD: missing filewrap.h in UNIX case
jhforthc.c:	$(ARCH)/macro.fr $(FORTHSRCS)
		./meta/forth2m $(F2MFLAGS) $(FORTHSRCS) >jhforthc.c

jhforths.o:	defs.h
jhforths.S:	$(ARCH)/macro.fr $(FORTHSRCS)
		./meta/forth2m $(F2MFLAGS) $(FORTHSRCS) >jhforths.S

############################################################################
# Compile C or assembly sources

CC =		gcc
CFLAGS =	-O3 -g
LD = 		gcc

OBJ =		$(unixobj)		# application objects only
LINK =		$(unixlink)		# flags, start & app objects, and libs

forth:		$(OBJ)
		$(LD) -o forth $(LINK)

############################################################################
# Make a library

AR =		ar

# TBD: junk
jhforthlib:	$(OBJ) filewrap.o
		ld -r -o forthlib.o $(OBJ) filewrap.o

lib:		unix.a

unix.a:	$(unixlobj)

############################################################################
# TBD

forth2m:	FORCE
		(cd meta; $(MAKE))
FORCE:

############################################################################
# Run test suite
sure:
		cd tests; ../forth load.fr

############################################################################
#TBD
unixtaskforth:	unix.o filewrap.o unixmem.o lostart.o ints.o intwrap.o

############################################################################
# Utilities

install:
		cp forth $(INSTALLAT)
		strip $(INSTALLAT)

uninstall:
		rm -f $(INSTALLAT)

clean:
		rm -f $(OBJ) forth		# remove objects
		rm -f jhforthc.c jhforths.S	# remove synthesized source
		rm -f mips-tty.c

distclean:	clean
		rm -f defs.h
		rm makefile			# remove this makefile!

########################################################################
# Add a rule to create a library
%.a:
		$(AR) r $@ $^
