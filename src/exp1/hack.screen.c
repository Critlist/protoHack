#include "hack.h"
		/* stuff for updating the screen and moving the cursor */

#ifndef VTONL
/* Modern: termcap prototypes for tgetent/tgetstr/tgetnum/tgetflag */
#include <termcap.h>
/* Modern: use tputs() so termcap padding ($<..>) isn't printed literally */
static int hack_putc(int c) { return putchar(c); }
static void xputs(char *s)
{
	if(s) tputs(s,1,hack_putc);
}
#else
static void xputs(char *s)
{
	if(s) fputs(s,stdout);
}
#endif

extern char xbuf[];

/* Modern: guard against out-of-bounds screen updates on large terminals */
static int mapok(int x, int y)
{
	return(x>=0 && x<80 && y>=0 && y<22);
}

void curs(int x, int y)
{
	if(y==cury && x==curx) return;	/* do nothing, gracefulx */
	if(abs(cury-y)<=3 && abs(curx-x)<=3) nocm(x,y);/* too close */
#ifdef VTONL
	else if(x<=3 && abs(cury-y)<=3) {
#else
	else if(x<=3 && abs(cury-y)<=3 || (!CM && x<abs(curx-x))) {
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
	else cm(x,y);
}
#ifndef SMALL
void cm(int x, int y)
{
	xputs(tgoto(CM,x-1,y-1));
	curx=x;
	cury=y;
}
#endif
void atl(struct lev *loc, char ch)
{
	int x,y;

	getxy(&x,&y,loc-levl);
	if(!mapok(x,y)) return;
	setscr(loc,ch);
	on(loc);
}
void on(struct lev *loc)
{
	int x,y;

	setnew(loc);
	getxy(&x,&y,loc-levl);
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
void at(int x, int y, char ch)
{
	if(!ch) return;
	if(!mapok(x,y)) return;
	y+=2;
	curs(x,y);
	putchar(ch);
	curx++;
}
void docrt(void)
{
	register x,y;
	register struct lev *room;

	if(u.uswallow) swallowed();
	else {
		cls();
		for(y=0;y<22;y++)
			for(x=0;x<80;x++) {
				if(!mapok(x,y)) continue;
				if(getnew((room= &levl[x][y]))) {
					resnew(room);
					at(x,y,getscr(room));
					if(getscr(room)==' ') {
						rseen(room);
						setscr(room,'.');
					} else sseen(room);
				} else if(gseen(room)) at(x,y,getscr(room));
			}
		scrlx=80;
		scrly=22;
		flags.dscr=scrhx=scrhy=0;
	}
	flags.botl=1;
	bot();
}
void pru(void)
{
	/* Modern: track last displayed @ to keep redraws in sync on modern terminals */
	static char pudx = -1;
	static char pudy = -1;
	static char pudis = 0;
	int x,y;

	getxy(&x,&y,u.uloc-levl);
	if(!mapok(x,y)) return;
	if(pudis && (pudx!=x || pudy!=y) && mapok(pudx,pudy))
		newsym(&levl[pudx][pudy]);
	setcan(u.uloc);
	if(u.uinvis) {
		prl(u.uloc);
		pudis=0;
	} else if(getscr(u.uloc)!='@') {
		atl(u.uloc,'@');
		pudis=1;
		pudx=x;
		pudy=y;
	}
}
void prl(struct lev *loc)
{
	register struct monst *mtmp;
	int x,y;

	getxy(&x,&y,loc-levl);
	if(!mapok(x,y)) return;
	if(loc==u.uloc && !u.uinvis) {
		pru();
		return;
	}
	setcan(loc);
	if((!gettyp(loc)) || (gettyp(loc)<DOOR && gettyp(u.uloc)==CORR))
		return;
	if((mtmp=g_at(loc,fmon)) && ((!ginv(mtmp)) || u.ucinvis))
		atl(loc,mtmp->data->mlet);
	else if(!gseen(loc) || getscr(loc)==' ') {
		/* Modern: refresh unseen tiles to avoid stale symbols on modern terminals */
		setnew(loc);
		sseen(loc);
		newsym(loc);
		on(loc);
	}
}
void newsym(struct lev *loc)
{
	register struct obj *otmp;
	register struct gen *gtmp;
	register int tmp;

	if(otmp=g_at(loc,fobj)) tmp=otmp->olet;
	else if(gtmp=g_at(loc,fgold)) tmp='$';
	else if((gtmp=g_at(loc,ftrap)) && (gtmp->gflag&SEEN)) tmp='^';
	else switch(gettyp(loc)) {
	case SDOOR:
	case WALL:
		if(gettyp((loc-1)) && gettyp((loc-1))!=CORR &&
 gettyp((loc+1)) && gettyp((loc+1))!=CORR) tmp='|';
		else tmp='-';
		break;
	case DOOR: tmp='+';
		break;
	case ROOM: if(loc==upstair) tmp='<';
		else if(loc==dnstair) tmp='>';
		else if(getlit(loc) || getcan(loc) || u.ublind) tmp='.';
		else tmp=' ';
		break;
	case CORR: if(loc==upstair) tmp='<';
		else tmp='#';
		break;
	}
	atl(loc,tmp);
}
void nosee(struct lev *loc)
{
	register struct monst *mtmp;

	rescan(loc);
	if((mtmp=g_at(loc,fmon)) && gstat(mtmp)<SLEEP && getscr(loc)==
 mtmp->data->mlet) {
		newsym(loc);
		return;
	}
	if(getscr(loc)=='.' && !getlit(loc) && !u.ublind) {
		if(getnew(loc) && loc!=ouloc) resnew(loc);
		else {
			setscr(loc,' ');
			on(loc);
		}
	}
}
void prustr(void)
{
	if(u.ustr>18) {
		if(u.ustr>117) fputs("18/00",stdout);
		else printf("18/%02d",u.ustr-18);
	} else printf("%-2d   ",u.ustr);
	curx+=5;
}
void pmon(struct monst *mon)
{
	if((!ginv(mon)) || u.ucinvis)
		atl(mon->mloc,mon->data->mlet);
}
void nscr(void)
{
	register int x,y;
	register struct lev *room;

	if(u.uswallow) return;
	for(y=scrly;y<=scrhy;y++)
		for(x=scrlx;x<=scrhx;x++)
			if(getnew((room= &levl[x][y]))) {
				resnew(room);
				at(x,y,getscr(room));
				if(getscr(room)==' ') {
					rescan(room);
					rseen(room);
 					setscr(room,'.');
				}
				else sseen(room);
			}
	flags.dscr=scrhx=scrhy=0;
	scrlx=80;
	scrly=22;
}
/* go x,y without cm (indirectly) */
void nocm(int x, int y)
{
	while (curx < x) {
		xputs(ND);
		curx++;
	}
	while (curx > x) {
		xputs(BC);
		curx--;
	}
	while (cury > y) {
		xputs(UP);
		cury--;
	}
	while(cury<y) {
		putchar('\n');
		cury++;
		curx=1; /* Modern: newline returns to column 1 with ONLCR */
	}
}
void bot(void)
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
		} else curx=66;
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
#ifndef VTONL
void startup(char *nam)
{
	char tptr[512];
	char *tbufptr;
	char *tp;
	char *tmp="bcbscoliclceuposcmhond";

	tbufptr=xbuf;
	if(!nam) nam="vt100";
	if(tgetent(tptr,nam)<1) panic("Unknown terminal type!\n");
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
	if(tbufptr>&xbuf[44]) pline("Too big...");
}
#endif
void cls(void)
{
	xputs(CL);
	curx=cury=1;
	flags.topl=0;
}
void home(void)
{
#ifndef VTONL
	if(!HO) curs(1,1);
	else xputs(HO);
#else
	xputs(HO);
#endif
	curx=cury=1;
}
/*VARARGS1*/
void pline(const char *line, ...)
{
	va_list ap;
#ifdef SMALL
	char pbuf[60];
#else
	static char *ptr=0,pbuf[60];
#endif

	if(flags.topl==2) {
		curs(savx,1);
		fputs(MORE,stdout);
		curx+=8;
#ifndef SMALL
		fflush(stdout);
#endif
		while(getchar()!=' ') ;
	}
	else flags.topl=2;
	if(flags.dscr) {
		if(!u.uinvis && getscr(u.uloc)!='@') pru();
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
		va_start(ap, line);
		vsnprintf(pbuf,sizeof(pbuf),line,ap);
		va_end(ap);
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
