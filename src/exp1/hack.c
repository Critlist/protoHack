#include "hack.h"
		/* misc procedures */

/* returns 2 to the num */
/* Modern: rename to avoid libm pow() collision */
int hack_pow(int num)
{
	register int tmp=1;

	while(num--) tmp*=2;
	return(tmp);
}
int parse(void)
{
	register int foo;
	int x,y;

	flags.move=1;
	getxy(&x,&y,u.uloc-levl);
	curs(x,y+2);

/* so you can see where you are (When invis) */
#ifndef SMALL
	fflush(stdout);
#endif
#ifndef NONUM
	if((foo=getchar())=='0') {
#endif
		while((foo=getchar())>='0' && foo<='9')/*FOO*/
			multi+=10*multi+foo-'0';
#ifndef NONUM
	}
	if(foo=='.') return(getchar()-040);
#endif
	if(foo== -1) return(parse());	/* interrupted */
	if(multi) {
		multi--;
		save_cm=foo;
	}
	if(flags.topl) {
		home();
		cl_end();
	}
	ouloc=0;
	flags.mdone=flags.topl=0;
	return(foo);
}
void done(char *st1)
{
	register int x;
	register struct obj *otmp;
#ifndef SMALL
	struct {
		unsigned level,points;
		char *name,*death;
	} rec[10], *t1;
	FILE *rfile;
	char *nbuf,*kbuf; /* names and killers */
	char xbuf[130];
	int rec_locked=0;
#endif

	cbout();
	signal(SIGINT,SIG_IGN);
	for(x=1;;x++) {
		glo(x);
		if(unlink(lock)) break;
	}
#ifdef LOCKS
	(*index(lock,'.'))=0;
	unlink(lock);
#endif
	signal(SIGINT,SIG_DFL);
	cls();
#ifndef SMALL
	/* Modern: lock record file during read/write */
	rec_locked=modern_lock_record();
	if(!rec_locked) puts("Warning: record file lock unavailable.");
#endif
#ifndef SMALL
	if(!uname) uname=getlogin();
	printf("Goodbye %s...\n\n",uname);
#else
	printf("Goodbye %s...\n\n",getlogin());
#endif
	if(*st1=='e') {
		u.urexp+=150;
#ifndef SMALL
		killer=st1;
#endif
		for(otmp=invent;otmp;otmp=otmp->nobj) {
			if(otmp->olet=='*') u.urexp+=gquan(otmp)*10*rnd(12);
			else if(otmp->olet=='\"') u.urexp+=5000;
		}
 printf("You escaped from the dungeon with %u points.\n",u.urexp);
	} else printf("You %s on dungeon level %d with %u points.\n",st1,
 dlevel,u.urexp);
#ifndef SMALL
	if(*st1=='q') killer=st1;
	printf("and %u pieces of gold, after %u moves.\n",u.ugold,moves);
 printf("You were level %d with a maximum of %d hit points when you %s.\n\n",
 u.ulevel,u.uhpmax,st1);
	if(!(rfile=fopen("record",READ))) {
		puts("Cannot read record file.");
		exit(1);
	}
	nbuf=xbuf;
	kbuf=buf;
	memset(rec,0,sizeof(rec)); /* Modern: safe defaults for unread entries */
	for(t1=rec;t1<&rec[10];t1++) {
		if(fscanf(rfile,"%d %u %[^,],%[^\n]",&t1->level,&t1->points,
 nbuf,kbuf) < 4) break;
		t1->name=nbuf;
		t1->death=kbuf;
		while(*nbuf++) ;
		while(*kbuf++) ;
	}
	fclose(rfile);
	if(u.urexp>rec[9].points) {
		signal(SIGINT,SIG_IGN); /* don't screw up record file */
		puts("You made the top ten list!\n");
		if(!(rfile=fopen("record",WRITE))) {
			puts("Cannot write record file!.\n");
			printf("Final score %d.  Killer=%s",u.urexp,killer);
			exit(2);
		}
		for(t1= &rec[8];t1->points<u.urexp && t1>=rec;t1--)
			*(t1+1)=(*t1);
		t1++; /* so it points to the right place */
		t1->points=u.urexp;
		t1->level=dlevel;
		t1->name=uname;
		t1->death=killer;
	} else rfile=0;
	puts("Number   Points    Name");
	for(t1=rec;t1<&rec[10];t1++) {
		if(!t1->name) continue; /* Modern: skip unread entries */
		printf("%2d      %6u    %s ",t1-rec,t1->points,t1->name);
		if(!strncmp("es",t1->death,2)) puts("escaped the dungeon.");
		else if(!strcmp(QUIT,t1->death))
			printf("quit on dungeon level %d.\n",t1->level);
		else printf("was killed on dungeon level %d by a%c %s.\n",
 t1->level,index(vowels,*t1->death)?'n':0,t1->death);
		if(rfile) fprintf(rfile,"%d %u %s,%s\n",t1->level,t1->points,
 t1->name,t1->death);
	}
	if(rfile) fclose(rfile);
	if(rec_locked) modern_unlock_record();
#endif
	/* Modern: release game lock */
	modern_unlock_game();
	exit(0);
}
void done1(int signum)
{
	(void)signum;
	signal(SIGINT,SIG_IGN);
	pline("Really quit?");
#ifndef SMALL
	fflush(stdout);
#endif
	if(getchar()=='y') done(QUIT);
	signal(SIGINT,done1);
}
void done2(int signum)
{
	register int x;

	(void)signum;
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	for(x=1;x<30;x++) {
		glo(x);
		if(unlink(lock)) break;
	}
#ifdef LOCKS
	(*index(lock,'.'))=0;	/* this assumes that everyone is locked */
	unlink(lock);
#endif
	cbout();
	cls();
	puts("Bye\r\n\n");
	/* Modern: release game lock */
	modern_unlock_game();
	exit(125);
}
void nomul(int nval)
{
	if(multi<0) return;
	if(flags.mv) {
		if(!nval && multi>80) {/* if multiple capital, don't stop */
			multi--;
			return;
		}
		if(flags.mdone) pru();
	}
	multi=nval;
	flags.mv=0;
}
struct gen *g_at(struct lev *loc, struct gen *ptr)
{
	while(ptr) {
		if(ptr->gloc==loc) return(ptr);
		ptr=ptr->ngen;
	}
	return(0);
}
/**
 * MODERN ADDITION (2025): POSIX termios replaces V7 sgtty
 *
 * WHY: V7 sgtty.h/gtty()/stty() do not exist on modern systems
 *
 * HOW: tcgetattr/tcsetattr with ICANON/ECHO flags equivalent to
 *      CBREAK/ECHO/CRMOD
 *
 * PRESERVES: Original terminal mode semantics (cbreak for gameplay,
 *            canonical for normal I/O)
 * ADDS: POSIX termios compatibility
 */
