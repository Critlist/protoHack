#include "hack.h"
		/* monsters... */

#define r_full(loc) (gettyp(loc)<DOOR || g_at(loc,fmon) || loc==u.uloc)

delmon(mon)
register struct monst *mon;
{
	register struct monst *mtmp;

	if(mon==fmon) fmon=fmon->nmon;
	else {
		for(mtmp=fmon;mtmp->nmon!=mon;mtmp=mtmp->nmon) ;
		mtmp->nmon=mon->nmon;
	}
	mfree(mon);
}

/* you have just been hit by a vampire or wraith */
losexp()
{
	register num;

	pline("Goodbye level %d.",u.ulevel--);
	num=rnd(10);
	u.uhp-=num;
	u.uhpmax-=num;
	if(u.ulevel>1) u.uexp=15*pow(u.ulevel-1);
	else u.uexp=5;
	
	flags.botl|=(HP|HPM|ULV|UEX);
}
rloc(mtmp)
register struct monst *mtmp;
{
	register struct lev *loc;

	if(mtmp==u.ustuck) {
		u.ustuck=(struct monst *)0;
		u.uswallow=u.uswldtim=0;
	}
	if(getscr(mtmp->mloc)==mtmp->data->mlet)
		newsym(mtmp->mloc);

	do loc= &levl[rand()%80][rand()%22];
	while(gettyp(loc)<DOOR || g_at(loc,fmon) || loc==u.uloc);

	mtmp->mloc=loc;
	if(getcan(loc) && (u.ucinvis || (!ginv(mtmp))))
		atl(loc,mtmp->data->mlet);
}
movemon()
{
	register struct monst *mtmp;

	for(mtmp=fmon;mtmp;mtmp=mtmp->nmon) {
		if(gspeed(mtmp)!=MSLOW || !(moves%2)) dochug(mtmp);
		if(gspeed(mtmp)==MFAST) dochug(mtmp);
	}
}
justswld(mtmp)
register struct monst *mtmp;
{
	u.ustuck=mtmp;
	u.uswallow=flags.botl=1;
	if(gstat(mtmp)==SLEEP) sstat(mtmp,MNORM);
	swallowed();
	rsinv(mtmp);
	if(mtmp->data->mlet=='P') k1("%s%s swallows you!",mtmp->data->mname);
	else k1("%s%s engulfs you!",mtmp->data->mname);
	hits= -1;
}
youswld(dam,die)
register dam,die;
{
	u.uhp-=dam;
	if(u.uswldtim++ == die) u.uhp= -1;
	flags.botl|=HP;
	hits= -1;
}
dochug(mtmp)
register struct monst *mtmp;
{
	register struct permonst *mdat;
	register tmp;
	struct stole *stmp;

	if(gcham(mtmp) && !rn2(6)) newcham(mtmp,&mon[rn1(6,2)][rn2(8)]);
	mdat=mtmp->data;
#ifndef SMALL
	if(mdat->mlet=='V' && !rn2(10)) {
		mtmp->data= &vbat;
		if(getcan(mtmp->mloc)) atl(mtmp->mloc,'B');
	} else if(mdat== &vbat && u.ustuck!=mtmp && !rn2(10)) {
		mtmp->data= &mon[6][2];
		if(getcan(mtmp->mloc)) atl(mtmp->mloc,'V');
	}
#endif
	if((!moves%20 || index(mregen,mdat->mlet)) && mtmp->mhp<mtmp->orig_hp)
		mtmp->mhp++; /* regenerate monsters. */
	if(gstat(mtmp)==MFROZ || gsinv(mtmp))
		return; /* frozen monsters don't do anything. */
	if(gstat(mtmp)==SLEEP) {/* wake up a monster, or get out of here. */
		if(!u.ustelth && (getcan(mtmp->mloc)) &&(!rn2(7) || u.uagmon)) {
			sstat(mtmp,0);
		} else return;
	}
	if(mdat->mmove>=rnd(12) && (gstat(mtmp)==FLEE || !near(mtmp->mloc,
 u.uloc,1)) && m_move(mtmp) && mdat->mmove<=12)
		return; /* move monsters and exit if rate<=12" */
	if(u.uswallow) {
		if(mtmp==u.ustuck) {
			switch(mtmp->data->mlet) {
			case ',':
				k1(CRUSH,mtmp->data->mname);
				youswld(4+u.uac,5);
				break;
			case '~':
				k1(CRUSH,mtmp->data->mname);
				youswld(rnd(6),7);
				break;
			default: 
				k1("%s%s digests you!",mtmp->data->mname);
				youswld(d(2,4),12);
				break;
			}
#ifndef SMALL
			if(u.uhp<1) killer=mdat->mname;
#endif
		}
		return;
	}
	if(mdat->mlet!='a' && near(mtmp->mloc,u.uloc,1)) {
		hits=0;
		nomul(0);
		if(!index("EyF&D",mdat->mlet))
			hitu(mdat->mhd,d(mdat->damn,mdat->damd),mdat->mname);
		switch(mdat->mlet) {
		case '&':
			if(!gcan(mtmp) && !rn2(15)) {
				if(!rn2(3)) break;
				makemon(&mon[7][6]);
				mnexto(fmon);
				hits= -1;
			} else {
				hitu(10,d(2,6),mdat->mname);
				hitu(10,d(2,6),mdat->mname);
				hitu(10,rnd(3),mdat->mname);
				hitu(10,rnd(3),mdat->mname);
				hitu(10,rn1(4,2),mdat->mname);
			}
			break;
		case '~':
		case ',': if(hits && !gcan(mtmp)) {
				mhit(mdat->mname);
				justswld(mtmp);
			}
			break;
		case 'A': if(!gcan(mtmp) && hits && rn2(2)) {
				mhit(mdat->mname);
				pline("You feel weaker!");
				losestr(1);
			}
			break;
#ifndef SMALL
		case 'B':
			if(mdat!= &vbat) break;
			if(mtmp==u.ustuck) {
				k1("%s%s sucks more blood out of you!",
 mdat->mname);
				u.uhp-=d(2,10);
			} else if(hits && !gcan(mtmp)) {
				k1("%s%s bites you on the neck!",mdat->mname);
				u.uhp-=d(2,4);
				u.ustuck=mtmp;
			}
			break;
#endif
		case 'C': hitu(4,rnd(6),mdat->mname);
			break;
		case 'c': if(!gcan(mtmp) && hits && rn2(2)) {
				mhit(mdat->mname);
				if(rn2(5)) {
					pline("You feel real drugged out now.");
					switch(rn2(3)) {
					case 0: u.uconfused=d(4,5);
						break;
					case 1: nomul(-d(4,5));
						break;
					case 2: u.ufast+=d(3,3);
						break;
					}
				} else {
					pline("You get turned to stone!");
					u.uhp= -1;
				}
			}
			break;
		case 'D': if(rn2(6) || gcan(mtmp)) {
				hitu(10,d(3,10),mdat->mname);
				hitu(10,rnd(8),mdat->mname);
				hitu(10,rnd(8),mdat->mname);
				break;
			} else inrange(mtmp);
			hits= -1;
			break;
		case 'd': hitu(6,d(2,4),mdat->mname);
			break;
		case 'E':
			if((!u.ublind) && (!rn2(3)) && multi>=0) {
				k1("You are frozen by %s%ss gaze!",
 mdat->mname);
				nomul(-rn1(10,5));
			}
			return;
		case 'e': hitu(10,d(3,6),mdat->mname);
			break;
		case 'F':
			if(gcan(mtmp)) break;
			k1("%s%s explodes!",mdat->mname);
			if(u.ucoldres) pline(NOCOLD);
			else {
				if(17-(u.ulevel/2)>rnd(20)) {
					pline("You get blasted!");
					u.uhp-=d(6,6);
				} else {
					pline("You duck the blast...");
					u.uhp-=d(3,6);
				}
				flags.botl|=HP;
			}
			delmon(mtmp);
			hits= -1;
			if(getscr(mtmp->mloc)=='F')
				newsym(mtmp->mloc);
			break;
		case 'g': if(!gcan(mtmp) && hits && multi>=0 && !rn2(6)) {
				mhit(mdat->mname);
 k1("You are frozen by %s%ss juices.",mdat->mname);
				nomul(-rnd(10));
			}
			break;
		case 'h': if(!gcan(mtmp) && hits && multi>=0 && !rn2(5)) {
				mhit(mdat->mname);
				nomul(-rnd(10));
				k1("%s%ss bite puts you to sleep!",mdat->mname);
			}
			break;
		case 'j': tmp=hitu(4,rnd(3),mdat->mname);
			tmp&=hitu(4,rnd(3),mdat->mname);
			if(tmp) {
				hitu(4,rnd(4),mdat->mname);
				hitu(4,rnd(4),mdat->mname);
			}
			break;
		case 'k': if(hitu(4,rnd(4),mdat->mname) && !gcan(mtmp) &&
	 !rn2(8)) {
				mhit(mdat->mname);
				poisoned("the bee's sting");
			}
			break;
		case 'L': if(!gcan(mtmp) && hits && u.ugold && rn2(2)) {
				mhit(mdat->mname);
				tmp=rnd((int)u.ugold);
				if(tmp<100 && u.ugold<100) tmp=u.ugold;
				u.ugold-=tmp;
				u.urexp-=tmp;
				pline("Your purse feels lighter.");
				rloc(mtmp);
				sstat(mtmp,FLEE);
				flags.botl|=GOLD;
				for(stmp=fstole;stmp;stmp=stmp->nstole) {
					if(stmp->smon==mtmp) {
						stmp->sgold+=tmp;
						break;
					}
				}
				if(!stmp) {
					stmp=alloc(sizeof(struct stole));
					stmp->smon=mtmp;
					stmp->sgold=tmp;
					stmp->nstole=fstole;
					stmp->sobj=0;
					fstole=stmp;
				}
			}
			break;
		case 'N': if(!gcan(mtmp) && hits && invent && rn2(2)) {
				mhit(mdat->mname);
				steal(mtmp);
				rloc(mtmp);
				sstat(mtmp,FLEE);
			}
			break;
		case 'n': hitu(11,d(2,6),mdat->mname);
			hitu(11,d(2,6),mdat->mname);
			break;
		case 'o': tmp=hitu(5,rnd(6),mdat->mname);
			if(hitu(5,rnd(6),mdat->mname) && !gcan(mtmp) && tmp &&
 !u.ustuck && rn2(2)) {
				u.ustuck=mtmp;
				mhit(mdat->mname);
				k1("%s%s has grabbed you!",mdat->mname);
				u.uhp-=d(2,8);
			} else if(u.ustuck==mtmp) {
				u.uhp-=d(2,8);
				flags.botl|=HP;
				pline("You are being crushed.");
			}
			break;
		case 'P': if(!gcan(mtmp) && hits && !rn2(4)) {
				mhit(mdat->mname);
				justswld(mtmp);
			} else hitu(15,d(2,4),mdat->mname);
			break;
		case 'Q': hitu(3,rnd(2),mdat->mname);
			hitu(3,rnd(2),mdat->mname);
			break;
		case 'R': 
			hitu(5,0,mdat->mname);
			if((!gcan(mtmp)) && hits && uarm && uarm->otyp!=2 &&
(!gminus(uarm) || uarm->otyp-gospe(uarm)!=1)) {
				mhit(mdat->mname);
				pline(RUST);
				minusone(uarm);
				u.uac++;
				flags.botl|=AC;
			}
			break;
		case 'S': if(!gcan(mtmp) && hits && !rn2(8)) {
				mhit(mdat->mname);
				poisoned("snake's bite");
			}
			break;
		case 's': if(hits && !rn2(8)) {
				mhit(mdat->mname);
				poisoned("scorpion's sting");
			}
			hitu(5,rnd(8),mdat->mname);
			hitu(5,rnd(8),mdat->mname);
			break;
		case 'T':  hitu(6,rnd(6),mdat->mname);
			hitu(6,rnd(6),mdat->mname);
			break;
		case 'U': hitu(9,d(3,4),mdat->mname);
			hitu(9,d(3,4),mdat->mname);
			break;
		case 'v': if(!gcan(mtmp) && hits && !u.ustuck) u.ustuck=mtmp;
			break;
		case 'V':
			if(hits) {
				u.uhp-=4;
				mhit(mdat->mname);
				if(!gcan(mtmp) && !rn2(3)) losexp();
			}
			break;
		case 'W': if(!gcan(mtmp) && hits && !rn2(5)) {
				mhit(mdat->mname);
				losexp();
			}
			break;
		case 'X':  for(tmp=0;tmp<3;tmp++)
				hitu(8,rnd(3),mdat->mname);
			break;
		case 'y':
			if(gcan(mtmp)) break;
			delmon(mtmp);
			if(getcan(mtmp->mloc)) newsym(mtmp->mloc);
			if(!u.ublind) {
				pline("A yellow light blinds you!");
				u.ublind=d(4,12);
				seeoff(0);
			}
			hits= -1;
			break;
		case 'Y': hitu(4,rnd(6),mdat->mname);
			break;
		}
		switch(hits) {
		case 0: k1("%s%s misses",mdat->mname);
			break;
		case -1: break;
		default: mhit(mdat->mname);
		}
#ifndef SMALL
		if(u.uhp<1) killer=mdat->mname;
#endif
		for(tmp=mdat->mmove-12;tmp > rnd(12);tmp-=12) m_move(mtmp);
/* extra movement for fast monsters */
	}
}
mhit(name)
register char *name;
{
	if(hits==1) k1("%s%s hits!",name);
	else {
		if(u.ublind) pline("It hits %s!",hnu[hits-2]);
		else pline("The %s hits %s!",name,hnu[hits-2]);
	}
	hits= -1;
}
inrange(mtmp)
register struct monst *mtmp;
{
	register dir;

	if(dir=near(mtmp->mloc,u.uloc,8)) {
		if(bhit(dir,8)==mtmp) buzz(1,mtmp->mloc,dir);
	}
}
m_move(mtmp)
register struct monst *mtmp;
{
	register struct lev *nloc,*oloc;
	int tries;
	char dir;

	if(u.uswallow) return(1);
	if(mtmp->data->mlet=='t' && !rn2(19)) {
		if(rn2(2)) {
			oloc=mtmp->mloc;
			mnexto(mtmp);	/* mnexto doesn't change old position*/
			if(getscr(oloc)=='t') newsym(oloc);
		} else rloc(mtmp);	/* rloc does */
		return(1);
	}
	if(mtmp->data->mlet=='D' && !gcan(mtmp)) inrange(mtmp);
	if(!u.uconfused && mtmp->data->mlet=='U' && !gcan(mtmp) &&
 getcan(mtmp->mloc) && !rn2(8)) {
		pline("You are confused!");
		u.uconfused=d(3,4);
	}
	oloc=mtmp->mloc;
	tries=15;
	{
		int mx,my,ux,uy;

		getxy(&mx,&my,mtmp->mloc-levl);
		getxy(&ux,&uy,u.uloc-levl);

		if(mx!=ux) dir=(mx<ux)?22:-22;
		else dir=0;

		if(my!=uy) dir+=(my<uy)? 1:-1;

		nloc=oloc+dir;
		if(r_full(nloc)) {
			if(mx==ux) dir+=rn2(2)?-22:22;
			else if(my==uy) dir+=rn2(2)?-1:1;
			nloc=oloc+dir;
			if(r_full(nloc)) {
				do {
					nloc=oloc+(dir=dirs[rn2(8)]);
				} while ((r_full(nloc)) && tries--);
				if(!tries) return(0);
			}
		}
	}
	mtmp->mloc=nloc;
	if(getscr(oloc)==mtmp->data->mlet)
		newsym(oloc);
	if((u.ucinvis || !ginv(mtmp)) && getcan(nloc))
		atl(nloc,mtmp->data->mlet);
	return(1);
}
/* make monster mtmp next to you.  Next to you is in an ever
incresing square if it doesn't find room.  */
mnexto(mtmp)
struct monst *mtmp;
{
	register struct lev *loc,**tfoo,*loy,*hiy,*hixy,*loxhy;
	struct lev *foo[25];

