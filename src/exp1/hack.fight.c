#include "hack.h"
		/* you fighting monsters (subset of move) */

abon()/* your bonus to hit (From strength) */
{
	if(u.ustr==3) return(-4);
	else if(u.ustr<6) return(-3);
	else if(u.ustr<8) return(-2);
	else if(u.ustr<17) return(-1);
	else if(u.ustr<69) return(0);	/* up to 18/50 */
	else if(u.ustr<118) return(1);
	else return(2);
}
amon(mtmp,obj,tmp)	/* Will you hit mtmp with obj (0 for bare hands) */
register struct monst *mtmp;	/* calls attmon if it hit */
register struct obj *obj;
register tmp;
{
	tmp+= u.ulevel+mtmp->data->ac+abon();
	if(obj) {
		if(obj->olet=='/' && obj->otyp==3) tmp+=3;
		else if(obj->olet==')') {
			if(gminus(obj)) tmp-=gospe(obj);
			else tmp+=gospe(obj);
			if(obj->otyp==8) tmp--; /* two handed sword */
			else if(obj->otyp==9) tmp+=2; /* dagger */
		}
	}
	if(gstat(mtmp)==SLEEP) {
		sstat(mtmp,0);
		tmp+=2;
	}
	if(gstat(mtmp)==MFROZ) {
		tmp+=4;
		if(!rn2(16)) sstat(mtmp,0);/* they might wake up */
	}
	if(gstat(mtmp)==FLEE) tmp+=2;
	/*if(tmp<1 && tmp>-4) tmp=1;*/	/* repeat 20 6 times ...*/
	if(tmp>=rnd(20)) {
		attmon(mtmp,obj);
		return;
	}
	if(obj!=uwep && obj->olet==')') miss(wepnam[obj->otyp],mtmp);
	else k1("You miss %s%s",mtmp->data->mname);
}
attmon(mtmp,obj)	/* you have hit monster mtmp with wep */
register struct monst *mtmp;
register struct obj *obj;
{
	register tmp;

	if(obj) {
		if(obj->olet=='/' && obj->otyp==3 && gospe(obj)>1) {
			if(!rn2(6)) {
				tmp=gospe(obj);
				tmp--;
				sospe(obj,tmp);
			}
			tmp=rn1(6,4);
		} else if(obj->olet==')') {
			if(index(mlarge,mtmp->data->mlet)) {
				tmp=rnd(wldam[obj->otyp]);
				if(obj->otyp==8) tmp+=d(2,6);
				else if(obj->otyp==6) tmp+=rnd(4);
			} else {
				tmp=rnd(wsdam[obj->otyp]);
				if(obj->otyp==6 || obj->otyp==4) tmp++;
			}
			if(gminus(obj)) tmp-=gospe(obj);
			else tmp+=gospe(obj);
		} else tmp=rnd(3);
	} else tmp=rnd(3);
	tmp+=u.udaminc;
	if(u.uswallow && mtmp->data->mlet=='P' && (tmp-=u.uswldtim)<1) {
		k1(HIT,mtmp->data->mname);
		return;
	}
	if(tmp<1) tmp=1;
	mtmp->mhp-=tmp;
	if(mtmp->mhp<1) {
		killed(mtmp);
		return;
	}
	if(obj==uwep || obj->olet!=')') k1(HIT,mtmp->data->mname);
	else hit(wepnam[obj->otyp],mtmp);
	if(!rn2(25) && mtmp->mhp<mtmp->orig_hp/2) {
		sstat(mtmp,FLEE);
		if(u.ustuck==mtmp) u.ustuck=0;
	}
	if(u.umconf) {
		pline(NOBLUE);
		if(getcan(mtmp->mloc))
 			pline("The %s appears confused.",mtmp->data->mname);
		sstat(mtmp,MCONF);
		u.umconf=0;
	}
	if(mtmp->data->mlet=='a' && obj==uwep) {
		if(rn2(2)) {
			kludge("You are splashed by %ss acid!","the blob'");
			u.uhp-=rnd(6);
			flags.botl|=HP;
#ifndef SMALL
			if(u.uhp<1) killer=mtmp->data->mname;
#endif
		}
		if(!rn2(6) && uwep && uwep->olet==')') {
			pline("Your %s corrodes!",wepnam[uwep->otyp]);
			minusone(uwep);
		}
	}
}
killed(mtmp)
register struct monst *mtmp;
{
	register struct gen *gtmp;
	register tmp;
	struct stole *stmp;
	struct obj *otmp;

	if(gcham(mtmp)) mtmp->data= &mon[6][6];
	k1("You destroy %s%s!",mtmp->data->mname);
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
				if(gtmp=g_at(mtmp->mloc,fgold))
					gtmp->gflag+=stmp->sgold+d(dlevel,30);
				else {
					gtmp=alloc(sizeof(struct gen));
					gtmp->ngen=fgold;
					gtmp->gflag=stmp->sgold+d(dlevel,30);
					fgold=gtmp;
				}
				if(getcan(gtmp->gloc=mtmp->mloc))
					atl(mtmp->mloc,'$');
			}
			if(stmp->sobj) {
				for(otmp=stmp->sobj;otmp->nobj;otmp=otmp->nobj)
					otmp->oloc=mtmp->mloc;
				otmp->nobj=fobj;
				fobj=stmp->sobj;
				if(getcan(otmp->oloc=mtmp->mloc))
					atl(otmp->oloc,otmp->olet);
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
		if(gtmp=g_at(mtmp->mloc,fgold))
			gtmp->gflag+=d(dlevel,35);
		else {
			gtmp=alloc(sizeof(struct gen));
			gtmp->ngen=fgold;
			fgold=gtmp;
			gtmp->gflag=d(dlevel,35);
		}
		if(getcan(gtmp->gloc=mtmp->mloc)) atl(gtmp->gloc,'$');
	}
	if(gettyp(mtmp->mloc)>=SDOOR && !u.uswallow &&
 (index("gNTV&",mtmp->data->mlet) || !rn2(5))) {
		mkobj();
		if(getcan(fobj->oloc=mtmp->mloc)) atl(mtmp->mloc,fobj->olet);
	}
	delmon(mtmp);
	if(getscr(mtmp->mloc)==mtmp->data->mlet)
		newsym(mtmp->mloc);
	if(u.uswallow) {
		u.uswldtim=u.uswallow=0;
		docrt();
	}
	if(u.uexp<10*pow(u.ulevel-1)) return;
	if(u.ulevel>13) return;
	pline(WCLEV,++u.ulevel);
	tmp=rnd(10);
	if(tmp<3) tmp=rnd(10);
	u.uhpmax+=tmp;
	u.uhp+=tmp;
	flags.botl|=(HP|HPM|ULV);
}
hit(str,mon)
register char *str;
register struct monst *mon;
{
	if(!getcan(mon->mloc)) pline("The %s hits it.",str);
	else pline("The %s hits the %s.",str,mon->data->mname);
}
miss(str,mon)
register char *str;
register struct monst *mon;
{
	if(!getcan(mon->mloc)) pline("The %s misses it.",str);
	else pline("The %s misses the %s.",str,mon->data->mname);
}
