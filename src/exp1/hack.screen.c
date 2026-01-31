#include "hack.h"
		/* stuff for updating the screen and moving the cursor */

extern char xbuf[];
curs(x,y)
register x,y;
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
		fputs(HO,stdout);
		curx=cury=1;
		nocm(x,y);
	}
#ifndef VTONL
	else if(!CM) nocm(x,y);
#endif
	else cm(x,y);
}
#ifndef SMALL
cm(x,y)
register x,y;
{
	fputs(tgoto(CM,x-1,y-1),stdout);
	curx=x;
	cury=y;
}
#endif
atl(loc,ch)
register struct lev *loc;
register char ch;
{
	setscr(loc,ch);
	on(loc);
}
on(loc)
struct lev *loc;
{
	int x,y;

	setnew(loc);
	getxy(&x,&y,loc-levl);
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
	y+=2;
	curs(x,y);
	putchar(ch);
	curx++;
}
docrt()
{
	register x,y;
	register struct lev *room;

	if(u.uswallow) swallowed();
	else {
		cls();
		for(y=0;y<22;y++)
			for(x=0;x<80;x++)
				if(getnew((room= &levl[x][y]))) {
					resnew(room);
					at(x,y,getscr(room));
					if(getscr(room)==' ') {
						rseen(room);
						setscr(room,'.');
					} else sseen(room);
				} else if(gseen(room)) at(x,y,getscr(room));
		scrlx=80;
		scrly=22;
		flags.dscr=scrhx=scrhy=0;
	}
	flags.botl=1;
	bot();
}
pru()
{
	setcan(u.uloc);
	if(u.uinvis) prl(u.uloc);
	else if(getscr(u.uloc)!='@') atl(u.uloc,'@');
}
prl(loc)
register struct lev *loc;
{
	register struct monst *mtmp;

	setcan(loc);
	if((!gettyp(loc)) || (gettyp(loc)<DOOR && gettyp(u.uloc)==CORR))
		return;
	if((mtmp=g_at(loc,fmon)) && ((!ginv(mtmp)) || u.ucinvis))
		atl(loc,mtmp->data->mlet);
	else if(!gseen(loc)) on(loc);
}
newsym(loc)
register struct lev *loc;
{
	register struct obj *otmp;
	register struct gen *gtmp;
	register tmp;

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
nosee(loc)
register struct lev *loc;
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
	if((!ginv(mon)) || u.ucinvis)
		atl(mon->mloc,mon->data->mlet);
}
nscr()
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
nocm(x,y)
register x,y;
{
	while (curx < x) {
		fputs(ND,stdout);
		curx++;
	}
	while (curx > x) {
		fputs(BC,stdout);
		curx--;
	}
	while (cury > y) {
		fputs(UP,stdout);
		cury--;
	}
	while(cury<y) {
		putchar('\n');
		cury++;
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
startup(nam)
register char *nam;
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
cls()
{
	fputs(CL,stdout);
	curx=cury=1;
	flags.topl=0;
}
home()
{
#ifndef VTONL
	if(!HO) curs(1,1);
	else fputs(HO,stdout);
#else
	fputs(HO,stdout);
#endif
	curx=cury=1;
}
/*VARARGS1*/
pline(line,a1,a2,a3,a4,a5,a6,a7,a8)
register char *line,*a1,*a2,*a3,*a4,a5,a6,a7,a8;
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
	fputs(CE,stdout);
#ifndef SMALL
	if(line==0) {
		if(!ptr) ptr="No message.";
		fputs(ptr,stdout);
		curx=savx;
	}
#endif
	if(index(line,'%')) {
		sprintf(pbuf,line,a1,a2,a3,a4,a5,a6,a7,a8);
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
