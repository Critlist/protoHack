#include <stdio.h>
#include <signal.h>
#include "hack.h"
#include "../compat.h"
#include "hack.vars"

char lock[11] = "alock";	/* long enough for login name */
/* note that lock is also used for temp file names */
#ifdef SIGWINCH
/* Modern: defer redraw until safe point in main loop */
static volatile int resize_pending=0;
void handle_resize(int signum)
{
	(void)signum;
	resize_pending=1;
	signal(SIGWINCH,handle_resize);
}
static void check_resize(void)
{
	if(!resize_pending) return;
	docrt();
	resize_pending=0;
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

char curx,cury,savx;

char xupstair,yupstair,xdnstair,ydnstair;
char seelx, seehx, seely, seehy;	/* corners of lit room */
char scrlx, scrhx, scrly, scrhy;	/* corners of new area on screen */
char save_cm;
char dlevel=1,dx,dy;	/* note that if you have unsigned chars, dx and dy will
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

int main(void)
{
#ifndef SMALL
	register char *sfoo,*tmp;
#endif
	register FILE *fp;

	/* Original 1982: signal(-1,1); — V7 idiom: apply to all signals, not portable */
	/* Original 1982: signal(16,SIG_IGN); — V7 signal 16, not meaningful on POSIX */
	if(chdir(HACKDIR)<0) { /* Modern: uses HACKDIR define instead of hardcoded path */
		write(2,"Cannot cd!\n\n",14);
		exit(3);
	}
 	srand(getpid());
#ifndef SMALL
	setbuf(stdout,obuf);
	sfoo=getenv("HACKOPTS");
	if(sfoo) {
		do {
			tmp=sfoo;
			if(sfoo=index(sfoo,',')) *sfoo++=0;
			set1(tmp);
		} while(sfoo);
	}
	if(getgid()!=42) {
#ifdef LOCKNUM
		lockcheck();
#else	/* no locks */
		strcpy(lock,getlogin());/* you might want to make this pid */
#endif
		signal(SIGINT,done1);
		signal(SIGQUIT,done2);
		signal(SIGTERM,fooexit); /* Original 1982: signal(15,...) */
		signal(SIGHUP,hangup); /* Modern: save on hangup */
#ifdef SIGWINCH
		signal(SIGWINCH,handle_resize);
#endif
		startup(getenv("TERM"));
		cls();
		fflush(stdout);
		if(fork()) wait(0);
		else execl("/bin/cat","cat","news",(char *)NULL); /* Modern: execl() list terminator for 64-bit safety */
		cbin();
		getret();
	} else {
#ifdef MAGIC
		signal(SIGINT,done1);
		signal(SIGTERM,fooexit); /* Original 1982: signal(15,...) */
		signal(SIGQUIT,domagic);
#else
		signal(SIGINT,done1);
		signal(SIGQUIT,done2);
#endif
		signal(SIGHUP,hangup); /* Modern: save on hangup */
#ifdef SIGWINCH
		signal(SIGWINCH,handle_resize);
#endif
		cbin();
		startup(getenv("TERM"));
#ifdef MAGIC
		if(sfoo=getenv("MAGIC")) {
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
	signal(SIGQUIT,done2);
	strcpy(lock,getlogin());
#endif
	signal(SIGINT,done1);
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
#ifdef SIGWINCH
		check_resize();
#endif
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
		if(!multi) {
			if(flags.dscr) nscr();
			if(flags.botl) bot();
			if(flags.mv) flags.mv=0;
			rhack(parse());
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
	fputs("\n\n--Hit space to continue--",stdout);
	fflush(stdout);
	while(getchar()!=' ') ;
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
