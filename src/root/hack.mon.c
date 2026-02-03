#include "../compat.h"
#include "hack.h"
extern char mregen[],WCLEV[],NOBLUE[],RUST[],CRUSH[],NOCOLD[],IT[],It[];
extern struct permonst vbat;
char hits;
char *hnu[]= {
	"twice",
	"three times",
	"four times",
	"five times"
};
char *kmsg[4]= {
	"You destroy %s%s!",
	"You blow away %s%s!",
	"You wale on %s%s!",
	"You have defeated %s%s!"
};

void movemon(void)
{
	register struct monst *mtmp;

	for(mtmp=fmon;mtmp;mtmp=mtmp->nmon) {
		if(mtmp->mspeed!=MSLOW || !(moves%2)) dochug(mtmp);
		if(mtmp->mspeed==MFAST) dochug(mtmp);
	}
}
void justswld(struct monst *mtmp)
{
	/* Original 1982: register char tx,ty; — declared but unused in justswld() */

	u.ustuck=mtmp;
	u.uswallow=flags.botl=1;
	u.uswldtim=0;
	if(mtmp->mstat==SLEEP) mtmp->mstat=MNORM;
	swallowed();
	if(mtmp->data->mlet=='P') k1("%s%s swallows you!",mtmp->data->mname);
	else k1("%s%s engulfs you!",mtmp->data->mname);
	hits= -1;
}
void youswld(struct monst *mtmp, int dam, int die)
{
	u.uhp-=dam;
	if(u.uswldtim++ == die) u.uhp= -1;
	flags.botl|=HP;
#ifndef SMALL
	if(u.uhp<1) killer=mtmp->data->mname;
#endif
	hits= -1;
}
void dochug(struct monst *mtmp)
{
	register struct permonst *mdat;
	register int tmp;
	char mdix,mdiy;
	struct stole *stmp;

	if(mtmp->cham && !rn2(6)) newcham(mtmp,&mon[rn1(6,2)][rn2(8)]);
	mdat=mtmp->data;
#ifndef SMALL
	if(mdat->mlet == 'V' && !rn2(10)){
		mtmp->data = &vbat;
		if(levl[mtmp->mx][mtmp->my].cansee) atl(mtmp->mx,mtmp->my,'B');
	}else if(mdat==&vbat && u.ustuck!=mtmp && !rn2(10)){
		mtmp->data = &mon[6][2];	/* bat into vampire */
		if(levl[mtmp->mx][mtmp->my].cansee) atl(mtmp->mx,mtmp->my,'V');
	}
#endif
	if((!moves%20 || index(mregen,mdat->mlet)) && mtmp->mhp<mtmp->orig_hp)
		mtmp->mhp++; /* regenerate monsters. */
	if(mtmp->mstat==MFROZ || mtmp->sinv)
		return; /* frozen monsters don't do anything. */
	if(mtmp->mstat==SLEEP) {/* wake up a monster, or get out of here. */
		if(levl[mtmp->mx][mtmp->my].cansee && !u.ustelth && (!rn2(7) ||
u.uagmon)) mtmp->mstat=0;
		else return;
	}
	if(mdat->mmove>=rnd(12) && (mtmp->mstat==FLEE || abs(mtmp->mx-u.ux)>1 ||
abs(mtmp->my-u.uy)>1) && m_move(mtmp) && mdat->mmove<=12)
		return; /* move monsters and exit if rate<=12" */
#ifndef SMALL
	if(u.ustuck==mtmp && mdat->mlet=='B'){
		k1("%s%s sucks more blood out of you!",mdat->mname);
		u.uhp-=d(2,10);
		if(u.uhp<1) killer=mdat->mname;
		return;
	}
#endif
	if(u.uswallow) {
		if(mtmp==u.ustuck)
			switch(mtmp->data->mlet) {
			case ',':
				k1(CRUSH,mtmp->data->mname);
				youswld(mtmp,4+u.uac,5);
				break;
			case '~':
				k1(CRUSH,mtmp->data->mname);
				youswld(mtmp,rnd(6),7);
				break;
			default:
				k1("%s%s digests you!",mtmp->data->mname);
				youswld(mtmp,d(2,4),12);
				break;
			}
		return;
	}
	mdix=abs(mtmp->mx-u.ux);
	mdiy=abs(mtmp->my-u.uy);
	if(mdat->mlet!='a' &&  mdix<2 && mdiy<2) {
#ifndef SMALL
		/* Original 1982: ioctl(0,TIOCFLUSH,0); */
		if(flags.flush) tcflush(0, TCIFLUSH);  /* Modern: POSIX termios */
#endif
		hits=0;
		nomul(0);
		if(!index("EyF&D",mdat->mlet))
			hitu(mdat->mhd,d(mdat->damn,mdat->damd),mdat->mname);
		switch(mdat->mlet) {
		case '&':
			if(!mtmp->mcan && !rn2(15)) {
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
		case ',': if(hits && !mtmp->mcan) {
				mhit(mdat->mname);
				justswld(mtmp);
			}
			break;
		case 'A': if(!mtmp->mcan && hits && rn2(2)) {
				mhit(mdat->mname);
				pline("You feel weaker!");
				losestr(1);
			}
			break;
#ifndef SMALL
		case 'B': if(mdat==&vbat && hits && !mtmp->mcan) {
				if(!u.ustuck && rn2(2)){
				k1("%s%s bites you in the neck!",mdat->mname);
					u.uhp-=d(2,4);
					u.ustuck = mtmp;
				}
				if(u.uhp<1) killer=mdat->mname;
				return;
			}
			break;
#endif
		case 'C': hitu(4,rnd(6),mdat->mname);
			break;
		case 'c': if(!mtmp->mcan && hits && rn2(2)) {
				mhit(mdat->mname);
				if(rn2(5)) {
					pline("You feel real drugged out now.");
					switch(rn2(3)) {
					case 0: u.uconfused=d(4,5);
						break;
					case 1: nomul(-d(3,4));
						break;
					case 2: u.ufast+=d(3,3);
						break;
					}
				} else {
					pline("You get turned to stone!");
					u.uhp= -1;
#ifndef SMALL
					killer=mdat->mname;
#endif
				}
			}
			break;
		case 'D': if(rn2(6) || mtmp->mcan) {
				hitu(10,d(3,10),mdat->mname);
				hitu(10,rnd(8),mdat->mname);
				hitu(10,rnd(8),mdat->mname);
				break;
			}
			buzz(1,mtmp->mx,mtmp->my,u.ux-mtmp->mx,u.uy-mtmp->my);
#ifndef SMALL
			if(u.uhp<1) killer=mdat->mname;
#endif
			hits= -1;
			break;
		case 'd': hitu(6,d(2,4),mdat->mname);
			break;
		case 'E':
			if((!u.ublind) && (!rn2(3)) && multi>=0) {
				k1("You are frozen by %s%ss gaze!",
 mdat->mname);
				nomul(-rn1(8,5));
			}
			return;
		case 'e': hitu(10,d(3,6),mdat->mname);
			break;
		case 'F':
			if(mtmp->mcan) break;
			k1("%s%s explodes!",mdat->mname);
			if(u.ucoldres) pline(NOCOLD);
			else {
				if(17-(u.ulevel/2)>rnd(20)) {
					pline("You get blasted!");
					losehp(d(6,6));
				} else {
					pline("You duck the blast...");
					losehp(d(3,6));
				}
#ifndef SMALL
				if(u.uhp<1) killer=mdat->mname;
#endif
			}
			delmon(mtmp);
			hits= -1;
			if(levl[mtmp->mx][mtmp->my].scrsym=='F')
				newsym(mtmp->mx,mtmp->my);
			break;
		case 'g': if(!mtmp->mcan && hits && multi>=0 && !rn2(6)) {
				mhit(mdat->mname);
k1("You are frozen by %s%ss juices.",mdat->mname);
				nomul(-rnd(10));
			}
			break;
		case 'h': if(!mtmp->mcan && hits && multi>=0 && !rn2(5)) {
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
		case 'k': if(hitu(4,rnd(4),mdat->mname) && !mtmp->mcan &&
	 !rn2(8)) {
				mhit(mdat->mname);
				poisoned("the bee's sting");
#ifndef SMALL
				if(u.uhp<1) killer=mdat->mname;
#endif
			}
			break;
		case 'L': if(!mtmp->mcan && hits && u.ugold && rn2(2)) {
				mhit(mdat->mname);
				tmp=rnd((int)u.ugold);
				if(tmp<100 && u.ugold<100) tmp=u.ugold;
				u.ugold-=tmp;
				u.urexp-=tmp;
				pline("Your purse feels lighter.");
				rloc(mtmp);
				mtmp->mstat=FLEE;
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
		case 'N': if(!mtmp->mcan && hits && invent && rn2(2)) {
				mhit(mdat->mname);
				steal(mtmp);
				rloc(mtmp);
				mtmp->mstat=FLEE;
			}
			break;
		case 'n': hitu(11,d(2,6),mdat->mname);
			hitu(11,d(2,6),mdat->mname);
			break;
		case 'o': tmp=hitu(5,rnd(6),mdat->mname);
			if(hitu(5,rnd(6),mdat->mname) && !mtmp->mcan && tmp &&
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
#ifndef SMALL
			if(u.uhp<1) killer=mdat->mname;
#endif
			break;
		case 'P': if(!mtmp->mcan && hits && !rn2(4)) {
				mhit(mdat->mname);
				justswld(mtmp);
			} else hitu(15,d(2,4),mdat->mname);
			break;
		case 'Q': hitu(3,rnd(2),mdat->mname);
			hitu(3,rnd(2),mdat->mname);
			break;
		case 'R':
			hitu(5,0,mdat->mname);
			if(!mtmp->mcan && hits && uarm && uarm->otyp!=2 &&
(!uarm->minus || uarm->otyp-uarm->spe!=1)) {
				mhit(mdat->mname);
				pline(RUST);
				minusone(uarm);
				u.uac++;
				flags.botl|=AC;
			}
			break;
		case 'S': if(!mtmp->mcan && hits && !rn2(8)) {
				mhit(mdat->mname);
				poisoned("snake's bite");
#ifndef SMALL
				if(u.uhp<1) killer=mdat->mname;
#endif
			}
			break;
		case 's': if(hits && !rn2(8)) {
				poisoned("scorpion's sting");
#ifndef SMALL
				if(u.uhp<1) killer=mdat->mname;
#endif
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
		case 'v': if(!mtmp->mcan && hits && !u.ustuck) u.ustuck=mtmp;
			break;
		case 'V':
			if(hits) {
				u.uhp-=4;
				mhit(mdat->mname);
				if(!mtmp->mcan && !rn2(3)) losexp();
#ifndef SMALL
				if(u.uhp<1) killer=mdat->mname;
#endif
			}
			break;
		case 'W': if(!mtmp->mcan && hits && !rn2(5)) {
				mhit(mdat->mname);
				losexp();
			}
			break;
		case 'X':  for(tmp=0;tmp<3;tmp++)
				hitu(8,rnd(3),mdat->mname);
			break;
		case 'y':
			if(mtmp->mcan) break;
			delmon(mtmp);
			if(levl[mtmp->mx][mtmp->my].cansee)
				newsym(mtmp->mx,mtmp->my);
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
		for(tmp=mdat->mmove-12;tmp > rnd(12);tmp-=12) m_move(mtmp);
/* extra movement for fast monsters */
	}
}
void mhit(char *name)
{
	if(hits<1) pline("bad mhit");
	else if(hits==1) k1("%s%s hits!",name);
	else {
		if(u.ublind) pline("It hits %s!",hnu[hits-2]);
		else pline("The %s hits %s!",name,hnu[hits-2]);
	}
	hits= -1;
}
void inrange(struct monst *mtmp)
{
	register char tx,ty;
	int zx,zy;

	tx=u.ux - mtmp->mx;
	ty=u.uy - mtmp->my;
/* this mess figures out if the person is within 8 */
	if((!tx && abs(ty)<8) || (!ty && abs(tx) <8) || (abs(tx)==abs(ty) &&
 abs(tx)<8)){
		zx = tx/abs(tx);	/* save dx and dy. */
		zy = ty/abs(ty);
		tx=dx;
		ty=dy;
		if(bhit(zx,zy,8)==mtmp) buzz(1,mtmp->mx,mtmp->my,dx,dy);
		dx=tx;	/* if we don't save dx and dy a capital move may
 screw up*/
		dy=ty;
	}
}
int m_move(struct monst *mtmp)
{
	register int nix,niy,omx,omy;
	char dx,dy;

	if(u.uswallow) return(1);
	if(mtmp->data->mlet=='t' && !rn2(19)) {
		if(rn2(2)) {
			dx=mtmp->mx;
			dy=mtmp->my;
			mnexto(mtmp);	/* mnexto doesn't change old position*/
			/* Modern: cast to unsigned char — dx/dy hold position here, not deltas */
			if(levl[(unsigned char)dx][(unsigned char)dy].scrsym=='t') newsym(dx,dy);
		} else rloc(mtmp);	/* rloc does */
		return(1);
	}
	dx=sgn(u.ux-mtmp->mx);
	dy=sgn(u.uy-mtmp->my);
	if(mtmp->mstat==MCONF || u.uinvis || (index("BI",mtmp->data->mlet) &&
!rn2(3))) {
		dx=rn1(3,-1);
		dy=rn1(3,-1);
	}
	if(mtmp->data->mlet=='D' && !mtmp->mcan) inrange(mtmp);
	if(!u.uconfused && mtmp->data->mlet=='U' && !mtmp->mcan &&
 levl[mtmp->mx][mtmp->my].cansee && !rn2(8)) {
		pline("You are confused!");
		u.uconfused=d(3,4);
	}
	if(!r_free(mtmp->mx+dx,mtmp->my+dy)) {
		if(!dx) dx=rn1(3,-1);
		else if(!dy) dy=rn1(3,-1);
	}
	nix=mtmp->mx+dx;
	niy=mtmp->my+dy;
	omx=mtmp->mx;
	omy=mtmp->my;
	if(r_free(nix,niy) && !(dx && dy && (levl[omx][omy].typ==DOOR ||
levl[nix][niy].typ==DOOR))) {
		mtmp->mx=nix;
		mtmp->my=niy;
	} else if(dx && r_free(nix,mtmp->my)) mtmp->mx=nix;
	else if(dy && r_free(mtmp->mx,niy)) mtmp->my=niy;
	else {
		if(!rn2(10) && index("tNL",mtmp->data->mlet)) rloc(mtmp);
		return(0);
	}
	if(levl[omx][omy].scrsym==mtmp->data->mlet)
		newsym(omx,omy);
	if((u.ucinvis || !mtmp->invis) && levl[mtmp->mx][mtmp->my].cansee)
		atl(mtmp->mx,mtmp->my,mtmp->data->mlet);
	return(1);
}
int r_free(int x, int y)
{
	if(levl[x][y].typ<DOOR || g_at(x,y,fmon) || (x==u.ux && y==u.uy)) return(0);
	else return(1);
}
void mnexto(struct monst *mtmp)
/* make monster mtmp next to you.  Next to you is in an ever
incresing square if it doesn't find room.  */
{
	register int x,y;
	struct {
		char zx,zy;
	} foo[25],*tfoo;
	int range;

	tfoo=foo;
	range=1;
	do {	/* full kludge action. */
		for(x=u.ux-range;x<=u.ux+range;x++)
			if(test(x,u.uy-range)) {
				tfoo->zx=x;
				tfoo++->zy=u.uy-range;
			}
		for(x=u.ux-range;x<=u.ux+range;x++)
			if(test(x,u.uy+range)) {
				tfoo->zx=x;
				tfoo++->zy=u.uy+range;
			}
		for(y=u.uy+1-range;y<u.uy+range;y++)
			if(test(u.ux-range,y)) {
				tfoo->zx=u.ux-range;
				tfoo++->zy=y;
			}
		for(y=u.uy+1-range;y<u.uy+range;y++)
			if(test(u.ux+range,y)) {
				tfoo->zx=u.ux+range;
				tfoo++->zy=y;
			}
		range++;
	} while(tfoo==foo);
	tfoo= &foo[rn2(tfoo-foo)];
	mtmp->mx=tfoo->zx;
	mtmp->my=tfoo->zy;
	if(levl[mtmp->mx][mtmp->my].cansee && ((!mtmp->invis) || u.ucinvis))
		atl(mtmp->mx,mtmp->my,mtmp->data->mlet);
}
int test(int x, int y)
{
	if(x<1 || x>78 || y<1 || y>20) return(0);
	if(g_at(x,y,fmon) || levl[x][y].typ<DOOR) return(0);
	return(1);
}
void poisoned(char *string)
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
		losehp(rn1(10,6));
		break;
	}
}
void steal(struct monst *mtmp)
{
	register struct obj *otmp,*ot1;
	register int tmp;
	struct stole *stmp;

	for(otmp=invent,tmp=0;otmp;otmp=otmp->nobj,tmp++) ;
	tmp=rn2(tmp);
	if(!tmp) {
		otmp=invent;
		invent=invent->nobj;
	} else {
		--tmp;
		for(otmp=invent;otmp && tmp>1;tmp--,otmp=otmp->nobj) ;
		if(!otmp->nobj || !otmp) {
			pline("Steal fails!");
			return;
		}
		ot1=otmp->nobj;
		otmp->nobj=otmp->nobj->nobj;
		otmp=ot1;
	}
	if(otmp==uarm) {
		u.uac+=uarm->otyp;
		if(uarm->minus) u.uac-=uarm->spe;
		else u.uac+=uarm->spe;
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
void killed(struct monst *mtmp)
{
	register struct gen *gtmp;
	register int tmp;
	struct stole *stmp;
	struct obj *otmp;

	if(mtmp->cham) mtmp->data= &mon[6][6];
#ifdef SMALL
	k1("You destroy %s%s!",mtmp->data->mname);
#else
	k1(kmsg[rand()%4],mtmp->data->mname);
#endif
	if(u.umconf) {
		pline(NOBLUE);
		u.umconf=0;
	}
	if(u.ustuck==mtmp) u.ustuck=0;
	tmp=1+(mtmp->data->mhd * mtmp->data->mhd);
	if(mtmp->data->ac<3) tmp+=(2*(7- mtmp->data->ac));
	if(index("AcsSDXaeRTVWU&In:P",mtmp->data->mlet))
		tmp+=(4*mtmp->data->mhd);
	if(index("DeV&P",mtmp->data->mlet)) tmp+=(10*mtmp->data->mhd);
	if(mtmp->data->mhd > 6) tmp+=50;
	u.uexp+=tmp;
	u.urexp+=4*tmp;
	flags.botl|=UEX;
	for(stmp=fstole;stmp;stmp=stmp->nstole) {
		if(stmp->smon==mtmp) {
			if(stmp->sgold) {
				if((gtmp=g_at(mtmp->mx,mtmp->my,fgold)))
					gtmp->gflag+=stmp->sgold+d(dlevel,30);
				else {
					gtmp=alloc(sizeof(struct gen));
					gtmp->ngen=fgold;
					gtmp->gflag=stmp->sgold+d(dlevel,30);
					fgold=gtmp;
				}
				if(levl[gtmp->gx=mtmp->mx][gtmp->gy=mtmp->my].
 cansee) atl(mtmp->mx,mtmp->my,'$');
			}
			if(stmp->sobj) {
				for(otmp=stmp->sobj;otmp->nobj;otmp=otmp->nobj){
					otmp->ox=mtmp->mx;
					otmp->oy=mtmp->my;
				}
				otmp->nobj=fobj;
				fobj=stmp->sobj;
				if(levl[otmp->ox=mtmp->mx][otmp->oy=mtmp->my].
 cansee) atl(otmp->ox,otmp->oy,otmp->olet);
			}
			if(stmp==fstole) fstole=fstole->nstole;
			else {
				struct stole *st1;

				for(st1=fstole;st1->nstole!=stmp;
 st1=st1->nstole) ;
				st1->nstole=stmp->nstole;
			}
			mfree(stmp);
			break;
		}
	}
	if(!stmp && mtmp->data->mlet=='L') {
		if((gtmp=g_at(mtmp->mx,mtmp->my,fgold)))
			gtmp->gflag+=d(dlevel,35);
		else {
			gtmp=alloc(sizeof(struct gen));
			gtmp->ngen=fgold;
			fgold=gtmp;
			gtmp->gflag=d(dlevel,35);
		}
		if(levl[gtmp->gx=mtmp->mx][gtmp->gy=mtmp->my].cansee)
			atl(gtmp->gx,gtmp->gy,'$');
	}
	if(levl[mtmp->mx][mtmp->my].typ>=SDOOR && !u.uswallow && (index("gNTV&",
 mtmp->data->mlet) || !rn2(5))) {
		mkobj(0);
		if(levl[fobj->ox=mtmp->mx][fobj->oy=mtmp->my].cansee)
			atl(mtmp->mx,mtmp->my,fobj->olet);
	}
	/* Modern: cache fields before delmon() to avoid use-after-free */
	{
		unsigned char mx=mtmp->mx, my=mtmp->my;
		char mlet=mtmp->data->mlet;

		delmon(mtmp);
		if(levl[mx][my].scrsym==mlet)
			newsym(mx,my);
	}
	if(u.uswallow) {
		u.uswldtim=u.uswallow=0;
		docrt();
	}
	if(u.uexp<10*hack_pow(u.ulevel-1)) return;  /* Original 1982: pow() — renamed for math.h conflict */
	if(u.ulevel>13) return;
	pline(WCLEV,++u.ulevel);
	tmp=rnd(10);
	if(tmp<3) tmp=rnd(10);
	u.uhpmax+=tmp;
	u.uhp+=tmp;
	flags.botl|=(HP|HPM|ULV);
}
void kludge(char *str, char *arg)
{
	if(u.ublind) pline(str,*str=='%'?IT:It);
	else pline(str,arg);
}
void k1(char *str, char *arg)
{
	if(u.ublind) pline(str,nul,*str=='%'?IT:It);
	else pline(str,*str=='%'?"The ":"the ",arg);
}
void rescham(void)
{	/* force all chameleons to become normal */
	register struct monst *mtmp;

	for(mtmp=fmon;mtmp;mtmp=mtmp->nmon)
		if(mtmp->cham) {
			mtmp->cham=0;
			newcham(mtmp,&mon[6][6]);
		}
}
void newcham(struct monst *mtmp, struct permonst *mdat)
/* make a chameleon look like a new monster */
{
	register float mp;

	if(mtmp->invis && levl[mtmp->mx][mtmp->my].cansee)
		newsym(mtmp->mx,mtmp->my);
	mtmp->invis=0;
	while(!mdat->mlet) mdat--;	/* ok? */
	mp=mtmp->mhp/mtmp->data->mhd;
	pline("hp=%d--hd=%d--mp=%g",mtmp->mhp,mtmp->data->mhd,mp);
	mtmp->mhp=(int)((float)mdat->mhd)*mp;
	pline("new=%d",mtmp->mhp);
	mtmp->data=mdat;
	mp=mtmp->orig_hp/mtmp->data->mhd;
	pline("orig=%d--mp=%g",mtmp->orig_hp,mp);
	mtmp->orig_hp=(int)((float)mdat->mhd)*mp;
	pline("new=%d",mtmp->orig_hp);
	if(mtmp->orig_hp<1) mtmp->orig_hp=mtmp->mhp=4;
	mtmp->mhp+=5;
	if(mtmp->mhp>mtmp->orig_hp) mtmp->mhp=mtmp->orig_hp;
	if(mdat->mlet=='I') {
		mtmp->invis=1;
		if(levl[mtmp->mx][mtmp->my].cansee) prl(mtmp->mx,mtmp->my);
	}
	if(levl[mtmp->mx][mtmp->my].cansee)
		pmon(mtmp);
}
int makemon(struct permonst *ptr)
{
	register struct monst *mtmp;
	register int tmp;
	int foo;

	mtmp=alloc(sizeof(struct monst));
	mtmp->sinv=mtmp->mcan=mtmp->invis=mtmp->cham=mtmp->mstat=mtmp->mspeed=0;
	if(ptr) {
		if(!ptr->mlet) {
			if(!u.ublind) pline("The %s vanishes!",ptr->mname);
			mfree(mtmp);/* what a waste */
			return(1);
		}
	} else {
		do {
			foo=dlevel/3+1;
			tmp=rn2(foo);
			ptr= &mon[tmp>7?rn2(8):tmp][rn2(7)];
		} while(!ptr->mlet);
	}
	mtmp->data=ptr;
	mtmp->nmon=fmon;
	fmon=mtmp;
	if(ptr->mlet=='I') mtmp->invis=1;
	if(index("p~,",ptr->mlet)) mtmp->invis=mtmp->sinv=1;
	if(ptr->mlet=='D') mtmp->orig_hp=mtmp->mhp=80;
	else if(!ptr->mhd) mtmp->orig_hp=mtmp->mhp=rnd(4);
	else mtmp->orig_hp=mtmp->mhp=d(ptr->mhd,8);
	if(ptr->mlet == ':' && !u.ucham) {
		mtmp->cham=1;
		newcham(mtmp,&mon[rn1(6,2)][rn2(8)]);
	}
	return(0);
}
int hitu(int mlev, int dam, char *name)
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
#ifndef SMALL
		if(u.uhp<1) killer=name;
#endif
		flags.botl|=HP;
	}
	if(name) hits++;
	return(1);
}
void rloc(struct monst *mtmp)
{
	register int tmp;
	register int tx,ty;

	if(mtmp==u.ustuck) u.ustuck=u.uswallow=u.uswldtim=0;
	if(levl[mtmp->mx][mtmp->my].scrsym==mtmp->data->mlet)
		newsym(mtmp->mx,mtmp->my);
	do tmp=levl[tx=rn1(77,2)][ty=rn1(19,2)].typ;
	while(tmp<DOOR || g_at(tx,ty,fmon) || (tx==u.ux && ty==u.uy));
	mtmp->mx=tx;
	mtmp->my=ty;
	if(levl[tx][ty].cansee && ((!mtmp->invis) || u.ucinvis))
		atl(tx,ty,mtmp->data->mlet);
}