#if 0
/* ORIGINAL 1982 CODE - preserved for reference */
cbout()
{
	struct sgttyb ttyp;

	gtty(0,&ttyp);
	ttyp.sg_flags&=~CBREAK;
	ttyp.sg_flags|=ECHO|CRMOD;
	stty(0,&ttyp);
}
cbin()
{
	struct sgttyb ttyp;

	gtty(0,&ttyp);
	ttyp.sg_flags|=CBREAK;
	ttyp.sg_flags&=~(ECHO|CRMOD);
	stty(0,&ttyp);
}
#endif
void cbout(void)
{
	struct termios ttyp;

	if(tcgetattr(0,&ttyp)!=0) return; /* Modern: tolerate missing tty */
	ttyp.c_lflag |= ICANON|ECHO;
	ttyp.c_oflag |= OPOST|ONLCR;
	tcsetattr(0,TCSADRAIN,&ttyp);
}
void cbin(void)
{
	struct termios ttyp;

	if(tcgetattr(0,&ttyp)!=0) return; /* Modern: tolerate missing tty */
	ttyp.c_lflag &= ~(ICANON|ECHO);
	ttyp.c_oflag |= OPOST|ONLCR; /* Modern: keep CRLF in raw mode for screen positioning */
	ttyp.c_cc[VMIN] = 1;
	ttyp.c_cc[VTIME] = 0;
	tcsetattr(0,TCSADRAIN,&ttyp);
}
void setan(char *str, char *buf)
{
	if(index(vowels,*str)) sprintf(buf,"an %s",str);
	else sprintf(buf,"a %s",str);
}
void getlin(char *str)	/* note that this uses writes to get around the buffering on */
			/* the standard output */
{			/* also note that only delete will delete chars (BUG) */
	register char *ostr=str;

#ifndef SMALL
	fflush(stdout);
#endif
	flags.topl=1;
	for(;;) {
		*str=getchar();/* if you dont, typeahead screws. */
		if(*str=='\177' || *str=='\b') {
			if(str!=ostr) {
				str--;
				write(1,"\b \b",3);
			} else write(1,"\007",1);
		} else if(*str=='\n' || *str=='\033' || *str=='\r') {
			*str=0;
			return;
		} else {
			write(1,str,1);
			str++;
			curx++;
		}
	}
}
void ndaminc(void)	/* redo u.udaminc */
{
	u.udaminc=0;
	if(uleft && uleft->otyp==14)
		u.udaminc=gminus(uleft)?-gospe(uleft):gospe(uleft);
	if(uright && uright->otyp==14)
		u.udaminc+=gminus(uright)?-gospe(uright):gospe(uright);
	if(u.ustr<6) u.udaminc--;
	else if(u.ustr>=118) u.udaminc+=6;
	else if(u.ustr>108) u.udaminc+=5;
	else if(u.ustr>93) u.udaminc+=4;
	else if(u.ustr>18) u.udaminc+=3;
	else if(u.ustr>15) u.udaminc++;
}
void shufl(char *base[], int num)
{
	char **tmp,*tmp1;
	int curnum;

	for(curnum=num-1;curnum>0;curnum--) {
		tmp= &base[rn2(curnum)];
		tmp1= *tmp;
		*tmp=base[curnum];
		base[curnum]=tmp1;
	}
}
char *alloc(int num)
{
	register char *val;

	if(!(val=malloc(num))) panic("%d too big\n",num);
	return(val);
}
void getret(void)
{
	fputs("\n\n--Hit space to continue--",stdout);
#ifndef SMALL
	fflush(stdout);
#endif
	while(getchar()!=' ') ;
}
void kludge(char *str, char *arg)
{
	if(u.ublind) pline(str,*str=='%'?IT:It);
	else pline(str,arg);
}
void k1(char *str, char *arg)
{
	if(u.ublind) pline(str,nul,*str=='%'?IT:It);
	else pline(str,*str=='%'?"The ":"the ",arg);
}
/*VARARGS1*/
/* Original 1982: panic(str,a1,a2,...) — converted to stdarg */
void panic(char *str, ...)
{
	va_list ap;

	cls();
	fputs("ERROR:  ",stdout);
	va_start(ap, str);
	vprintf(str, ap);
	va_end(ap);
	cbout();
	exit(100);
}
void getxy(int *x, int *y, int loc)
{
	register x1;

	x1=0;
	while(loc>22) loc-=22,x1++;
	*x=x1;
	*y=loc;
}
int getdir(void)
{
	register char foo;

	pline("What direction?");
#ifndef SMALL
	fflush(stdout);
#endif
	foo=getchar();
	flags.topl=1;
	return(movecm(foo));
}
int near(struct lev *loc1, struct lev *loc2, int range)
{
	register schar *tmp;

	do {
		for(tmp=dirs;*tmp;tmp++)
			if(loc2==loc1+((*tmp)*range)) return(*tmp);
	} while(--range);
	return(0);
}
#ifndef SMALL
void set1(char *str)
{
	register num;

	if(!strncmp(str,"no",2)) {
		num=0;
		str+=2;
	} else num=1;
	if(!strcmp(str,"one")) flags.one=num;
	else if(!strcmp(str,"step")) flags.step=num;
	else if(!strncmp(str,"name=",5)) {
		str+=5;
		if(uname) mfree(uname);
		uname=alloc(strlen(str)+1);
		strcpy(uname,str);
	} else if(!strncmp(str,"term=",5)) {
		startup(str+5);
		docrt();
	} else pline("Unknown option '%s'",str);
}
#endif
