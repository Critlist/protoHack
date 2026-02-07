#include <stdio.h>
#include <signal.h>
#include "hack.h"
#include "../compat.h"
#include "hack.vars"

char lock[64] = "alock";	/* Modern: increased from 11 to fit longer usernames */
/* note that lock is also used for temp file names */
#ifdef SIGWINCH
/* MODERN ADDITION (2026): Terminal resize protection for modern WM events
 *
 * WHY: Window resizes corrupt fixed-coordinate rendering on modern terminals.
 *
 * HOW: SIGWINCH sets a flag; main loop polls size and forces a redraw or
 *      blocks with a size warning until 80x24 is restored.
 *
 * PRESERVES: Original 80x22 map and display logic.
 * ADDS: Stable behavior under resize.
 */
static volatile int resize_pending=0;
static int last_cols=0, last_rows=0;
void handle_resize(int signum)
{
	(void)signum;
	resize_pending=1;
	signal(SIGWINCH,handle_resize);
}
static void check_resize(void)
{
	struct winsize ws;
	int cols=0, rows=0;

	if(!resize_pending) return;
	resize_pending=0;
	if(ioctl(0,TIOCGWINSZ,&ws)==0) {
		cols=(int)ws.ws_col;
		rows=(int)ws.ws_row;
	}
	if(cols<=0 || rows<=0) {
		cols=80;
		rows=24;
	}
	if(cols<80 || rows<24) {
		for(;;) {
			fd_set rfds;
			struct timeval tv;

			cls();
			printf("\n\nTERMINAL TOO SMALL!\n");
			printf("Current: %dx%d, Required: 80x24\n",cols,rows);
			printf("Resize to continue. Q=quit, S=save\n");
			fflush(stdout);

			FD_ZERO(&rfds);
			FD_SET(0,&rfds);
			tv.tv_sec=0;
			tv.tv_usec=200000;
			if(select(1,&rfds,(fd_set *)0,(fd_set *)0,&tv)>0) {
				int ch=getchar();
				if(ch=='Q') { done1(0); return; }
				if(ch=='S') { dosave(); return; }
			}
			if(ioctl(0,TIOCGWINSZ,&ws)==0) {
				cols=(int)ws.ws_col;
				rows=(int)ws.ws_row;
			}
			if(cols>=80 && rows>=24) break;
		}
	}
	if(cols!=last_cols || rows!=last_rows) {
		docrt();
		pline("[Terminal resized to %dx%d - display refreshed]",cols,rows);
		last_cols=cols;
		last_rows=rows;
	}
}

static void poll_resize(void)
{
	struct winsize ws;
	int cols=0, rows=0;

	if(ioctl(0,TIOCGWINSZ,&ws)==0) {
		cols=(int)ws.ws_col;
		rows=(int)ws.ws_row;
	}
	if(cols<=0 || rows<=0) return;
	if(cols!=last_cols || rows!=last_rows) resize_pending=1;
}
#endif
#ifdef LOCKNUM
#if 0
/* ORIGINAL 1982 CODE - preserved for reference */
char safelock[] = "rtmp";
char perm[] = "perm";
#endif
#endif

extern char BLANK[],HUNG[],WEAK[],FAINT[];

/* Original 1982: FILE *fopen(); — now provided by <stdio.h> */

struct rm levl[80][22];
struct monst *fmon=0;
struct gen *fgold=0, *ftrap=0;
struct stole *fstole;
struct obj *fobj=0, *invent, *uwep, *uarm, *uright=0, *uleft=0;
struct flag flags;
struct you u;

/* Modern: coordinate variables changed to unsigned char for safe array subscript use */
unsigned char curx,cury;
char savx;

unsigned char xupstair,yupstair,xdnstair,ydnstair;
unsigned char seelx, seehx, seely, seehy;	/* corners of lit room */
unsigned char scrlx, scrhx, scrly, scrhy;	/* corners of new area on screen */
char save_cm;
char dlevel=1;
char dx,dy;	/* note that if you have unsigned chars, dx and dy will
screw up */

unsigned moves=0;

int multi=0;

/* Original 1982: int done1(),done2(); int domagic(); int fooexit(); */
/* Now provided by prototypes in hack.h */

char buf[100];

#ifndef SMALL
char obuf[BUFSIZ];
char *killer;
#endif

/* Modern: restore canonical/echoed terminal state on exit */
static void cleanup_tty(void)
{
	cbout();
}

/* Modern: handle deferred signals in normal flow */
static int handle_pending(void)
{
#ifdef MAGIC
	if(magic_pending()) {
		domagic(0);
		return(1);
	}
#endif
	if(hangup_pending()) {
		hangup(0);
		return(1);
	}
	if(quit_pending()) {
		done2(0);
		return(1);
	}
	if(exit_pending()) {
		done1(0);
		return(1);
	}
	return(0);
}

