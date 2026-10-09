hack:	hack.o hack.cmd.o hack.cmdsub.o hack.fight.o hack.files.o hack.levl.o
hack:	hack.main.o hack.mon.o hack.move.o hack.obj.o hack.screen.o hack.see.o
	ld -X -i -o hack /lib/crt0.o ../rnd.o *.o -ltermlib -lc
hack.o hack.cmd.o hack.cmdsub.o hack.fight.o hack.files.o hack.levl.o:	hack.h
hack.main.o hack.mon.o hack.move.o hack.obj.o hack.screen.o hack.see.o:	hack.h
hack.main.o:	hack.vars
.c.o:	; cc -O -c $*.c