	tfoo=foo;
	loy=u.uloc-23;
	hiy=u.uloc-21;
	loxhy=u.uloc+21;
	hixy=u.uloc+23;
	do {	/* full kludge action. */
		for(loc=loy;loc<=loxhy;loc+=22)
			if(!r_full(loc)) *(tfoo++)=loc;
		for(loc=hiy;loc<=hixy;loc+=22)
			if(!r_full(loc)) *(tfoo++)=loc;
		for(loc=loy+1;loc<hiy;loc++)
			if(!r_full(loc)) *(tfoo++)=loc;
		for(loc=loxhy+1;loc<hixy;loc++)
			if(!r_full(loc)) *(tfoo++)=loc;
		loy-=23;
		hiy-=21;
		loxhy+=21;
		hixy+=23;
	} while(tfoo==foo);
	tfoo= &foo[rn2(tfoo-foo)];
	mtmp->mloc= *tfoo;
	if(getcan(mtmp->mloc) && ((!ginv(mtmp)) || u.ucinvis))
		atl(mtmp->mloc,mtmp->data->mlet);
}
poisoned(string)
register char *string;
{
	k1("%s%s was poisoned!",string);
	if(u.upres) {
		pline("The poison has no affect.");
		return;
	}
	switch(rnd(6)) {
	case 1: u.uhp= -1;
		break;
	case 2:
	case 3:
	case 4:
		losestr(rn1(3,3));
		break;
	case 5:
	case 6:
		u.uhp-=rn1(10,6);
		flags.botl|=HP;
		break;
	}
}
steal(mtmp)
struct monst *mtmp;
{
	register struct obj *otmp,*ot1;
	register tmp;
	struct stole *stmp;

