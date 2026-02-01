#include "hack.h"
#include <stdio.h>
/* Modern: termcap prototypes for tgetent/tgetstr/tgetnum/tgetflag */
#ifndef VTONL
#include <termcap.h>
/* Modern: termios for termcap ospeed */
#include "../compat.h"
#endif
#ifndef VTONL
char tbuf[8192];	/* Modern: expand termcap buffer for longer modern entries */
char *HO, *CL, *CE, *UP, *CM, *ND, *BC;
/* Modern: use tputs() so termcap padding ($<..>) isn't printed literally */
static int hack_putc(int c) { return putchar(c); }
static void xputs(char *s)
{
	if(s) tputs(s,1,hack_putc);
}
static void cmov(int x, int y)
{
	if(!CM) panic("Cm %d %d->%d %d\n",curx,cury,x,y);
	xputs(tgoto(CM,x-1,y-1));
	cury = y;
	curx = x;
}
#else
char *CE="\033[K";
char *HO="\033[H";
char *CL="\033[H\033[J";
char *BC="\b";
char *ND="\033[C";
char *UP="\033M";
static void xputs(char *s)
{
	if(s) fputs(s,stdout);
}
static void cmov(int x, int y)
{
	printf("\033[%d;%dH",y,x);
	cury = y;
	curx = x;
}
#endif

extern char MORE[],HUNG[],WEAK[],FAINT[],BLANK[];
int curs();

/* Modern: guard against out-of-bounds screen updates on large terminals */
static int mapok(int x, int y)
{
	return(x>=0 && x<80 && y>=0 && y<22);
}

