#include "hack.h"
		/* procedure having to do with your movement */

movecm(cmd)
register char cmd;
{
#ifndef NONUM
	if(cmd>'0' && cmd <'5')
		return(dir=dirs[cmd-'1']);
	if(cmd>'5' && cmd <='9')
		return(dir=dirs[cmd-'2']);
#endif
	dir=0;
	switch(cmd) {
	case 'l': dir=22;
		break;
	case 'h': dir= -22;
		break;
	case 'j': dir=1;
		break;
	case 'k': dir= -1;
		break;
	case 'y': dir= -23;
		break;
	case 'u': dir=21;
		break;
	case 'b': dir= -21;
		break;
	case 'n': dir=23;
		break;
	default: return(0);
	}
	return(1);
}
domove()
{
	register struct lev *oldu,*nu;
	register struct monst *mtmp;
	struct gen *gold,*trap,*gtm1;
	struct obj *otmp,*obj;
	char let;

	if(u.utrap) {
		if(u.upit) pline("You are still in a pit.");
		else pline("You are caught in a beartrap.");
		u.utrap--;
		return;
	}
	if(u.uconfused) dir=dirs[rn2(8)];
	nu=u.uloc+dir;
	if(u.uswallow) {
		attmon(u.ustuck,uwep);
		if(multi) multi=0;
		return;
	}
	if(u.ustuck && nu!=u.ustuck->mloc) {
		k1("You cannot escape from %s%s!",u.ustuck->data->mname);
		if(multi) multi=0;
		return;
	}
	if(mtmp=g_at(nu,fmon)) {
		nomul(0);
		if(u.ublind) amon(mtmp,uwep,0);
		else if(gsinv(mtmp)) {
			if(mtmp->data->mlet=='M') {
				pline("That's a %s!",mtmp->data->mname);
				if(!u.ustuck) u.ustuck=mtmp;
				rsinv(mtmp);
				atl(nu,'M');
				return;
			} else if(mtmp->data->mlet=='p') {
				if(u.uac+5>rnd(20)) {
 pline("You are impaled by a falling piercer!");
					u.uhp-=d(4,6);
					flags.botl|=HP;
#ifndef SMALL
					if(u.uhp<1) killer="falling piercer";
#endif
				} else pline("You duck a falling piercer!");
				rinv(mtmp);
				rsinv(mtmp);
				atl(nu,'p');
			} else justswld(mtmp);
			return;
		} else if(!flags.mdone || ((!u.ucinvis) && ginv(mtmp)))
			amon(mtmp,uwep,0);
		return;
	}
	if(gettyp(nu)<DOOR || ((trap=g_at(nu,ftrap)) && trap->gflag&SEEN &&
 flags.mv)) {
		flags.mv=multi=0;
		if(flags.mdone) pru();
		if(!u.uconfused) flags.move=0;
		return;
	}
	oldu=u.uloc;
	u.uloc=nu;
	flags.mdone=1;
	if(flags.mv>1) {
		if(gettyp(nu)==DOOR) {
#ifdef SMALL
			if(!u.ublind) multi=multi>80?multi-1:1;
#else
			if(flags.one && !u.ublind) multi=multi>80?multi-1:1;
#endif
			else nomul(0);
		}
		if(nu==upstair || nu==dnstair) nomul(0);
		if(!u.ublind) {
			if(abs(dir)==1) {
#ifndef SMALL
				if(flags.mv>2 && (gettyp(nu+dir)==DOOR ||
 gettyp(nu+22)==CORR || gettyp(nu-22)==CORR)) nomul(0);
#endif
				if(gettyp(nu+22)==DOOR || gettyp(nu-22)==DOOR)
					nomul(0);
			} else {
#ifndef SMALL
				if(flags.mv>2 && (gettyp(nu+dir)==DOOR ||
 gettyp(nu+1)==CORR || gettyp(nu-1)==CORR)) nomul(0);
#endif
				if(gettyp(nu+1)==DOOR || gettyp(nu-1)==DOOR)
					nomul(0);
			}
		}
	}
	if(getscr(oldu)=='@') {
		newsym(oldu);
		ouloc=oldu;
	}
	if(!u.ublind) {
		if(getlit(oldu)) {
			if(getlit(nu)) {
				if(gettyp(nu)==DOOR) prl1(nu+dir,dir,prl);
				else if(gettyp(oldu)==DOOR)
					prl1(oldu-dir,-dir,nosee);
			} else {
				unsee(oldu);
				setcan(oldu);
				prl1(nu+dir,dir,prl);
			}
		} else {
			if(getlit(nu)) setsee();
			else {
				prl1(nu+dir,dir,prl);
				if(gettyp(nu)==DOOR) {
					if(abs(dir)!=22) {
						prl(nu-22);
						prl(nu+22);
					} else if(abs(dir)!=1) {
						prl(nu-1);
						prl(nu+1);
					}
				}
			}
			prl1(oldu-dir,-dir,nosee);
		}
	} else if(!(gseen(oldu))) {
		rescan(oldu);
		on(oldu);
	} else rescan(oldu);
	if(!multi) pru();
	if(gold=g_at(nu,fgold)) {
		if(gold->gflag<2) gold->gflag=2;
		pline("%u gold pieces",gold->gflag);
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
		if(u.uinvis) newsym(nu);
	}
	if(obj=g_at(nu,fobj)) {
		for(otmp=invent,let=0;otmp;otmp=otmp->nobj) let+=weight(otmp);
		let+=weight(obj);
		if(let>85) pline("Your pack is full.");
		else gobj(obj);
		if(flags.mv>1) nomul(0);
	}
	if(trap) {
		if(u.uinvis && !(trap->gflag&SEEN)) newsym(nu);
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
				if(!dnstair) {
			pline("A trapdoor opens and a rock falls on you!");
					u.uhp-=d(2,10);
					flags.botl|=HP;
#ifndef SMALL
					if(u.uhp<1) killer="falling rock";
#endif
				} else {
 pline("A trap door opens up under you!");
					if(u.ufloat || u.ustuck) {
 pline(DONTF);
						break;
					}
#ifndef SMALL
					fflush(stdout);
#endif
					seeoff(1);
					do {
						dodown();
						setscr(u.uloc,'<');
					} while(!rn2(4) && dnstair);
					do {
						u.uloc= &levl[rand()%80]
 [rand()%22];
					} while(gettyp(u.uloc) !=ROOM ||
 g_at(u.uloc,fmon));
					setsee();
					docrt();
				}
				break;
			case DART:
				if(hitu(9,rnd(3),WDART)) {
 					pline("A dart zaps out and hits you!");
					if(!rn2(6)) poisoned(WDART);
#ifndef SMALL
					if(u.uhp<1) killer="dart";
#endif
				} else
 pline("A dart wizzes by you and vanishes!");
				break;
			case TELE: newsym(u.uloc);
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
				u.uhp-=rnd(6);
				flags.botl|=HP;
#ifndef SMALL
				if(u.uhp<1) killer="pit";
#endif
				break;
			}
		}
	}
}
tele()
{
	unsee(u.uloc);
	u.ustuck=0;
	u.uswallow=u.utrap=0;
	do u.uloc= &levl[rn2(80)][rn2(22)];
	while(gettyp(u.uloc)!=(dnstair?ROOM:CORR) || g_at(u.uloc,fmon) ||
 g_at(u.uloc,ftrap));
	setsee();
}
prl1(loc,dir,proc)
register struct lev *loc;
register dir;
register int (*proc)();
{

	(*proc)(loc);
	switch(dir) {
	case -23: (*proc)(loc+22);
		(*proc)(loc+44);
		(*proc)(loc+1);
		(*proc)(loc+2);
		break;
	case -21: (*proc)(loc-2);
		(*proc)(loc-1);
		(*proc)(loc+22);
		(*proc)(loc+44);
		break;
	case 21: (*proc)(loc-44);
		(*proc)(loc-22);
		(*proc)(loc+1);
		(*proc)(loc+2);
		break;
	case 23: (*proc)(loc-2);
		(*proc)(loc-1);
		(*proc)(loc-44);
		(*proc)(loc-22);
		break;
	case 22:
	case -22: (*proc)(loc+1);
		(*proc)(loc-1);
		break;
	default: (*proc)(loc-22);
		(*proc)(loc+22);
		break;
	}
}