	for(otmp=invent,tmp=0;otmp;otmp=otmp->nobj,tmp++) ;
	tmp=rn2(tmp);
	if(!tmp) {
		otmp=invent;
		invent=invent->nobj;
	} else {
		--tmp;
		for(otmp=invent;otmp && tmp>1;tmp--,otmp=otmp->nobj) ;
		ot1=otmp->nobj;
		otmp->nobj=otmp->nobj->nobj;
		otmp=ot1;
	}
	if(otmp==uarm) {
		u.uac+=uarm->otyp;
		if(gminus(uarm)) u.uac-=gospe(uarm);
		else u.uac+=gospe(uarm);
		uarm=0;
		flags.botl|=AC;
	} else if(otmp==uwep) uwep=0;
	else if(otmp==uleft) {
		uleft=0;
		ringoff(otmp);
	} else if(otmp==uright) {
		uright=0;
		ringoff(otmp);
	}
	strcpy(buf,"She stole ");
	doname(otmp,&buf[10]);
	pline(buf);
	otmp->nobj=0;
	for(stmp=fstole;stmp;stmp=stmp->nstole) {
		if(stmp->smon==mtmp) {
			otmp->nobj=stmp->sobj;
			stmp->sobj=otmp;
			return;
		}
	}
	stmp=alloc(sizeof(struct stole));
	stmp->sgold=0;
	stmp->smon=mtmp;
	stmp->sobj=otmp;
	stmp->nstole=fstole;
	fstole=stmp;
}

