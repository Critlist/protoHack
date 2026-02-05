#include "hack.h"
#include "../compat.h"
/* Original 1982: #include <sgtty.h> */
#include <stdio.h>
#include <signal.h>

extern char CALL[],OF[],QUIT[],ESCAPED[],*HIT[];
extern char *MISS[];
extern char WARROW[],WDART[],vowels[],DONTF[],NOBLUE[];
extern char READ[],WRITE[],DONTH[];
#ifdef VTONL
extern char CE[];
#else
extern char *CE;
#endif

void unsee(void)
{
	register int x,y;
	register struct rm *lev;
	struct monst *mtmp;

	if(u.ublind) return;
	if(seehx) {
		for(x=seelx;x<=seehx;x++)
			for(y=seely;y<=seehy;y++) {
				(lev= &levl[x][y])->cansee=0;
				if(lev->scrsym=='@') newsym(x,y);
				if((mtmp=g_at(x,y,fmon)) && mtmp->mstat<SLEEP &&
 mtmp->data->mlet==lev->scrsym) newsym(x,y);
			}
		seehx=0;
	} else {
		for(x=u.ux-1;x<u.ux+2;x++)
			for(y=u.uy-1;y<u.uy+2;y++) {
				(lev= &levl[x][y])->cansee=0;
				if(lev->scrsym=='@') newsym(x,y);
				else if((mtmp=g_at(x,y,fmon)) &&
 mtmp->mstat<SLEEP && mtmp->data->mlet==lev->scrsym) newsym(x,y);
				else if(lev->scrsym=='.') {
					lev->scrsym=' ';
					lev->new=1;
					on(x,y);
				}
			}
	}
}
void seeoff(int mode)
{	/* 1 means misc movement, 0 means blindness (Usually) */
	register int x,y;
	register struct rm *lev;

	if(seehx) {
		for(x=seelx;x<=seehx;x++)
			for(y=seely;y<=seehy;y++) {
				(lev= &levl[x][y])->cansee=0;
				if(mode && lev->scrsym=='@') newsym(x,y);
			}
		seehx=0;
	} else {
		for(x=u.ux-1;x<u.ux+2;x++)
			for(y=u.uy-1;y<u.uy+2;y++) {
				(lev= &levl[x][y])->cansee=0;
				if(mode) {
					if(lev->scrsym=='@') newsym(x,y);
					else if(lev->scrsym=='.') lev->seen=0;
				} else if(lev->scrsym=='.')
					lev->seen=0;
			}
	}
}
int movecm(char cmd)
{
	dx=dy=0;
	switch(cmd) {
	case 'h': dx= -1;
		break;
	case 'j': dy=1;
		break;
	case 'k': dy= -1;
		break;
	case 'l': dx=1;
		break;
	case 'y': dx=dy= -1;
		break;
	case 'u': dx=1;
		dy= -1;
		break;
	case 'b': dx= -1;
		dy=1;
		break;
	case 'n': dx=dy=1;
		break;
	default: return(0);
	}
	return(1);
}
void domove(void)
{
	char tmpx,tmpy;
	register struct monst *mtmp;
	register struct rm *tmpr,*ust;
	struct gen *gold,*trap,*gtm1;
	struct obj *otmp,*obj;
	char let;

	/* Modern: guard against out-of-bounds access when at map edges */
	if(u.ux+dx < 0 || u.ux+dx > 79 || u.uy+dy < 0 || u.uy+dy > 21) {
		flags.mv=multi=0;
		if(flags.mdone) pru();
		if(!u.uconfused) flags.move=0;
		return;
	}
	if(u.utrap) {
		if(u.upit) pline("You are still in a pit.");
		else pline("You are caught in a beartrap.");
		u.utrap--;
		return;
	}
	if(u.uconfused) {
		do {
			dx=rn1(3,-1);
			dy=rn1(3,-1);
		} while(!dx && !dy);
	}
	tmpr= &levl[u.ux+dx][u.uy+dy];
	ust= &levl[u.ux][u.uy];
	tmpx=u.ux;
	tmpy=u.uy;
	if(u.uswallow) {
		/* Original 1982: attmon(u.ustuck,uwep,0); — extra arg ignored in K&R */
		attmon(u.ustuck,uwep);
		if(multi) multi=0;
		return;
	}
	if(u.ustuck && (u.ux+dx!=u.ustuck->mx || u.uy+dy!=u.ustuck->my)) {
		k1("You cannot escape from %s%s!",u.ustuck->data->mname);
		if(multi) multi=0;
		return;
	}
	if((mtmp=g_at(u.ux+dx,u.uy+dy,fmon))) {
		nomul(0);
		if(u.ublind) let=amon(mtmp,uwep,0);
		else if(mtmp->sinv) {
			if(mtmp->data->mlet=='M') {
				pline("That's a %s!",mtmp->data->mname);
				if(!u.ustuck) u.ustuck=mtmp;
				mtmp->sinv=0;
				atl(mtmp->mx,mtmp->my,'M');
				return;
			} else if(mtmp->data->mlet=='p') {
				if(u.uac+5>rnd(20)) {
 pline("You are impaled by a falling piercer!");
					u.uhp-=d(4,6);
					flags.botl|=HP;
#ifndef SMALL
					if(u.uhp<1) killer=mtmp->data->mname;
#endif
				} else pline("You duck a falling piercer!");
				mtmp->invis=mtmp->sinv=0;
				atl(mtmp->mx,mtmp->my,'p');
			} else justswld(mtmp);
			return;
		} else if(!flags.mdone || ((!u.ucinvis) && mtmp->invis))
			let=amon(mtmp,uwep,0);
		return;
	}
	if(tmpr->typ<DOOR || ((trap=g_at(u.ux+dx,u.uy+dy,ftrap)) &&
 trap->gflag&SEEN && flags.mv)) {
		flags.mv=multi=0;
		if(flags.mdone) pru();
		if(!u.uconfused) flags.move=0;
		return;
	}
	if(dx && dy && (tmpr->typ==DOOR || ust->typ==DOOR)) {
		flags.move=flags.mv=multi=0;
		if(flags.mdone) pru();
		return;
	}
	u.ux+=dx;
	u.uy+=dy;
	flags.mdone=1;
	if(flags.mv>1) {
		if((xupstair==u.ux && yupstair==u.uy) || (xdnstair==u.ux &&
 ydnstair==u.uy)) nomul(0);
		if(!u.ublind) {
			if(levl[u.ux+dy][u.uy-dx].typ==DOOR ||
 levl[u.ux-dy][u.uy+dx].typ==DOOR) nomul(0);
#ifndef SMALL
			if(flags.mv>2) {
				if(levl[u.ux+dy][u.uy+dx].typ==CORR ||
 levl[u.ux-dy][u.uy-dx].typ==CORR || levl[u.ux+dx][u.uy+dy].typ<=DOOR) nomul(0);
			}
#endif
		}
		if(tmpr->typ==DOOR) {
#ifdef SMALL
			if(!u.ublind) multi=multi>80?multi-1:1;
#else
			if(flags.one && !u.ublind) multi=multi>80?multi-1:1;
#endif
			else nomul(0);
		}
	}
	if(ust->scrsym=='@') {
		newsym(tmpx,tmpy);
		oldux=tmpx;
		olduy=tmpy;
	}
	if(!u.ublind) {
		if(ust->lit) {
			if(tmpr->lit) {
				if(tmpr->typ==DOOR) prl1(u.ux+dx,u.uy+dy);
				else if(ust->typ==DOOR) nose1(tmpx-dx,tmpy-dy);
			} else {
				unsee();
				ust->cansee=1;
				prl1(u.ux+dx,u.uy+dy);
			}
		} else {
			if(tmpr->lit) setsee();
			else {
				prl1(u.ux+dx,u.uy+dy);
				if(tmpr->typ==DOOR) {
					if(dy) {
						prl(u.ux-1,u.uy);
						prl(u.ux+1,u.uy);
					} else {
						prl(u.ux,u.uy-1);
						prl(u.ux,u.uy+1);
					}
				}
			}
			nose1(tmpx-dx,tmpy-dy);
		}
	} else if(!ust->seen) {
		ust->cansee=0;
		ust->new=1;
		on(tmpx,tmpy);
	} else ust->cansee=0;
	if(!multi) pru();
	if((gold=g_at(u.ux,u.uy,fgold))) {
		if(gold->gflag<2) gold->gflag=2;
		pline("%u gold pieces.",gold->gflag);
		u.urexp+=gold->gflag;
		u.ugold+=gold->gflag;
		flags.botl|=GOLD;
		if(gold==fgold) fgold=fgold->ngen;
		else {
			for(gtm1=fgold;gtm1->ngen!=gold;gtm1=gtm1->ngen)
				;
			gtm1->ngen=gold->ngen;
		}
		mfree(gold);
		if(flags.mv>1) nomul(0);
		if(u.uinvis) newsym(u.ux,u.uy);
	}
	if((obj=g_at(u.ux,u.uy,fobj))) {
		for(otmp=invent,let=0;otmp;otmp=otmp->nobj) let+=weight(otmp);
		let+=weight(obj);
		if(let>85) {
#ifdef MAGIC
			if(!flags.magic)
#endif
				pline("Your pack is full.");
#ifdef MAGIC
			else {
				pline("Weight = %d",let);
				gobj(obj);
			}
#endif
		} else gobj(obj);
		if(flags.mv>1) nomul(0);
	}
	if(trap) {
		nomul(0);
		if(trap->gflag&SEEN && !rn2(6))
			pline("You escape a%s.",traps[trap->gflag&037]);
		else {
			trap->gflag|=SEEN;
			switch(trap->gflag&037) {
			case SLPTRP:
				pline("A cloud of gas puts you to sleep!");
				nomul(-rnd(25));
				break;
			case BEAR: u.utrap=rn1(4,4);
				u.upit=0;
				pline("A bear trap closes on your foot!");
				break;
			case ARROW:
				if(hitu(10,rnd(6),WARROW))
					pline("An arrow shot you!");
				else pline("You duck an arrow!");
				break;
			case TDOOR:
				if(!xdnstair) {
			pline("A trapdoor opens and a rock falls on you!");
#ifndef SMALL
					if(losehp(d(2,10))<1)
						killer="falling rock";
#else
					u.uhp-=d(2,10);
					flags.botl|=HP;
#endif
				} else {
 pline("A trap door opens up under you!");
					if(u.ufloat || u.ustuck) {
						pline(DONTF);
						break;
					}
					seeoff(1);
#ifndef SMALL
					fflush(stdout);
#endif
/* let the poor kid know what happened */
					do dodown();
					while(!rn2(4) && xdnstair);
					do {
						u.ux=rnd(79);
						u.uy=rn2(22);
					} while(levl[u.ux][u.uy].typ<ROOM ||
 g_at(u.ux,u.uy,fmon));
					setsee();
					docrt();
				}
				break;
			case DART:
				if(hitu(9,rnd(3),WDART)) {
 pline("A dart zaps out and hits you!");
					if(!rn2(6)) {
						poisoned(WDART);
#ifndef SMALL
						if(u.uhp<1)
							killer="poison dart";
#endif
					}
				} else
 pline("A dart wizzes by you and vanishes!");
				break;
			case TELE: newsym(u.ux,u.uy);
				tele();
				break;
			case PIT:
				if(u.ufloat) {
					pline("A pit opens up under you!");
					pline(DONTF);
					break;
				}
				pline("You fall into a pit!");
				u.utrap=rn1(6,2);
				u.upit=1;
#ifndef SMALL
				if(losehp(rnd(6))) killer="pit";
#else
				u.uhp-=rnd(6);
				flags.botl|=HP;
#endif
				break;
			default:
				pline("Bad trap %d",trap->gflag);
			}
		}
	}
}
void delmon(struct monst *mon)
{
	register struct monst *mtmp;

	if(mon==fmon) fmon=fmon->nmon;
	else {
		for(mtmp=fmon;mtmp->nmon!=mon;mtmp=mtmp->nmon) ;
		mtmp->nmon=mon->nmon;
	}
	mfree(mon);
}
/* Original 1982: pow(num) — renamed to avoid conflict with math.h built-in */
int hack_pow(int num) /* returns 2 to the num */
{
	register int tmp=1;

	while(num--) tmp*=2;
	return(tmp);
}
void tele(void)
{
	unsee();
	u.uswldtim=u.uswallow=u.utrap=0;
	u.ustuck=(struct monst *)0;
	do {
		u.ux=rn1(80,0);
		u.uy=rn1(22,0);
	} while(levl[u.ux][u.uy].typ<=DOOR || g_at(u.ux,u.uy,fmon) ||
 g_at(u.ux,u.uy,fobj) || g_at(u.ux,u.uy,ftrap) || g_at(u.ux,u.uy,fgold));
	setsee();
}
void doname(struct obj *obj, char *buf)
{
	switch(obj->olet) {
	case '"':
		strcpy(buf,"The amulet of Frobozz.");
		break;
	case '%':
 if(obj->quan>1) sprintf(buf,"%d %ss.",obj->quan,foodnam[obj->otyp]);
		else sprintf(buf,"a %s.",foodnam[obj->otyp]);
		break;
	case ')':
		if(obj->known) {
			if(obj->quan>1) sprintf(buf,"%d %c%d %ss",obj->quan,
 obj->minus?'-':'+',obj->spe,wepnam[obj->otyp]);
			else sprintf(buf,"a %c%d %s",obj->minus?'-':'+',
 obj->spe,wepnam[obj->otyp]);
		} else {
			if(obj->quan>1) sprintf(buf,"%d %ss",obj->quan,
 wepnam[obj->otyp]);
			else setan(wepnam[obj->otyp],buf);
		}
		while(*++buf) ;
		*buf++='.';
		*buf=0;
		break;
	case '[':
		if(obj->known) sprintf(buf,"a suit of %c%d %s armor",
 obj->minus?'-':'+',obj->spe,armnam[obj->otyp-2]);
		else sprintf(buf,"a suit of %s armor",armnam[obj->otyp-2]);
		if(obj==uarm) strcat(buf,"  (being worn)");
		while(*++buf) ;
		*buf++='.';
		*buf=0;
		break;
	case '!':
		if(oiden[obj->otyp]&POTN || potcall[obj->otyp]) {
			if(obj->quan>1) sprintf(buf,"%d potions",obj->quan);
			else sprintf(buf,"a potion");
			while(*buf) buf++;
			if(!potcall[obj->otyp])
				sprintf(buf,OF,pottyp[obj->otyp]);
			else sprintf(buf,CALL,potcall[obj->otyp]);
		} else {
			if(obj->quan>1) sprintf(buf,"%d %s potions.",obj->quan,
 potcol[obj->otyp]);
			else setan(potcol[obj->otyp],buf),
				strcat(buf," potion.");
		}
		break;
	case '?':
		if(obj->quan>1) sprintf(buf,"%d scrolls",obj->quan);
		else strcpy(buf,"a scroll");
		while(*buf) buf++;
		if(oiden[obj->otyp]&SCRN) sprintf(buf,OF,scrtyp[obj->otyp]);
		else if(scrcall[obj->otyp])
			sprintf(buf,CALL,scrcall[obj->otyp]);
		else sprintf(buf," labeled '%s'.",scrnam[obj->otyp]);
		break;
	case '/':
		if(oiden[obj->otyp]&WANN)
			sprintf(buf,"a wand of %s.",wantyp[obj->otyp]);
		else if(wandcall[obj->otyp])
			sprintf(buf,"a wand called %s.",wandcall[obj->otyp]);
		else setan(wannam[obj->otyp],buf),
			strcat(buf," wand.");
		if(obj->known) {
			while(*buf) buf++;
			sprintf(buf,"  (%d).",obj->spe);
		}
		break;
	case '=':
		if(oiden[obj->otyp]&RINN) {
			if(obj->known) sprintf(buf,"a %c%d ring of %s",
 obj->minus?'-':'+',obj->spe,ringtyp[obj->otyp]);
			else sprintf(buf,"a ring of %s",ringtyp[obj->otyp]);
		} else if(ringcall[obj->otyp])
			sprintf(buf,"a ring called %s",ringcall[obj->otyp]);
		else setan(rinnam[obj->otyp],buf),
			strcat(buf," ring");
		if(obj==uright) strcat(buf,"  (on right hand)");
		if(obj==uleft) strcat(buf,"  (on left hand)");
		while(*++buf) ;
		*buf++='.';
		*buf=0;
		break;
	case '*':
		if(obj->quan>1) sprintf(buf,"%d %s gems.",obj->quan,
 potcol[obj->otyp]);
		else {
			setan(potcol[obj->otyp],buf);
			strcat(buf," gem.");
		}
		break;
	default:
		strcpy(buf,"bug!");
	}
	if(obj==uwep) strcat(buf,"  (weapon in hand)");
}
int parse(void)
{
	register char foo;

	flags.move=1;
	curs(u.ux,u.uy+2);
	fflush(stdout);/*TODO: Fix double input bug in --More-- flush*/
	while((foo=getchar())>='0' && foo<='9')
		multi+=10*multi+foo-'0';
	/* Modern: ignore SIGQUIT control char left in input buffer (Ctrl-\) */
	if(foo=='\034') return(parse());
	if(foo== -1) {
		if(errno==EINTR) return(0);
		request_exit();
		return(0);
	}	/* Modern: EOF means tty vanished */
	if(multi) {
		multi--;
		save_cm=foo;
	}
	if(flags.topl) {
		home();
		cl_end();
	}
	flags.mdone=flags.topl=oldux=olduy=0;
	return(foo);
}
static volatile sig_atomic_t want_exit = 0;
static volatile sig_atomic_t want_hangup = 0;
static volatile sig_atomic_t want_quit = 0;
static void sig_exit(int);
static void sig_quit(int);

