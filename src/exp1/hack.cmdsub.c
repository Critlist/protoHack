#include "hack.h"
		/* small procedures to impliment commands */

losestr(num)
register num;
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
were(otmp)
register struct obj *otmp;
{
	strcpy(buf,"You were wearing ");
	doname(otmp,&buf[17]);
	pline(buf);
}
ringoff(obj)
register struct obj *obj;
{
	register tmp;

	if(obj->otyp<13 && ((uleft && obj->otyp==uleft->otyp) || (uright &&
obj->otyp==uright->otyp))) return;
	switch(obj->otyp) {
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
		for(tmp=gospe(obj);tmp;tmp--) {
			if(!gminus(obj)) {
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
	case 15: if(gminus(obj)) u.uac-=gospe(obj);
		else u.uac+=gospe(obj);
		flags.botl|=AC;
		break;
	case 16:
		if(gminus(obj)) {
			u.uhp+=gospe(obj);
			u.uhpmax+=gospe(obj);
		} else {
			u.uhp-=gospe(obj);
			u.uhpmax-=gospe(obj);
#ifndef SMALL
			if(u.uhp<1) killer="ring";
#endif
		}
		flags.botl|=(HP|HPM);
		break;
	}
}
struct monst *
bhit(ddir,range)	/* zap for monster in direction ddir */
{			/* sets global variable dir to end location */
			/* (by cheating) (KLUDGE) */
	register struct monst *mtmp;
	register struct lev *loc;

	if(u.uswallow) return(u.ustuck);
	loc=u.uloc;
	while(range--) {
		loc+=ddir;
		if(mtmp=g_at(loc,fmon)) {
			dir=loc;
			return(mtmp);
		}
		if(gettyp(loc)<CORR) {
			loc-=ddir;
			return(0);
		}
	}
	dir=loc;
	return(0);
}
buzz(type,sloc,dir)
register struct lev *sloc;
{
	register char range,let;
	struct monst *mon;

	if(u.uswallow) {
		pline("The %s rips into the %s.",fl[type],
 u.ustuck->data->mname);
		zhit(u.ustuck,type);
		return;
	}
	range=rn1(7,9);
	switch(abs(dir)){
	case 1: let='|';
		break;
	case 22: let='-';
		break;
	case 23: let='\\';
		break;
	case 21: let='/';
		break;
	}
	while(range-->0) {
		sloc+=dir;
		if(gettyp(sloc)) {
			at(getx(sloc),gety(sloc),let);
			on(sloc);
		}
		if(mon=g_at(sloc,fmon)) {
			if(rnd(20)<14+mon->data->ac) {
				hit(fl[type],mon);
				zhit(mon,type);
				range-=2;
			} else miss(fl[type],mon);
		} else if(sloc==u.uloc) {
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
		if(gettyp(sloc)<=DOOR) {
			if(getcan(sloc)) pline("The %s bounces!",fl[type]);
			dir= 0-dir;
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
	case 2: if(!index("WVZ",tmp)) sstat(mon,MFROZ);/* sleep*/
		break;
	case 3: if(!index("X&Ygf",tmp)) mon->mhp-=d(6,6);/* cold */
		if(index("&D~",tmp)) mon->mhp-=d(3,6);
		break;
	case 4: if(index("WVZ",tmp)) return;
		mon->mhp= -1;/* death*/
		break;
	}
	if(mon->mhp<1) killed(mon);
}
chwepon(color)
register char *color;
{
	pline("Your %s glows %s.",wepnam[uwep->otyp],color);
}
pluslvl()
{
	register num;

	pline("You feel more experienced.");
	num=rnd(10);
	u.uhpmax+=num;
	u.uhp+=num;
	u.uexp=(10*hack_pow(u.ulevel-1))+1;
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
	}
	u.uhunger=newhunger;
}
plusone(obj)
register struct obj *obj;
{
	rcursed(obj);
	if(gminus(obj)) {
		obj->spestuff--;
		if(!gospe(obj)) rminus(obj);
	} else obj->spestuff++;
}
minusone(obj)
register struct obj *obj;
{
	if(gminus(obj)) obj->spestuff++;
	else if(gospe(obj)) obj->spestuff--;
	else {
		sminus(obj);
		sospe(obj,1);
	}
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
	int x,y;

	curs(savx,1);
	puts(MORE);
	getxy(&x,&y,u.uloc-levl);
	curs(x,y+2);
#ifndef SMALL
	fflush(stdout);
#endif
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
		if(gcursed(obj)) {
			pline(CURSED);
			return;
		} else uwep=0;
	}
	if(obj==invent) invent=invent->nobj;
	else {
		for(otmp=invent;otmp->nobj!=obj;otmp=otmp->nobj) ;
		otmp->nobj=obj->nobj;
	}
	obj->oloc=u.uloc;
	obj->nobj=fobj;
	fobj=obj;
	strcpy(buf,"You dropped ");
	doname(fobj,&buf[12]);
	pline(buf);
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
	}
	pline("Save begins...");
	fflush(stdout);
	signal(SIGINT,SIG_IGN);
	signal(SIGALRM,SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	if((fp=fopen(SAVEF,WRITE))==0) {
		pline("Cannot open save file.");
		return;
	}
	savelev(fp);
	for(otmp=invent;otmp;otmp=otmp->nobj) {
		bwrite(fp,otmp,sizeof(struct obj));
		if(otmp==uarm) bwrite(fp,"a",1);
		else if(otmp==uwep) bwrite(fp,"w",1);
		else if(otmp==uleft) bwrite(fp,"l",1);
		else if(otmp==uright) bwrite(fp,"r",1);
		else bwrite(fp,"n",1);
	}
	bwrite(fp,nul,sizeof(struct obj));
	bwrite(fp,&flags,sizeof(struct flag));
	bwrite(fp,&dlevel,1);
	bwrite(fp,&moves,2);
	bwrite(fp,&u,sizeof(struct you));
	if(u.ustuck) {
		bwrite(fp,u.ustuck,sizeof(u.ustuck));
		delmon(u.ustuck);
	}
	bwrite(fp,mon,sizeof(struct permonst)*8*7);
	bwrite(fp,oiden,20);
	bwrite(fp,potcol,POTNUM*2); /*dangerous, but... */
	bwrite(fp,scrnam,SCRNUM*2);
	bwrite(fp,wannam,WANDNUM*2);
	bwrite(fp,rinnam,RINGNUM*2);
	rept(potcall,&potcall[POTNUM],fp);
	rept(scrcall,&scrcall[SCRNUM],fp);
	rept(wandcall,&wandcall[WANDNUM],fp);
	rept(ringcall,&ringcall[RINGNUM],fp);
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
#ifdef LOCKS
	if(lock[1]=='l') {
		*index(lock,'.')=0;
		unlink(lock);
	}
#endif
	puts("Be seeing you...\n");
	cbout();
	exit(1);
}
rept(begin,end,fp)
char **begin;
register char **end;
register FILE *fp;
{
	int tmp;
	while(begin!=end) {
		if(*begin) {
			tmp=strlen(*begin)+1;
			bwrite(fp,&tmp,2);
			bwrite(fp,&begin,2);
			bwrite(fp,*begin,tmp);
		}
		begin++;
	}
}
#endif
