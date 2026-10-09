# Hack makefile.
hack.new:	hack.new1.o hack.new2.o rnd.o hack.new3.o
	ld -X -o hack.new /lib/crt0.o rnd.o hack.new[123].o -lc
hack.new1.o:	hackfoo.h
hack.new2.o:	hackfoo.h
hack.new3.o:	hackfoo.h
all:	hack.new
hack:	hack.new
small:	hack.new
.c.o: ; cc -O -c $*.c
