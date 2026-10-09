# Hack makefile.
hack:	hack.o hack.lev.o hack.pri.o hack.do.o hack.do1.o hack.main.o
hack:	hack.mon.o rnd.o
	ld -X -i -o hack /lib/crt0.o rnd.o hack.o hack.lev.o hack.pri.o hack.do.o hack.do1.o hack.main.o hack.mon.o -ltermlib -lc
	chmod u+s hack
hack.o hack.lev.o hack.mon1.o hack.pri.o hack.do.o hack.do1.o:	hack.h
hack.main.o hack.mon.o: hack.h
hack.main.o:	hack.vars
mklev:	mklev.c hack.h
	cc -O mklev.c rnd.o
	rm mklev.o
.c.o: ; cc -c $*.c
all:	hack mklev
hack.new:	hack.new1.o hack.new2.o rnd.o hack.new3.o
	ld -X -o hack.new /lib/crt0.o rnd.o hack.new[123].o -lc
hack.new1.o:	hackfoo.h
hack.new2.o:	hackfoo.h
hack.new3.o:	hackfoo.h
small:	hack.new