void set_exit_signals(void)
{
	struct sigaction sa;

	memset(&sa,0,sizeof(sa));
	sa.sa_handler = sig_exit;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGINT,&sa,(struct sigaction *)0);
	sigaction(SIGTERM,&sa,(struct sigaction *)0);
	sigaction(SIGHUP,&sa,(struct sigaction *)0);
}

void set_quit_signal(void)
{
	struct sigaction sa;

	memset(&sa,0,sizeof(sa));
	sa.sa_handler = sig_quit;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGQUIT,&sa,(struct sigaction *)0);
}

int exit_pending(void)
{
	if(want_exit) {
		want_exit=0;
		return(1);
	}
	return(0);
}

int hangup_pending(void)
{
	if(want_hangup) {
		want_hangup=0;
		return(1);
	}
	return(0);
}

int quit_pending(void)
{
	if(want_quit) {
		want_quit=0;
		return(1);
	}
	return(0);
}

void request_exit(void)
{
	want_exit = 1;
}

static void sig_exit(int signum)
{
	if(signum==SIGHUP || signum==SIGTERM) want_hangup=1;
	else want_exit=1;
}

static void sig_quit(int signum)
{
	(void)signum;
	want_quit = 1;
}
#ifdef MAGIC
static volatile sig_atomic_t want_magic = 0;
static void sig_magic(int);