curs(x,y)
register x,y;
{
	if(y==cury && x==curx) return;	/* do nothing, gracefulx */
#ifndef VTONL
	if(!ND && (curx!=x || x<=3)) {
		cmov(x,y);
		return;
	}
	if(abs(cury-y)<=3 && abs(curx-x)<=3) nocm(x,y);/* too close */
	else if (x<=3 && abs(cury-y)<=3 || (!CM && x<abs(curx-x))) {
#else
	if(abs(cury-y)<=3 && abs(curx-x)<=3) nocm(x,y);/* too close */
	else if(x<=3 && abs(cury-y)<=3) {
#endif
		putchar('\r');/* return first */
		curx=1;
		nocm(x,y);
	} else if(HO && x<=3 && y<=3) {
		xputs(HO);
		curx=cury=1;
		nocm(x,y);
	}
#ifndef VTONL
	else if(!CM) nocm(x,y);
#endif
	else cmov(x,y);
}
swallowed()
{
	cls();
	curs(u.ux-1,u.uy+1);
	fputs("/-\\",stdout);
	curx+=3;
	curs(u.ux-1,u.uy+2);
	fputs("|@|",stdout);
	curx+=3;
	curs(u.ux-1,u.uy+3);
	fputs("\\-/",stdout);
	curx+=3;
}
#ifndef VTONL
startup(nam)
register char *nam;
{
	register char *tptr;
	char *tbufptr;
	char *tp;
	char *tmp="bcbscoliclceuposcmhond";

	tptr=alloc(2048); /* Modern: larger termcap entry buffer for modern systems */
	tbufptr=tbuf;
	if(!nam) nam="vt100";
	if(tgetent(tptr,nam)<1) panic("Unknown terminal type!\n");
	/* Modern: set termcap ospeed from termios for padding calculations */
	{
		struct termios ttyp;

		if(tcgetattr(0,&ttyp)==0) ospeed = cfgetospeed(&ttyp);
	}
	if(!(BC=tgetstr(tmp,&tbufptr))) {	
		if(!tgetflag(tmp+2)) panic("Terminal must backspace.");
		else BC=tbufptr;
		tbufptr+=2;
		*BC='\b';
	}
	if(tgetnum(tmp+4)<80 || tgetnum(tmp+6)<24)
		panic("Screen must be at least 24 by 80!\n");
	else if(!(CL=tgetstr(tmp+8,&tbufptr)) || !(CE=tgetstr(tmp+10,&tbufptr))
 || !(UP=tgetstr(tmp+12,&tbufptr)) || tgetflag(tmp+14))
		panic("Hack needs CL, CE, UP, and no OS.\n");
	CM=tgetstr(tmp+16,&tbufptr);
	HO=tgetstr(tmp+18,&tbufptr); 
	if(!(ND=tgetstr(tmp+20,&tbufptr))) panic("Hack needs ND\n");;
	if(tbufptr>&tbuf[8191]) pline("Too big...");
	mfree(tptr);
}
#endif
/*VARARGS1*/
panic(str,a1,a2,a3,a4,a5,a6)
char *str;
{
	cls();
	fputs("ERROR:  ",stdout);
	printf(str,a1,a2,a3,a4,a5,a6);
	cbout();
	exit(100);
}
cls()
{
	xputs(CL);
	curx=cury=1;
	flags.topl=0;
}
home()
{
	if(!HO) curs(1,1);
	else xputs(HO);
	curx=cury=1;
}
atl(x,y,ch)
register x,y;
{
	register struct rm *crm;

	if(!mapok(x,y)) return;
	crm= &levl[x][y];
	if(crm->seen && crm->scrsym==ch) return;
	crm->scrsym=ch;
	crm->new=1;
	on(x,y);
}
on(x,y)
register x,y;
{
	if(!mapok(x,y)) return;
	if(flags.dscr) {
		if(x<scrlx) scrlx=x;
		else if(x>scrhx) scrhx=x;
		if(y<scrly) scrly=y;
		else if(y>scrhy) scrhy=y;
	} else {
		flags.dscr=1;
		scrlx=scrhx=x;
		scrly=scrhy=y;
	}
}
at(x,y,ch)
register x,y;
char ch;
{
	if(!ch) return;
	if(!mapok(x,y)) return; /* Modern: guard screen output to map bounds */
	y+=2;
	curs(x,y);
	putchar(ch);
	curx++;
}
docrt()
{
	register x,y;
	register struct rm *room;

	if(u.uswallow) swallowed();
	else {
		cls();
		for(y=0;y<22;y++)
			for(x=0;x<80;x++)
				if((room= &levl[x][y])->new) {
					room->new=0;
					at(x,y,room->scrsym);
					if(room->scrsym==' ') {
						room->seen=0;
						room->scrsym='.';
					} else room->seen=1;
				} else if(room->seen) at(x,y,room->scrsym);
		scrlx=80;
		scrly=22;
		flags.dscr=scrhx=scrhy=0;
	}
	flags.botl=1;
	bot();
}
pru()
{
	/* Modern: track last displayed @ to keep redraws in sync on modern terminals */
	static char pudx = -1;
	static char pudy = -1;
	static char pudis = 0;

	if(!mapok(u.ux,u.uy)) return;
	if(pudis && (pudx!=u.ux || pudy!=u.uy) && mapok(pudx,pudy))
		newsym(pudx,pudy);
	if(!u.ublind) levl[u.ux][u.uy].cansee=1;
	if(u.uinvis) {
		prl(u.ux,u.uy);
		pudis=0;
	} else if(levl[u.ux][u.uy].scrsym!='@') {
		atl(u.ux,u.uy,'@');
		pudis=1;
		pudx=u.ux;
		pudy=u.uy;
	}
}
prl(x,y)
{
	register struct rm *room;
	register struct monst *mtmp;
	register struct obj *otmp;
	register struct gen *gtmp;

	if(!mapok(x,y)) return;
	if(x==u.ux && y==u.uy && !u.uinvis) {
		pru();
		return;
	}
	room= &levl[x][y];
	room->cansee=1;
	if((!room->typ) || (room->typ<DOOR && levl[u.ux][u.uy].typ==CORR))
		return;
	if((mtmp=g_at(x,y,fmon)) && ((!mtmp->invis) || u.ucinvis))
		atl(x,y,mtmp->data->mlet);
	else if(otmp=g_at(x,y,fobj))
		atl(x,y,otmp->olet);
	else if(gtmp=g_at(x,y,fgold))
		atl(x,y,'$');
	else if((gtmp=g_at(x,y,ftrap)) && (gtmp->gflag&SEEN))
		atl(x,y,'^');
	else if(!room->seen || room->scrsym==' ') {
		/* Modern: refresh unseen tiles to avoid stale symbols on modern terminals */
		room->new=room->seen=1;
		newsym(x,y);
		on(x,y);
	}
	room->seen=1;
}
newsym(x,y)
register x,y;
{
	register struct obj *otmp;
	register struct gen *gtmp;
	register struct rm *room;
	register tmp;

	room= &levl[x][y];
	if(otmp=g_at(x,y,fobj)) tmp=otmp->olet;
	else if(gtmp=g_at(x,y,fgold)) tmp='$';
	else if((gtmp=g_at(x,y,ftrap)) && (gtmp->gflag&SEEN)) tmp='^';
	else switch(room->typ) {
	case SDOOR:
	case WALL:
		if((room-1)->typ && (room-1)->typ!=CORR && (room+1)->typ &&
 (room+1)->typ!=CORR) tmp='|';
		else tmp='-';
		break;
	case DOOR: tmp='+';
		break;
	case ROOM: if(x==xupstair && y==yupstair) tmp='<';
		else if(x==xdnstair && y==ydnstair) tmp='>';
		else if(room->lit || room->cansee || u.ublind) tmp='.';
		else tmp=' ';
		break;
	case CORR: if(x==xupstair && y==yupstair) tmp='<';
		else tmp='#';
		break;
	default: tmp='`';
#ifndef SMALL
		pline("Bad newsym %d %d",x,y);
#endif
	}
	atl(x,y,tmp);
}
nosee(x,y)
register x,y;
{
	register struct rm *room;
	struct monst *mtmp;

	(room= &levl[x][y])->cansee=0;
	if((mtmp=g_at(x,y,fmon)) && mtmp->mstat<SLEEP && room->scrsym==
 mtmp->data->mlet) {
		newsym(x,y);
		return;
	}
	if(room->scrsym=='.' && !room->lit && !u.ublind) {
		if(room->new && (x!=oldux || y!=olduy)) room->new=0;
		else {
			room->scrsym=' ';
			room->new=1;
			on(x,y);
		}
	}
}
prl1(x,y)
register x,y;
{
	if(dx) {
		if(dy) {
			prl(x-(2*dx),y);
			prl(x-dx,y);
			prl(x,y);
			prl(x,y-dy);
			prl(x,y-(2*dy));
		} else {
			prl(x,y-1);
			prl(x,y);
			prl(x,y+1);
		}
	} else {
		prl(x-1,y);
		prl(x,y);
		prl(x+1,y);
	}
}
nose1(x,y)
register x,y;
{
	if(dx) {
		if(dy) {
			nosee(x,u.uy);
			nosee(x,u.uy-dy);
			nosee(x,y);
			nosee(u.ux-dx,y);
			nosee(u.ux,y);
		} else {
			nosee(x,y-1);
			nosee(x,y);
			nosee(x,y+1);
		}
	} else {
		nosee(x-1,y);
		nosee(x,y);
		nosee(x+1,y);
	}
}
/*VARARGS1*/
pline(line,arg1,arg2,arg3,arg4)
register char *line,*arg1,*arg2,*arg3,*arg4;
{
#ifdef SMALL
	char pbuf[60];
#else
	static char *ptr=0,pbuf[60];
#endif

	if(flags.topl==2) {
		curs(savx,1);
		fputs(MORE,stdout);
		curx+=8;
		fflush(stdout);
		while(getchar()!=' ') ;
	}
	else flags.topl=2;
	if(flags.dscr) {
		if(!u.uinvis && levl[u.ux][u.uy].scrsym!='@') pru();
		nscr();
	}
	if(flags.botl) bot();
	if(cury==1) putchar('\r');
	else home();
	/* Modern: use cl_end() for CE fallback handling */
	cl_end();
#ifndef SMALL
	if(line==0) {
		if(!ptr) ptr="No message.";
		fputs(ptr,stdout);
		curx=savx;
	}
#endif
	if(index(line,'%')) {
		sprintf(pbuf,line,arg1,arg2,arg3,arg4);
#ifndef SMALL
		ptr=pbuf;
#endif
		savx=strlen(pbuf);
		fputs(pbuf,stdout);
	} else {
		fputs(line,stdout);
#ifndef SMALL
		ptr=line;
#endif
		savx=strlen(line);
	}
	curx= ++savx;
}
/* Modern: use termcap-aware clear-to-end-of-line with fallback */
void cl_end(void)
{
	if(CE) {
		xputs(CE);
		return;
	}
	/* Modern: fallback when CE missing (clear to column 80) */
	{
		int cx=curx, cy=cury;
		while(curx<80) {
			putchar(' ');
			curx++;
		}
		curs(cx,cy);
	}
}
losehp(n)
register n;
{
	u.uhp-=n;
	flags.botl|=HP;
	return(u.uhp<1);
}
prustr()
{
	if(u.ustr>18) {
		if(u.ustr>117) fputs("18/00",stdout);
		else printf("18/%02d",u.ustr-18);
	} else printf("%-2d   ",u.ustr);
	curx+=5;
}
pmon(mon)
register struct monst *mon;
{
	if((!mon->invis) || u.ucinvis)
		atl(mon->mx,mon->my,mon->data->mlet);
}
nscr()
{
	register x,y;
	register struct rm *room;

	if(u.uswallow) return;
	for(y=scrly;y<=scrhy;y++)
		for(x=scrlx;x<=scrhx;x++)
			if((room= &levl[x][y])->new) {
				room->new=0;
				at(x,y,room->scrsym);
				if(room->scrsym==' ') {
					room->cansee=room->seen=0;
 					room->scrsym='.';
				}
				else room->seen=1;
			}
	flags.dscr=scrhx=scrhy=0;
	scrlx=80;
	scrly=22;
}
nocm(x,y)
	/* go x,y without cm (indirectly) */
register x,y;
{
	if(cury > y) {
		if(UP) {
			while(cury > y) {
				xputs(UP);
				cury--;
			}
		} else if(HO) {
			home();
			curs(x,y);
			return;
		}
	} else if(cury < y) {
		while(cury < y) {
			putchar('\n');
			cury++;
			curx=1; /* Modern: newline returns to column 1 with ONLCR */
		}
	}
	if(curx < x) {
		while(curx < x) {
			xputs(ND);
			curx++;
		}
	} else if(curx > x) {
		while(curx > x) {
			xputs(BC);
			curx--;
		}
	}
}
bot()
{
	if(flags.botl&ALL) {
		curs(1,24);
 printf("Level %-2d  Gold %-5u  Hp %3d(%d)",dlevel,u.ugold,u.uhp,u.uhpmax);
		if(u.uhpmax<10) fputs("  ",stdout);
		else if(u.uhpmax<100) putchar(' ');
		printf("Ac %-2d  Str ",u.uac);
		prustr();
		printf("  Exp %2d/%-5u",u.ulevel,u.uexp);
		if(u.uhs) {
			fputs("      ",stdout);
			switch(u.uhs) {
			case 1: fputs(HUNG,stdout);
				break;
			case 2: fputs(WEAK,stdout);
				break;
			case 3: fputs(FAINT,stdout);
				break;
			}
			curx=78;
		} else curx=64;
		flags.botl=0;
		return;
	}
	if(flags.botl&GOLD) {
		curs(16,24);
		curx=21;
		printf("%-5u",u.ugold);
	}
	if(flags.botl&HP) {
		curs(26,24);
		curx=29;
		printf("%3d",u.uhp);
	}
	if(flags.botl&HPM) {
		curs(30,24);
		printf("%d)",u.uhpmax);
		if(u.uhpmax<100) putchar(' ');
		curx=u.uhpmax<10?33:34;
	}
	if(flags.botl&AC) {
		curs(37,24);
		printf("%-2d",u.uac);
		curx=39;
	}
	if(flags.botl&STR) {
		curs(45,24);
		prustr();
		curx=50;
	}
	if(flags.botl&ULV) {
		curs(56,24);
		printf("%2d",u.ulevel);
		curx=58;
	}
	if(flags.botl&UEX) {
		curs(59,24);
		printf("%-5u",u.uexp);
		curx=64;
	}
	if(flags.botl&DHS) {
		curs(70,24);
		curx=78;
		switch(u.uhs) {
		case 0: fputs(BLANK,stdout);
			break;
		case 1: fputs(HUNG,stdout);
			break;
		case 2: fputs(WEAK,stdout);
			break;
		case 3: fputs(FAINT,stdout);
			break;
		}
	}
	flags.botl=0;
}