/* make a chameleon look like a new monster */
newcham(mtmp,mdat)
register struct monst *mtmp;
register struct permonst *mdat;
{
	register float mp;

	if(getcan(mtmp->mloc) && ginv(mtmp)) newsym(mtmp->mloc);
	rinv(mtmp);
	while(!mdat->mlet && mdat>mon) mdat--;	/* ok? */
	if(mdat==mon) {
		newcham(mtmp,&mon[7,6]);
		return;
	}
	mp=mtmp->mhp/mtmp->data->mhd;
	mtmp->mhp=(int)((float)mdat->mhd*mp);
	mtmp->data=mdat;
	mp=mtmp->orig_hp/mtmp->data->mhd;
	mtmp->orig_hp=(int)((float)mdat->mhd*mp);
	if(mtmp->orig_hp<1) mtmp->orig_hp=mtmp->mhp=4;
	mtmp->mhp+=5;
	if(mtmp->mhp>mtmp->orig_hp) mtmp->mhp=mtmp->orig_hp;
	if(mdat->mlet=='I') {
		sinv(mtmp);
		if(getcan(mtmp->mloc)) prl(mtmp->mloc);
	}
	if(getcan(mtmp->mloc)) pmon(mtmp);
}
makemon(ptr)
register struct permonst *ptr;
{
	register struct monst *mtmp;
	register tmp;
	int foo;

	if(ptr) {
		if(!ptr->mlet) {
			if(!u.ublind) pline("The %s vanishes!",ptr->mname);
			return;
		}
	} else {
		do {
			foo=dlevel/3+1;
			tmp=rn2(foo);
			ptr= &mon[tmp>7?rn2(8):tmp][rn2(7)];
		} while(!ptr->mlet);
	}
	mtmp=alloc(sizeof(struct monst));
	mtmp->hib1=mtmp->lob1=0;
	mtmp->data=ptr;
	mtmp->nmon=fmon;
	fmon=mtmp;
	if(ptr->mlet=='I') sinv(mtmp);
	if(index("p~,",ptr->mlet)) {
		sinv(mtmp);
		ssinv(mtmp);
	}
	if(ptr->mlet=='D') mtmp->orig_hp=mtmp->mhp=80;
	else if(!ptr->mhd) mtmp->orig_hp=mtmp->mhp=rnd(4);
	else mtmp->orig_hp=mtmp->mhp=d(ptr->mhd,8);
	if(ptr->mlet == ':' && !u.ucham) {
		scham(mtmp);
		newcham(mtmp,&mon[rn1(6,2)][rn2(8)]);
	}
}
hitu(mlev,dam,name)
register mlev,dam;
register char *name;
{
	char tmp;

	tmp= -1+u.uac+mlev;
	if(multi<0) tmp+=4;
	if(u.uinvis) tmp-=2;
	if(u.uconfused) tmp++;
	if(u.ublind) tmp++;
	if(tmp<rnd(20)) return(0);
	if(dam) {
		u.uhp-=dam;
		flags.botl|=HP;
	}
	if(name) hits++;
	return(1);
}
swallowed()
{
	int x,y;

	cls();
	getxy(&x,&y,u.uloc-levl);
	curs(x-1,y+1);
	fputs("/-\\",stdout);
	curx+=3;
	curs(x-1,y+2);
	fputs("|@|",stdout);
	curx+=3;
	curs(x-1,y+3);
	fputs("\\-/",stdout);
	curx+=3;
}