int main(void)
{
#ifndef SMALL
	register char *sfoo,*tmp;
#endif
	register FILE *fp;

	/* Original 1982: signal(-1,1); — V7 idiom: apply to all signals, not portable */
	/* Original 1982: signal(16,SIG_IGN); — V7 signal 16, not meaningful on POSIX */
	if(chdir(HACKDIR)<0) { /* Modern: uses HACKDIR define instead of hardcoded path */
		write(2,"Cannot cd!\n\n",12); /* Modern: was 14, string is 12 bytes */
		exit(3);
	}
 	srand(getpid());
	atexit(cleanup_tty);
#ifndef SMALL
	setbuf(stdout,obuf);
	sfoo=getenv("HACKOPTS");
	if(sfoo) {
		do {
			tmp=sfoo;
			if((sfoo=index(sfoo,','))) *sfoo++=0;
			set1(tmp);
		} while(sfoo);
	}
	if(getgid()!=42) {
#ifdef LOCKNUM
		lockcheck();
#else	/* no locks */
		strcpy(lock,getlogin());/* you might want to make this pid */
#endif
		set_exit_signals(); /* Modern: signal-safe exit/save dispatch */
		set_quit_signal(); /* Modern: signal-safe SIGQUIT for non-magic */
#ifdef SIGWINCH
		signal(SIGWINCH,handle_resize);
#endif
		startup(getenv("TERM"));
		resize_pending=1; /* Modern: force size check after startup */
		{
			struct winsize ws;
			if(ioctl(0,TIOCGWINSZ,&ws)==0) {
				last_cols=(int)ws.ws_col;
				last_rows=(int)ws.ws_row;
			}
		}
		cls();
		fflush(stdout);
		if(fork()) wait(0);
		else execl("/bin/cat","cat","news",(char *)NULL); /* Modern: execl() list terminator for 64-bit safety */
		cbin();
		getret();
	} else {
#ifdef MAGIC
		set_magic_signal(); /* Modern: signal-safe magic dispatch */
		set_exit_signals(); /* Modern: signal-safe exit/save dispatch */
#else
		set_exit_signals(); /* Modern: signal-safe exit/save dispatch */
		set_quit_signal(); /* Modern: signal-safe SIGQUIT for non-magic */
#endif
#ifdef SIGWINCH
		signal(SIGWINCH,handle_resize);
#endif
		cbin();
		startup(getenv("TERM"));
		resize_pending=1; /* Modern: force size check after startup */
		{
			struct winsize ws;
			if(ioctl(0,TIOCGWINSZ,&ws)==0) {
				last_cols=(int)ws.ws_col;
				last_rows=(int)ws.ws_row;
			}
		}
#ifdef MAGIC
		if((sfoo=getenv("MAGIC"))) {
			lockcheck();
			while(*sfoo) {
				switch(*sfoo++) {
				case 'm': flags.magic=1;
					break;
				case 'n': srand(*sfoo++);
					break;
				}
			}
		} else strcpy(lock,getlogin());
#else	/* nomagic*/
		strcpy(lock,getlogin());
#endif
	}
#else	/*small*/
	cbin();
	set_exit_signals(); /* Modern: signal-safe exit/save dispatch */
	set_quit_signal(); /* Modern: signal-safe SIGQUIT for non-magic */
	strcpy(lock,getlogin());
#endif
#ifndef SMALL
	strcat(SAVEF,getlogin());
	if((fp=fopen(SAVEF,READ))!=0) {
		puts("Restoring old save file...");
		fflush(stdout);
		dorecover(fp);
		flags.move=0;
	} else {
#endif
		flags.maze=rn1(5,25);
		shufl(wannam,WANDNUM);
		shufl(potcol,GEMNUM);
		shufl(rinnam,RINGNUM);
		shufl(scrnam,SCRNUM);
		invent=alloc(sizeof(struct obj));
		invent->olet='%';
		invent->otyp=0;
		invent->quan=2;
		invent->nobj=uwep=alloc(sizeof(struct obj));
		uwep->olet=')';
		uwep->otyp=4;
		uwep->nobj=uarm=alloc(sizeof(struct obj));
		uarm->olet='[';
		uarm->otyp=3;
		uarm->spe=uwep->spe=uarm->quan=uwep->quan=uarm->known=
uwep->known=1;
		uarm->nobj=uarm->cursed=uarm->minus=uwep->cursed=uwep->minus=0;
		u.uac=6;
		u.ulevel=1;
		u.uhunger=900;
		u.uhpmax=u.uhp=12;
		if(!rn2(20)) u.ustrmax=u.ustr=rn1(20,14);
		else u.ustrmax=u.ustr=16;
		ndaminc();
#ifndef SMALL
		flags.move=flags.one=1;
#else
		flags.move=1;
#endif
#ifdef SIGWINCH
		check_resize(); /* Modern: deferred resize check, runs every turn */
#endif
		glo(1);
		mklev();
		fp=fopen(lock,READ);
		getlev(fp);
		fclose(fp);
		u.ux=xupstair;
		u.uy=yupstair;
		cls();
		setsee();
		flags.botl=1;
#ifndef SMALL
	}
#endif
	for(;;) {
		if(flags.move) {
			if(!u.ufast || moves%2==0) {
				if(fmon) movemon();
				if(!rn2(60)) {
					makemon(0);
					fmon->mx=fmon->my=0;
					rloc(fmon);
				}
			}
			if(u.ufast && !--u.ufast)
				pline("You slow down.");
			if(u.uconfused && !--u.uconfused)
				pline("You feel less confused now.");
			if(u.ublind && !--u.ublind) {
				pline("You can see again.");
				setsee();
			}
			if(u.uinvis && !--u.uinvis) {
				pru();
				pline("You are no longer invisible.");
			}
			++moves;
			--u.uhunger;
			if((u.uregen || u.ufeed) && moves%2) u.uhunger--;
			if(u.uhp<1) {
				pline("You die...");
				done("died");
			}
			if(u.uhp<u.uhpmax) {
				if(u.ulevel>9) {
					if(u.uregen || !(moves%3)) {
						flags.botl|=HP;
						u.uhp+=rnd(u.ulevel-9);
						if(u.uhp>u.uhpmax)
							u.uhp=u.uhpmax;
					}
				} else if(u.uregen || (!(moves%(22-u.ulevel
 *2)))) {
					flags.botl|=HP;
					u.uhp++;
				}
			}
			if(u.utel && !rn2(85)) tele();
			if(u.usearch) dosearch();
			if(u.uhunger<151 && u.uhs==0) {
				pline("You are beginning to feel hungry.");
				u.uhs=1;
				flags.botl|=DHS;
			} else if(u.uhunger<51 && u.uhs==1) {
				pline("You are beginning to feel weak.");
				u.uhs=2;
				losestr(1);
				flags.botl|=DHS;
			} else if(u.uhunger<1) {
				pline("You faint from lack of food.");
				if(u.uhs!=3) {
					u.uhs=3;
					flags.botl|=DHS;
				}
				nomul(-20);
				u.uhunger=rn1(4,22);
			}
		}
		flags.move=1;
#ifdef SIGWINCH
		poll_resize();
		check_resize();
#endif
		if(handle_pending()) continue;
		if(!multi) {
			if(flags.dscr) nscr();
			if(flags.botl) bot();
			if(flags.mv) flags.mv=0;
			rhack(parse());
			if(handle_pending()) continue;
		} else if(multi<0) {
			if(!++multi) pline("You can move again.");
		} else {
			if(flags.mv) {
				if(multi<80) --multi;
				domove();
			} else {
				--multi;
				rhack(save_cm);
			}
		}
	}
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
/* when it works, replace this with a
	#define alloc(num) malloc(num)
*/
{
	register char *val;

	if(!(val=malloc(num))) panic("%d too big\n",num);
	return(val);
}
void losestr(int num)
{
	if(u.ustr>18) {
		u.ustr-=15*num;
		if(u.ustr<18) u.ustr=17;
	} else if(u.ustr>3) {
		u.ustr-=num;
		if(u.ustr<3) u.ustr=3;
	} else return;
	ndaminc();
	flags.botl|=STR;
}
void getret(void)
{
	int ch;

	fputs("\n\n--Hit space to continue--",stdout);
	fflush(stdout);
	while((ch=getchar())!=' ' && ch!='\n' && ch!='\r') ;
}
#ifdef LOCKNUM
void leave(int signum) /* Modern: signal handlers require int param */
{
	(void)signum;
	/* Modern: release flock()-based game lock on exit */
	modern_unlock_game();
	exit(1);
}
void lockcheck(void)
{
	extern int errno;
	register int fd;

	signal(SIGQUIT,leave);
	signal(SIGINT,leave);

	/* Modern: clean up stale locks and use flock()-based locking */
	modern_cleanup_locks();
	if(!modern_lock_game()) {
		puts("Try again in a minute.");
		exit(2);
	}
	getlock();
	fd=creat(lock,0600);
	if(fd== -1) panic("Cant creat %s\n",lock);
	else {
		int pid;

		pid=getpid();
		write(fd,&pid,sizeof(pid));
		close(fd);
	}
}
void getlock(void)
{
	register int i,fd;

	for(i=0;i<LOCKNUM;i++) {
		lock[0]++;
		if((fd=open(lock,0))<0) return;
		if(check(fd)) {
			close(fd);
			return;
		} else if(i==LOCKNUM-1) {
			puts("Too many hacks running now.");
			exit(0);
		}
	}
}
int check(int fd)
{
	extern int errno;
	register int i;
	int pid;

	read(fd,&pid,sizeof(pid));
	if(kill(pid,16) == -1 && errno==3) {
		unlink(lock);
		i=1;
		do glo(i++);
		while(unlink(lock)>=0);
		lock[5]=0;/* make it a null again */
		return(1);
	}
	return(0);
}
#endif
void glo(int foo)
{
	register char *tf;

	tf=lock;
	while(*tf && *tf!='.') tf++;
	*tf++='.';
	sprintf(tf,"%d",foo);
}
#ifdef MAGIC
void fooexit(int signum) /* Modern: signal handlers require int param */
{
	(void)signum;
	pline("You have been killed by a **THE HACKER**");
	pline("You have 15 seconds to save your game (If save is up...)");
	fflush(stdout);
	sleep(2);
	signal(SIGALRM,done2);
	alarm(15);
}
#endif
