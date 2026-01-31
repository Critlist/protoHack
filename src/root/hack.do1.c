#include <stdio.h>
#include <signal.h>
#include "hack.h"

extern char NOTHIN[],MMIS[],WAND[],WCLEV[],SAVEF[],ESCAPED[],CURSED[];
extern char READ[],WRITE[],MORE[],WEARI[],NOCOLD[];

#define bwrite(fp,loc,num) fwrite(loc,1,num,fp)
#define mread(fp,loc,num) fread(loc,1,num,fp)

char *fl[]= {
	MMIS,
	"bolt of fire",
	"sleep ray",
	"bolt of cold",
	"death ray"
};

FILE *fopen();
struct monst *bhit();
/* More various user do commands */

ringoff(obj)
register struct obj *obj;
{
	register tmp;

	if(obj->otyp<13 && ((uleft && obj->otyp==uleft->otyp) || (uright &&
obj->otyp==uright->otyp))) return;
	switch(obj->otyp) {
	case 0: break;
	case 1: u.utel=0;
		break;
	case 2: u.uregen=0;
		break;
	case 3: u.usearch=0;
		break;
	case 4: u.ucinvis=0;
		break;
	case 5: u.ustelth=0;
		break;
	case 6: u.ufloat=0;
		break;
	case 7: u.upres=0;
		break;
	case 8: u.uagmon=0;
		break;
	case 9: u.ufeed=0;
		break;
	case 10: u.ufireres=0;
		break;
	case 11: u.ucoldres=0;
		break;
	case 12: u.ucham=0;
		break;
	case 13:
		for(tmp=obj->spe;tmp;tmp--) {
			if(!obj->minus) {
				if(u.ustr>18) {
					u.ustr-=15;
					if(u.ustr<18) u.ustr=18;
				} else if(u.ustr>4) u.ustr--;
				if(u.ustrmax>18) {
					u.ustrmax-=15;
					if(u.ustrmax<18) u.ustrmax=18;
				} else u.ustrmax--;
			} else {
				if(u.ustr>17) {
					u.ustr+=15;
					u.ustrmax+=15;
				} else {
					u.ustr++;
					if(u.ustrmax>17) u.ustrmax+=15;
					else u.ustrmax++;
				}
			}
		}
		flags.botl|=STR;
	case 14:	/* falls through again */
		ndaminc();
		break;
	case 15: if(obj->minus) u.uac-=obj->spe;
		else u.uac+=obj->spe;
		flags.botl|=AC;
		break;
	case 16:
		if(obj->minus) {
			u.uhp+=obj->spe;
			u.uhpmax+=obj->spe;
		} else {
			u.uhp-=obj->spe;
			u.uhpmax-=obj->spe;
#ifndef SMALL
			if(u.uhp<1) killer="ring";
#endif
		}
		flags.botl|=(HP|HPM);
		break;
	}
}
hit(str,mon)
register char *str;
register struct monst *mon;
{
	if(!levl[mon->mx][mon->my].cansee) pline("The %s hits it.",str);
	else pline("The %s hits the %s.",str,mon->data->mname);
}
miss(str,mon)
register char *str;
register struct monst *mon;
{
	if(!levl[mon->mx][mon->my].cansee) pline("The %s misses it.",str);
	else pline("The %s misses the %s.",str,mon->data->mname);
}
#ifndef SMALL
findit()
{
	char num;
	register char zx,zy;
	register struct gen *gtmp,*gt1;
	char lx,hx,ly,hy;

	for(lx=u.ux;(num=levl[lx-1][u.uy].typ) && num!=CORR;lx--) ;
	for(hx=u.ux;(num=levl[hx+1][u.uy].typ) && num!=CORR;hx++) ;
	for(ly=u.uy;(num=levl[u.ux][ly-1].typ) && num!=CORR;ly--) ;
	for(hy=u.uy;(num=levl[u.ux][hy+1].typ) && num!=CORR;hy++) ;
	num=0;
	for(zy=ly;zy<=hy;zy++)
		for(zx=lx;zx<=hx;zx++) {
			if(levl[zx][zy].typ==SDOOR) {
				levl[zx][zy].typ=DOOR;
				atl(zx,zy,'+');
				num++;
			} else if(gtmp=g_at(zx,zy,ftrap)) {
				if(!gtmp->gflag&SEEN) {
					gtmp->gflag|=SEEN;
					atl(zx,zy,'^');
					num++;
				}
			} else if((gtmp=g_at(zx,zy,fgold)) && !gtmp->gflag) {
				if(!makemon(&mon[5][2])) {
					fmon->mx=zx;
					fmon->my=zy;
					atl(zx,zy,'M');
					num++;
				}
				if(gtmp==fgold) fgold=gtmp->ngen;
				else {
					for(gt1=fgold;gt1->ngen!=gtmp;
 gt1=gt1->ngen) ;
					gt1->ngen=gtmp->ngen;
				}
				mfree(gtmp);
			}
		}
	return(num);
}
#endif
struct monst *
bhit(ddx,ddy,range)
{			/* sets global variables dx and dy to end location */
			/* (KLUDGE) */
	register struct monst *mtmp;
	register x,y;

	if(u.uswallow) return(u.ustuck);
	x=u.ux;
	y=u.uy;
	while(range--) {
		x+=ddx;
		y+=ddy;
		if(mtmp=g_at(x,y,fmon)) {
			dx=x;
			dy=y;
			return(mtmp);
		}
		if(levl[x][y].typ<CORR) {
			dx=x-ddx;
			dy=y-ddy;
			return(0);
		}
	}
	dx=x;
	dy=y;
	return(0);
}
buzz(type,sx,sy,dx,dy)
register sx,sy;
{
	struct rm *lev;
	char range,let;
	struct monst *mon;

	if(u.uswallow) {
		pline("The %s rips into the %s.",fl[type],
 u.ustuck->data->mname);
		zhit(u.ustuck,type);
		if(u.ustuck->mhp<1) killed(u.ustuck);
		return;
	}
	range=rn1(7,9);
	if(dx==dy) let='\\';
	else if(dx && dy) let='/';
	else if(dx) let='-';
	else let='|';
	while(range-->0) {
		sx+=dx;
		sy+=dy;
		if((lev= &levl[sx][sy])->typ) {
			at(sx,sy,let);
			on(sx,sy);
			lev->new=1;
		}
		if(mon=g_at(sx,sy,fmon)) {
			if(rnd(20)<14+mon->data->ac) {
				zhit(mon,type);
				if(mon->mhp<1) killed(mon);
				else hit(fl[type],mon);
				range-=2;
			} else miss(fl[type],mon);
		} else if(sx==u.ux && sy==u.uy) {
			if(rnd(20)<11+u.uac) {
				range-=2;
				pline("The %s hits you!",fl[type]);
				switch(type) {
				case 0: u.uhp-=d(2,6);
					break;
				case 1: if(u.ufireres)
						pline("You don't feel hot!");
					else u.uhp-=d(6,6);
					break;
				case 2: nomul(-rnd(25));
					break;
				case 3: if(u.ucoldres) pline(NOCOLD);
					else u.uhp-=d(6,6);
					break;
				case 4: u.uhp= -1;
				}
#ifndef SMALL
				if(u.uhp<1) killer=fl[type];
#endif
			} else pline("The %s wizzes by you!",fl[type]);
		}
		if(lev->typ<=DOOR) {
			if(lev->cansee) pline("The %s bounces!",fl[type]);
			dx= 0-dx;
			dy=0-dy;
			range--;
		}
	}
}
zhit(mon,type)
register struct monst *mon;
register type;
{
	register char tmp;

	tmp=mon->data->mlet;
	switch(type) {
	case 0: mon->mhp-=d(2,6);/* magic mis */
		break;
	case 1: if(!index("&XDgiQ",tmp)) mon->mhp-=d(6,6);/* fire */
		if(index("Y&",tmp)) mon->mhp-=d(3,6);
		break;
	case 2: if(!index("WVZ",tmp)) mon->mstat=MFROZ;/* sleep*/
		break;
	case 3: if(!index("X&Ygf",tmp)) mon->mhp-=d(6,6);/* cold */
		if(index("&D~",tmp)) mon->mhp-=d(3,6);
		break;
	case 4: if(index("WVZ",tmp)) return;
		mon->mhp= -1;/* death*/
		break;
	}
}
dosearch()
{
	char x,y;
	struct gen *tgen;

	for(x=u.ux-1;x<u.ux+2;x++)
		for(y=u.uy-1;y<u.uy+2;y++)
			if(levl[x][y].typ==SDOOR && !rn2(7)) {
				levl[x][y].typ=DOOR;
				atl(x,y,'+');
				nomul(0);
			} else {
				for(tgen=ftrap;tgen;tgen=tgen->ngen)
					if(tgen->gx==x && tgen->gy==y &&
 (!rn2(8) || ((!u.usearch) && tgen->gflag&SEEN))) {
						nomul(0);
						pline("You find a%s",
 traps[tgen->gflag&037]);
						if(!(tgen->gflag&SEEN)) {
							tgen->gflag|=SEEN;
							atl(x,y,'^');
						}
					}
			}
}
#ifndef SMALL
dosave()
{
	register tmp;
	struct obj *otmp;
	char **foo;
	int bar;
	register FILE *fp,*ofp;

	if((fp=fopen("nosave",READ))!=0) {
		pline("Save is down right now.");
		fclose(fp);
#ifdef MAGIC
		if(!flags.magic) return;
#endif
	}
	pline("Save begins...");
	fflush(stdout);
	signal(SIGINT,SIG_IGN);
	signal(SIGALRM,SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	if((fp=fopen(SAVEF,WRITE))==0) panic("No save file\n");
	savelev(fp);
	for(otmp=invent;otmp;otmp=otmp->nobj) {
		bwrite(fp,otmp,sizeof(struct obj));
		if(otmp==uarm) bwrite(fp,"a",1);
		else if(otmp==uwep) bwrite(fp,"w",1);
		else if(otmp==uleft) bwrite(fp,"l",1);
		else if(otmp==uright) bwrite(fp,"r",1);
		else bwrite(fp,"n",1);
		mfree(otmp);
	}
	bwrite(fp,nul,sizeof(struct obj));
	bwrite(fp,&flags,sizeof(struct flag));
	bwrite(fp,&dlevel,1);
	bwrite(fp,&moves,2);
	bwrite(fp,&u,sizeof(struct you));
	bwrite(fp,mon,sizeof(struct permonst)*8*7);
	bwrite(fp,oiden,20);
	bwrite(fp,potcol,POTNUM*2); /*dangerous, but... */
	bwrite(fp,scrnam,SCRNUM*2);
	bwrite(fp,wannam,WANDNUM*2);
	bwrite(fp,rinnam,RINGNUM*2);
	for(foo=potcall;foo!= &potcall[POTNUM];foo++) {
		if(*foo) {
			bar=strlen(*foo)+1;
			bwrite(fp,&bar,2);
			bwrite(fp,&foo,2);
			bwrite(fp,*foo,bar);
		}
	}
	for(foo=scrcall;foo!= &scrcall[SCRNUM];foo++) {
		if(*foo) {
			bar=strlen(*foo)+1;
			bwrite(fp,&bar,2);
			bwrite(fp,&foo,2);
			bwrite(fp,*foo,bar);
		}
	}
	for(foo=wandcall;foo!= &wandcall[WANDNUM];foo++) {
		if(*foo) {
			bar=strlen(*foo)+1;
			bwrite(fp,&bar,2);
			bwrite(fp,&foo,2);
			bwrite(fp,*foo,bar);
		}
	}
	for(foo=ringcall;foo!= &ringcall[RINGNUM];foo++) {
		if(*foo) {
			bar=strlen(*foo)+1;
			bwrite(fp,&bar,2);
			bwrite(fp,&foo,2);
			bwrite(fp,*foo,bar);
		}
	}
	bwrite(fp,nul,2);
	for(tmp=1;;tmp++) {
		glo(tmp);
		if(tmp!=dlevel) {
			if((ofp=fopen(lock,READ))==0) break;
			getlev(ofp);
			fclose(ofp);
			savelev(fp);
		}
		unlink(lock);
	}
	fclose(fp);
	cls();
	if(lock[1]=='l') {
		*index(lock,'.')=0;
		unlink(lock);
	}
	puts("Be seeing you...\n");
	cbout();
	exit(1);
}
dorecover(fp)
register FILE *fp;
{
	register tmp;
	register FILE *nfp;
	struct obj tmpo,*otmp;
	int bar;
	char **foo;

	unlink(SAVEF);
	getlev(fp);
	mread(fp,&tmpo,sizeof(struct obj));
	while(tmpo.olet) {
		otmp=alloc(sizeof(struct obj));
		*otmp=tmpo;
		otmp->nobj=invent;
		invent=otmp;
		mread(fp,buf,1);
		switch(*buf) {
		case 'w': uwep=otmp;
			break;
		case 'r': uright=otmp;
			break;
		case 'l': uleft=otmp;
			break;
		case 'a': uarm=otmp;
		}
		mread(fp,&tmpo,sizeof(struct obj));
	}
	mread(fp,&flags,sizeof(struct flag));
	mread(fp,&dlevel,1);
	mread(fp,&moves,2);
	mread(fp,&u,sizeof(struct you));
	mread(fp,mon,sizeof(struct permonst)*8*7);
	mread(fp,oiden,20);
	mread(fp,potcol,POTNUM*2); /*dangerous, but... */
	mread(fp,scrnam,SCRNUM*2);
	mread(fp,wannam,WANDNUM*2);
	mread(fp,rinnam,RINGNUM*2);
	mread(fp,&bar,2);
	while(bar) {
		mread(fp,&foo,2);
		*foo=alloc(bar);
		mread(fp,*foo,bar);
		mread(fp,&bar,2);
	}
	for(tmp=1;;tmp++) {
		if(tmp==dlevel) continue;
		if(feof(fp) || getlev(fp)) break;
		glo(tmp);
		if((nfp=fopen(lock,WRITE))==0)
			panic("No temp file %s\n",lock);
		savelev(nfp);
		fclose(nfp);
	}
	if(tmp!=2) {/* 2 means you are still on the first level */
		rewind(fp);
		getlev(fp);
	}
	fclose(fp);
	docrt();
}
#endif
loseone(obj,x,y)
register struct obj *obj;
{
	register struct obj *otmp,*ot1;

	if(obj->quan>1) {
		obj->quan--;
		if((ot1=g_at(x,y,fobj)) && ot1->otyp==obj->otyp &&
 ot1->olet==obj->olet && obj->spe==ot1->spe && ot1->quan<31)
			ot1->quan++;
		else {
			otmp=alloc(sizeof(struct obj));
			*otmp= *obj;
			otmp->quan=1;
			otmp->nobj=fobj;
			fobj=otmp;
			otmp->ox=x;
			otmp->oy=y;
		}
	} else {
		if(obj==invent) invent=invent->nobj;
		else {
			for(otmp=invent;otmp->nobj!=obj;otmp=otmp->nobj) ;
			otmp->nobj=obj->nobj;
		}
		obj->nobj=fobj;
		fobj=obj;
		obj->ox=x;
		obj->oy=y;
	}
}
getdir()
{
	register char foo;

	pline("What direction?");
	fflush(stdout);
	foo=getchar();
	flags.topl=1;
	return(movecm(foo));
}
chwepon(color)
register char *color;
{
	pline("Your %s glows %s.",wepnam[uwep->otyp],color);
}
litroom()
{
	register num,zx,zy;

	if(levl[u.ux][u.uy].typ==CORR) {
		pline("The corridor glows briefly.");
		return;
	} else pline("The room is lit.");
	if(levl[u.ux][u.uy].lit) return;
	if(levl[u.ux][u.uy].typ==DOOR) {
		if(levl[u.ux][u.uy+1].typ==ROOM) zy=u.uy+1;
		else if(levl[u.ux][u.uy-1].typ==ROOM) zy=u.uy-1;
		else zy=u.uy;
		if(levl[u.ux+1][u.uy].typ==ROOM) zx=u.ux+1;
		else if(levl[u.ux-1][u.uy].typ==ROOM) zx=u.ux-1;
		else zx=u.ux;
	} else {
		zx=u.ux;
		zy=u.uy;
	}
	for(seelx=u.ux;(num=levl[seelx-1][zy].typ) && num!=CORR;seelx--) ;
	for(seehx=u.ux;(num=levl[seehx+1][zy].typ) && num!=CORR;seehx++) ;
	for(seely=u.uy;(num=levl[zx][seely-1].typ) && num!=CORR;seely--) ;
	for(seehy=u.uy;(num=levl[zx][seehy+1].typ) && num!=CORR;seehy++) ;
	for(zy=seely;zy<=seehy;zy++)
		for(zx=seelx;zx<=seehx;zx++) {
			levl[zx][zy].lit=1;
			if(!levl[zx][zy].cansee) prl(zx,zy);
		}
}
pluslvl()
{
	register num;

	pline("You feel more experienced.");
	num=rnd(10);
	u.uhpmax+=num;
	u.uhp+=num;
	u.uexp=(10*pow(u.ulevel-1))+1;
	pline(WCLEV,++u.ulevel);
	flags.botl|=(HP|HPM|ULV|UEX);
}
nothin(obj) /* strange feeling from potions and scrolls */
register struct obj *obj;
{
	pline("A strange feeling passes over you.");
	if(obj->olet=='?') {
		if((!(oiden[obj->otyp]&SCRN)) && (!scrcall[obj->otyp]))
			docall(obj);
	} else if((!(oiden[obj->otyp]&POTN)) && (!potcall[obj->otyp]))
		docall(obj);
	useup(obj);
}
lesshungry(num)
register num;
{
	register unsigned newhunger;

	newhunger=u.uhunger+num;
	if(u.uhunger<151 && newhunger>150) {
		if(u.uhunger<51 && u.ustr<u.ustrmax) losestr(-1);
		flags.botl|=DHS;
		u.uhs=0;
	} else if(u.uhunger<51 && newhunger>50) {
		pline("You only feel hungry now.");
		if(u.ustr<u.ustrmax) losestr(-1);
		flags.botl|=DHS;
		u.uhs=1;
	} else if(u.uhunger<1 && newhunger <50) {
		pline("You feel weak now.");
		flags.botl|=DHS;
		u.uhs=2;
	}
	u.uhunger=newhunger;
}
plusone(obj)
register struct obj *obj;
{
	obj->cursed=0;
	if(obj->minus) {
		if(!--obj->spe) obj->minus=0;
	} else obj->spe++;
}
minusone(obj)
register struct obj *obj;
{
	if(obj->minus) obj->spe++;
	else if(obj->spe) obj->spe--;
	else obj->minus=obj->spe=1;
}
doinv(str)
register char *str;
{
	register struct obj *otmp=invent;
	register char ilet='a';

#ifndef SMALL
	if(!flags.step) cls();
#else
	cls();
#endif
	while(otmp) {
		if(!str || index(str,otmp->olet)) {
			sprintf(buf,"%c -  ",ilet);
			doname(otmp,&buf[5]);
#ifndef SMALL
			if(flags.step) pline(buf);
			else {
#endif
				fputs(buf,stdout);
				fputs("\r\n",stdout);
#ifndef SMALL
			}
#endif
		}
		otmp=otmp->nobj;
		if(++ilet>'z') ilet='A';
	}
#ifndef SMALL
	if(!flags.step) {
#endif
		getret();
		docrt();
#ifndef SMALL
	}
#endif
}
dodown()
{
	register FILE *fp;

	glo(dlevel);
	fp=fopen(lock,WRITE);
	savelev(fp);
	fclose(fp);
	glo(++dlevel);
	if((fp=fopen(lock,READ))==0) {
		mklev();
		while((fp=fopen(lock,READ))==0) {
			pline("Cant mklev %s!",lock);
			fflush(stdout);
			mklev();
		}
	}
	getlev(fp);
	fclose(fp);
	u.ux=xupstair;
	u.uy=yupstair;
}
doup()
{
	register FILE *fp;

	if(dlevel==1) done(ESCAPED);
	glo(dlevel);
	fp=fopen(lock,WRITE);
	savelev(fp);
	fclose(fp);
	glo(--dlevel);
	if((fp=fopen(lock,READ))==0) panic("cannot open %s\n",lock);
	getlev(fp);
	fclose(fp);
	u.ux=xdnstair;
	u.uy=ydnstair;
}
docall(obj)
register struct obj *obj;
{
	register char *str;
	register char **str1;

	pline("Call it:");
	getlin(buf);
	flags.topl=1;
	if(*buf) {
		str=alloc(strlen(buf)+1);
		strcpy(str,buf);
	} else str=0;
	switch(obj->olet){
	case '?': str1= &scrcall[obj->otyp];
		break;
	case '!': str1= &potcall[obj->otyp];
		break;
	case '/': str1= &wandcall[obj->otyp];
		break;
	case '=': str1= &ringcall[obj->otyp];
		break;
	}
	if(*str1) mfree(*str1);
	*str1=str;
}
more()
{
	curs(savx,1);
	puts(MORE);
	curs(u.ux,u.uy+2);
	fflush(stdout);
	while(getchar()!=' ') ;
}
dodr1(obj)
register struct obj *obj;
{
	register struct obj *otmp;

	if(obj==uarm || obj==uright || obj==uleft) {
		pline(WEARI);
		multi=flags.move=0;
		return;
	}
	if(obj==uwep) {
		if(obj->cursed) {
			pline(CURSED);
			return;
		} else uwep=0;
	}
	if(obj==invent) invent=invent->nobj;
	else {
		for(otmp=invent;otmp->nobj!=obj;otmp=otmp->nobj) ;
		otmp->nobj=obj->nobj;
	}
	obj->ox=u.ux;
	obj->oy=u.uy;
	obj->nobj=fobj;
	fobj=obj;
}