void set_magic_signal(void)
{
	struct sigaction sa;

	memset(&sa,0,sizeof(sa));
	sa.sa_handler = sig_magic;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0; /* allow EINTR so magic triggers immediately */
	sigaction(SIGQUIT,&sa,(struct sigaction *)0);
}

int magic_pending(void)
{
	if(want_magic) {
		want_magic=0;
		return(1);
	}
	return(0);
}

static void sig_magic(int signum)
{
	(void)signum;
	want_magic = 1;
}

void domagic(int signum) /* Modern: used as signal handler, requires int param  */
{
	(void)signum;
	if(flags.magic) tellall();
	else if(getgid()==42) {
		flags.wmag=flags.magic=1;
		tellall();
	} else {
		home();
		cl_end();
		fflush(stdout);
		if(!strcmp(crypt(getpass("Magic Word: "),"JF"),
 "JFXum3WsO5NY.")) {
			flags.wmag=flags.magic=1;
			tellall();
		}
	}
}
void tellall(void)
{
	char *abuf;
	int ntimes;
	unsigned char tx,ty; /* Modern: unsigned char for safe array subscript use */
	char tcmd;
	char tbuf[15];
	char buf[80];
	register struct monst *mtmp;
	register struct gen *gtmp;
	register struct obj *otmp;
	struct stole *stmp;

#ifdef MAGIC
	set_magic_signal();
#endif
	home();
	putchar('*');
	cl_end();
	fflush(stdout);
	flags.topl=1;
	tcmd=getchar();
	ntimes=0;
	while(tcmd>='0' && tcmd<='9') {
		ntimes=ntimes*10 + tcmd-'0';
		tcmd=getchar();
	}
	if(ntimes<1) ntimes=1;
	if(index("O[)/?!%*=",tcmd)) {
		abuf=tbuf;
		while((*abuf=getchar())!=' ') abuf++;
		*abuf=0;
		abuf=tbuf;
	} else if(tcmd=='M') tx=getchar();
	if(ntimes>1) pline("%d times:",ntimes);
	while(ntimes-->0) {
	switch(tcmd) {
	case 's': pline("down %d,%d--up %d %d",xdnstair,ydnstair,xupstair,
 yupstair);
		break;
case 'p': pline("daminc=%d blind=%d fast=%d confused=%d",
 u.udaminc,u.ublind,u.ufast,u.uconfused);
		pline("invis=%d swldtim=%d urexp=%d",u.uinvis,u.uswldtim,
 u.urexp);
		break;
	case 'v':
		for(stmp=fstole;stmp;stmp=stmp->nstole) {
			if(stmp->sgold) pline("%s at %d %d stole %d gold.",
 stmp->smon->data->mname,stmp->smon->mx,stmp->smon->my,stmp->sgold);
			if(stmp->sobj) {
				sprintf(buf,"%s at %d %d stole .",stmp->smon->
 data->mname,stmp->smon->mx,stmp->smon->my);
				doname(stmp->sobj,index(buf,'.'));
				pline(buf);
				otmp=stmp->sobj;
				while((otmp=otmp->nobj)) {
					strcpy(buf,"and ");
					doname(otmp,&buf[4]);
					pline(buf);
				}
			}
		}
		break;
	case 'n':
		flags.magic=0;
		pline("Goodbye wizard...");
		return;
	case 'N':
		flags.magic=flags.wmag=0;
		pline("Goodbye...");
		return;
	case 'O':
		tx= *abuf;
		for(;;) {
			mkobj(tx);
			doname(fobj,buf);
			pline(buf);
			flags.topl=1;
			fflush(stdout);
			if(getchar()=='y') break;
			free(fobj);
			fobj=fobj->nobj;
		}
		fobj->ox=u.ux;
		fobj->oy=u.uy;
		break;
	case '?':
	case '!':
	case '/':
	case '[':
	case ')':
	case '*':
	case '%':
	case '=':
		mkobj(tcmd);
		if(*abuf) {
			fobj->otyp= *abuf-'a';
			if(*(abuf+1)) {
				fobj->quan=1+ *(abuf+1)-'a';
				if(*(abuf+2)) fobj->spe= *(abuf+2)-'a';
			}
		}
		fobj->ox=u.ux;
		fobj->oy=u.uy;
		doname(fobj,buf);
		pline(buf);
		break;
	case 'C':
		for(mtmp=fmon;mtmp;mtmp=mtmp->nmon)
			if(mtmp->cham) newcham(mtmp,&mon[rn1(6,1)][rn2(7)]);
		break;
	default:
		pline("Nothing done.");
		break;
	case 'U':
		pluslvl();
		break;
	case 'x':
		if(fobj) {
			mfree(fobj);
			fobj=fobj->nobj;
		}
		break;
	case 'X': while(fmon) {
			if(levl[fmon->mx][fmon->my].scrsym==fmon->data->mlet)
				newsym(fmon->mx,fmon->my);
			mfree(fmon);
			fmon=fmon->nmon;
		}
		break;
	case '<':
		seeoff(1);
		doup();
		setsee();
		docrt();
		break;
	case 'L':
		dlevel=flags.maze-1;
	case '>':
		seeoff(1);
		dodown();
		setsee();
		docrt();
		break;
	case 'I':
		for(tx=0;tx<20;tx++) oiden[tx]=0377;
		break;
	case 'T':
		tele();
		break;
	case 'u':
		if(u.ustr<118) {
			u.ustr=u.ustrmax=118;
			ndaminc();
		}
		if(u.uhp<10*u.ulevel) u.uhp=u.uhpmax=10*u.ulevel;
		flags.botl=1;
		break;
	case 'V': pline("strmax = %d",u.ustrmax);
		break;
	case 'M':
		if(tx!=' ') {
			struct permonst *foo;

			for(foo=mon;foo->mname;foo++) {
				if(foo->mlet==tx) {
					makemon(foo);
					break;
				}
			}
			if(!foo->mlet) makemon(0);
		} else makemon(0);
		mnexto(fmon);
		break;
	case 'm':
		if(fmon) {
			cls();
			pline("You at %d, %d moves %u hunger %d",u.ux,u.uy,
 moves,u.uhunger);
			for(mtmp=fmon;mtmp;mtmp=mtmp->nmon) {
 sprintf(buf,"%s at %d, %d with %d hp (%d). %c %c",mtmp->data->mname,mtmp->mx,
 mtmp->my,mtmp->mhp,mtmp->orig_hp,mtmp->mstat+'a',mtmp->mspeed+'a');
				if(mtmp->sinv) strcat(buf,"  (SP!)");
				if(mtmp->invis) strcat(buf,"  (Invis!)");
				if(mtmp->mcan) strcat(buf,"  (Can't!)");
				if(mtmp->cham) strcat(buf,"   (chameleon!)");
				pline(buf);
				curs(mtmp->mx,mtmp->my+2);
				putchar(mtmp->data->mlet);
				curx++;
			}
			more();
			docrt();
		}
		break;
	case '$':
		if(fgold) {
			cls();
			for(gtmp=fgold;gtmp;gtmp=gtmp->ngen) {
				pline("Gold at %d, %d worth %u",gtmp->gx,
 gtmp->gy,gtmp->gflag);
				curs(gtmp->gx,gtmp->gy+2);
				putchar('$');
				curx++;
			}
			more();
			docrt();
		}
		break;
	case 't':
		if(ftrap) {
			cls();
			for(gtmp=ftrap;gtmp;gtmp=gtmp->ngen) {
				pline("a%s at %d, %d %s",traps[gtmp->gflag&037],
 gtmp->gx,gtmp->gy,(gtmp->gflag&SEEN)?"seen":"not seen");
				curs(gtmp->gx,gtmp->gy+2);
				putchar('^');
				curx++;
			}
			more();
			docrt();
		}
		break;
	case 'o':
		if(fobj) {
			cls();
			for(otmp=fobj;otmp;otmp=otmp->nobj) {
				sprintf(buf,"At %d, %d %c%d .",otmp->ox,
 otmp->oy,otmp->minus?'-':'+',otmp->spe);
				doname(otmp,index(buf,'.'));
				pline(buf);
				curs(otmp->ox,otmp->oy+2);
				putchar(otmp->olet);
				curx++;
			}
			more();
			docrt();
		}
		break;
	case 'l':
		for(ty=0;ty<22;ty++)
			for(tx=0;tx<80;tx++)
				if(levl[tx][ty].typ) {
					if(levl[tx][ty].typ==SDOOR) {
						levl[tx][ty].typ=DOOR;
						newsym(tx,ty);
					}
					if(tx != u.ux || ty!=u.uy) prl(tx,ty);
				}
	}
	}
}
#endif
void done1(int signum) /* Modern: signal handlers require int param */
{
	(void)signum;
	signal(SIGINT,SIG_IGN);
	pline("Really quit?");
	fflush(stdout);
	if(getchar()!='y') return;
	done(QUIT);
}
void done(char *st1)
{
	register int x;
	int tmp;
	struct {
		int level;
		unsigned points;
		char *str;
		char death[20];
	} rec[10],*t1;
	char *recfile="record";
	FILE *rfile;
	register int flg;
	int rec_locked=0;

#ifdef MAGIC
	if(flags.magic && *st1=='d'){
		u.uhp=u.uhpmax;
		pline("For some reason you are still are alive.");
		u.uswldtim=flags.move=multi=0;
		flags.botl=1;
		return;
	}
#endif
#ifdef SMALL
	if(getgid()==42 && *st1=='d') {
		u.uhp=1;
		flags.botl|=HP;
		return;
	}
#endif
	signal(SIGINT,SIG_IGN);
#ifndef SMALL
	if(!uname) uname=getlogin();
#endif
	cbout();
	for(x=1;x<30;x++) {
		glo(x);
		if(unlink(lock)) break;
	}
#ifdef LOCKNUM
	if(getgid()!=42) {
		(*index(lock,'.'))=0;
		unlink(lock);
	}
#endif
	signal(SIGINT,SIG_DFL);
	cls();
#ifdef SMALL
	printf("Goodbye %s...\n\n",getlogin());
#else
	printf("Goodbye %s...\n\n",uname);
#endif
	if(*st1=='e') {
		struct obj *otmp;

		u.urexp+=150;
		for(otmp=fobj;otmp;otmp=otmp->nobj) {
			if(otmp->olet=='*') u.urexp+=otmp->quan*10*rnd(12);
			else if(otmp->olet=='\"') u.urexp+=5000;
		}
 printf("You escaped from the dungeon with %u points.\n",u.urexp);
#ifndef SMALL
		killer=st1;
#endif
	} else printf("You %s on dungeon level %d with %u points.\n",st1,dlevel,
 u.urexp);
#ifndef SMALL
	if(*st1=='q') killer=st1;
	printf("and %u pieces of gold, after %u moves.\n",u.ugold,moves);
 printf("You were level %d with a maximum of %d hit points when you %s.\n",
 u.ulevel,u.uhpmax,st1);
	rec_locked=modern_lock_record();
	if(!rec_locked) puts("Warning: record file lock unavailable.");
	if(!(rfile=fopen(recfile,READ))) puts("No record file");
	else {
		putchar('\n');
		/* Modern: zero-init so unread entries have safe defaults */
		memset(rec, 0, sizeof(rec));
		buf[0] = '\0';
		for(t1=rec;t1<&rec[10];t1++) {
 if(fscanf(rfile,"%d %u %[^,],%[^\n]",&t1->level,&t1->points,buf,t1->death) < 4) break;
			t1->str=alloc(strlen(buf)+1);
			strcpy(t1->str,buf);
		}
		flg=0;
#ifdef MAGIC
		if(u.urexp>rec[9].points && !flags.wmag) {
#else
		if(u.urexp>rec[9].points) {
#endif
			signal(SIGINT,SIG_IGN);
			puts("You made the top ten list!\n");
			fclose(rfile);
			if(!(rfile=fopen(recfile,WRITE)))
				panic("No record file\n");
			else {
			/* stick in new entry */
				for(tmp=8;rec[tmp].points<u.urexp && tmp>-1;
 tmp--)
					rec[tmp+1]=rec[tmp];
				tmp++; /* point to right place */
				rec[tmp].points=u.urexp;
				rec[tmp].level=dlevel;
				rec[tmp].str=uname;
				strcpy(rec[tmp].death,killer);
				flg++;
			}
		}
		puts("Number  Points   Name");
		for(t1=rec,tmp=1;t1<&rec[10];tmp++,t1++) {
			if(!t1->str) continue;  /* Modern: skip unread entries */
			printf("%2d    %6u  %s ",tmp,t1->points,t1->str);
			if(!strcmp(ESCAPED,t1->death))
				puts("escaped the dungeon");
			else if(!strcmp(t1->death,QUIT))
 printf("quit on dungeon level %d.\n",t1->level);
			else
 printf("was killed on dungeon level %d by %s %s.\n",t1->level,
 index(vowels,*t1->death)?"an":"a",t1->death);
			if(flg) fprintf(rfile,"%d %u %s,%s\n",t1->level,
 t1->points,t1->str,t1->death);
		}
		fclose(rfile);
	}
	if(rec_locked) modern_unlock_record();
#endif
	modern_unlock_game();
	exit(0);
}
void setsee(void)
{
	register int x,y;

	if(u.ublind) {
		pru();
		return;
	}
	if(!levl[u.ux][u.uy].lit) {
		seehx=0;
		for(y=u.uy-1;y<u.uy+2;y++)
			for(x=u.ux-1;x<u.ux+2;x++)
				if(x==u.ux && y==u.uy) pru();
				else prl(x,y);
	} else {
		for(seelx=u.ux;levl[seelx-1][u.uy].lit;seelx--);
		for(seehx=u.ux;levl[seehx+1][u.uy].lit;seehx++);
		for(seely=u.uy;levl[u.ux][seely-1].lit;seely--);
		for(seehy=u.uy;levl[u.ux][seehy+1].lit;seehy++);
		for(y=seely;y<=seehy;y++)
			for(x=seelx;x<=seehx;x++)
				if(x==u.ux && y==u.uy) pru();
				else prl(x,y);
	}
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
struct gen *
g_at(int x, int y, struct gen *ptr)
{
	while(ptr) {
		if(ptr->gx==x && ptr->gy==y) return(ptr);
		ptr=ptr->ngen;
	}
	return(0);
}
int abon(void)/* your bonus to hit (From strength) */
{
	if(u.ustr==3) return(-4);
	else if(u.ustr<6) return(-3);
	else if(u.ustr<8) return(-2);
	else if(u.ustr<17) return(-1);
	else if(u.ustr<69) return(0);	/* up to 18/50 */
	else if(u.ustr<118) return(1);
	else return(2);
}
int sgn(int num)
{
	if(num<0) return(-1);
	else return(num>0);
}
void losexp(void)
{
	register int num;

	pline("Goodbye level %d.",u.ulevel--);
	num=rnd(10);
	u.uhp-=num;
	u.uhpmax-=num;
	if(u.ulevel>1) u.uexp=15*hack_pow(u.ulevel-1);
	else u.uexp=5;
	flags.botl|=(HP|HPM|ULV|UEX);
}
void useup(struct obj *obj)
{
	register struct obj *otmp;

	if(obj->quan>1) {
		obj->quan--;
		return;
	} else if(obj==invent) invent=invent->nobj;
	else {
		for(otmp=invent;otmp->nobj!=obj;otmp=otmp->nobj) ;
		otmp->nobj=obj->nobj;
	}
	mfree(obj);
	if(obj==uwep) uwep=0;
}
struct obj *
getobj(char *let, char *word)
{
	register struct obj *otmp;
	register char ilet;
	int foo;

	foo=0;
	for(otmp=invent;otmp;otmp=otmp->nobj) {
		/* Modern: tolerate NULL let for "any item" queries */
		if(!let || index(let,otmp->olet)) foo++;
	}
	if(!invent || (let && *let!=')' && !foo)) {
		pline("You don't have anything to %s.",word);
		return(0);
	}
	for(;;) {
		pline("%s what (* for list)?",word);
		flags.topl=1;
		fflush(stdout);
		ilet=getchar();
		if(ilet=='\033') return(0);
		if(ilet=='*') {
			if(!let || foo>1) doinv(let);
			else {
				for(otmp=invent;otmp;otmp=otmp->nobj) {
					if(!let || index(let,otmp->olet)) prinv(otmp);
				}
			}
		} else {
			if(ilet>='A' && ilet<='Z') ilet=26+ilet-'A';
			else ilet-='a';
			for(otmp=invent;otmp && ilet;ilet--,otmp=otmp->nobj) ;
			if(!otmp) {
				pline(DONTH);
				continue;
			}
			break;
		}
	}
	if(let && *let!=')' && !index(let,otmp->olet)) {
		pline("You can't %s that.",word);
		return(0);
	} else return(otmp);
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
void prinv(struct obj *obj)
{
	register struct obj *otmp;
	register char ilet='a';

	for(otmp=invent;otmp!=obj;otmp=otmp->nobj)
		if(++ilet>'z') ilet='A';
	sprintf(buf,"%c - ",ilet);
	doname(otmp,&buf[4]);
	pline(buf);
}
int weight(struct obj *obj)
{
	switch(obj->olet) {
	case '"': return(2);
	case '[': return(8);
	case '%': if(obj->otyp) return(obj->quan);
	case '?': return(3*obj->quan);
	case '!': return(2*obj->quan);
	case ')': if(obj->otyp==8) return(4);
		if(obj->otyp<4) return(obj->quan/2);
		return(3);
	case '=': return(1);
	case '*': return(obj->quan);
	case '/': return(3);
	default: pline("Bad weight %c",obj->olet);
		return(0);
	}
}
void getlin(char *str)
{			/* also note that only delete will delete chars (BUG) */
	register char *ostr=str;

	flags.topl=1;
	fflush(stdout);
	for(;;) {
		*str=getchar();
		if(*str=='\177') {
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
void done2(int signum) /* Modern: signal handlers require int param */
{
	register int x;

	(void)signum;
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	for(x=1;x<30;x++) {
		glo(x);
		if(unlink(lock)) break;
	}
#ifdef LOCKNUM
	if(getgid()!=42) {
		(*index(lock,'.'))=0;
		unlink(lock);
	}
#endif
	cbout();
	cls();
	puts("Bye!\n\n");
	modern_unlock_game();
	exit(3);
}
void gobj(struct obj *obj)
{
	register struct obj *otmp;

	if(obj==fobj) fobj=fobj->nobj;
	else {
		for(otmp=fobj;otmp->nobj!=obj;otmp=otmp->nobj) ;
		otmp->nobj=obj->nobj;
	}
	if(!invent) {
		invent=obj;
		obj->nobj=0;
	} else for(otmp=invent;otmp;otmp=otmp->nobj) {
		if(otmp->otyp==obj->otyp && otmp->olet==obj->olet) {
			if(obj->otyp<4 && obj->olet==')' &&
 obj->quan+otmp->quan<128 && obj->spe==otmp->spe && obj->minus==otmp->minus) {
				otmp->quan+=obj->quan;
				mfree(obj);
				obj=otmp;
				break;
			} else if(index("%?!*",otmp->olet)) {
				otmp->quan+=obj->quan;
				mfree(obj);
				obj=otmp;
				break;
			}
		}
		if(!otmp->nobj) {
			otmp->nobj=obj;
			obj->nobj=0;
			break;
		}
	}
/* note that this loop never exits normally (We HOPE...) */
	prinv(obj);
	if(u.uinvis) newsym(u.ux,u.uy);
}
void ndaminc(void)
{	/* redo u.udaminc */
	u.udaminc=0;
	if(uleft && uleft->otyp==14)
		u.udaminc=uleft->minus?-uleft->spe:uleft->spe;
	if(uright && uright->otyp==14)
		u.udaminc+=uright->minus?-uright->spe:uright->spe;
	if(u.ustr<6) u.udaminc--;
	else if(u.ustr>=118) u.udaminc+=6;
	else if(u.ustr>108) u.udaminc+=5;
	else if(u.ustr>93) u.udaminc+=4;
	else if(u.ustr>18) u.udaminc+=3;
	else if(u.ustr>15) u.udaminc++;
}
/* Will you hit mtmp with obj.  Tmp is bonus to hit */
int amon(struct monst *mtmp, struct obj *obj, int tmp)
{
	tmp+= u.ulevel+mtmp->data->ac+abon();
	if(obj) {
		if(obj->olet=='/' && obj->otyp==3) tmp+=3;
		else if(obj->olet==')') {
			if(obj->minus) tmp-=obj->spe;
			else tmp+=obj->spe;
			if(obj->otyp==8) tmp--; /* two handed sword */
			else if(obj->otyp==9) tmp+=2; /* dagger */
		}
	}
	if(mtmp->mstat==SLEEP) {
		mtmp->mstat=0;
		tmp+=2;
	}
	if(mtmp->mstat==MFROZ) {
		tmp+=4;
		if(!rn2(16)) mtmp->mstat=0;/* they might wake up */
	}
	if(mtmp->mstat==FLEE) tmp+=2;
	if(tmp>=rnd(20)) return(attmon(mtmp,obj));
	else {
		if(obj!=uwep && obj->olet==')') miss(wepnam[obj->otyp],mtmp);
		else k1(MISS[rand()%3],mtmp->data->mname);
	}
	return(0);
}
int attmon(struct monst *mtmp, struct obj *obj)
{
	register int tmp;

	if(obj) {
		if(obj->olet=='/' && obj->otyp==3) tmp=rn1(6,4);
		else if(obj->olet==')') {
			if(index(mlarge,mtmp->data->mlet)) {
				tmp=rnd(wldam[obj->otyp]);
				if(obj->otyp==8) tmp+=d(2,6);
				else if(obj->otyp==6) tmp+=rnd(4);
			} else {
				tmp=rnd(wsdam[obj->otyp]);
				if(obj->otyp==6 || obj->otyp==4) tmp++;
			}
			if(obj->minus) tmp-=obj->spe;
			else tmp+=obj->spe;
		} else tmp=rnd(3);
	} else tmp=rnd(3);
	tmp+=u.udaminc;
	if(u.uswallow && mtmp->data->mlet=='P' && (tmp-=u.uswldtim)<1) {
		k1(HIT[rand()%3],mtmp->data->mname);
		return 0; /* Modern: explicit return value for int function */
	}
	if(tmp<1) tmp=1;
	mtmp->mhp-=tmp;
	if(mtmp->mhp<1) {
		killed(mtmp);
		return(0);
	}
	if(obj==uwep || obj->olet!=')') k1(HIT[rand()%3],mtmp->data->mname);
	else hit(wepnam[obj->otyp],mtmp);
	if(!rn2(25) && mtmp->mhp<mtmp->orig_hp/2) {
		mtmp->mstat=FLEE;
		if(u.ustuck==mtmp) u.ustuck=0;
	}
	if(u.umconf) {
		pline(NOBLUE);
		if(levl[mtmp->mx][mtmp->my].cansee)/* will be fixed */
 			pline("The %s appears confused.",mtmp->data->mname);
		mtmp->mstat=MCONF;
		u.umconf=0;
	}
	if(mtmp->data->mlet=='a' && obj==uwep) {
		if(rn2(2)) {
			kludge("You are splashed by %ss acid!","the blob'");
#ifndef SMALL
			if(losehp(rnd(6)))
				killer=mtmp->data->mname;
#else
			losehp(rnd(6));
#endif
		}
		if(!rn2(6) && uwep && uwep->olet==')') {
			pline("Your %s corrodes!",wepnam[uwep->otyp]);
			minusone(uwep);
		}
	}
	return(1);
}
