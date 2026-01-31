#include "hack.h"
#include <signal.h>
#include <stdio.h>

/* Note that lint hates this (Moral: don't lint it) */
#define bwrite(fp,loc,num) fwrite(loc,1,num,fp)
#define mread(fp,loc,num) fread(loc,1,num,fp)

extern char nul[],mregen[],READ[];

savelev(fp)
register FILE *fp;
{
	register struct monst *mtmp;
	register struct gen *gtmp;
	register struct obj *otmp;
	struct stole *stmp;

	if(fp==0) panic("Bad save\n");
	bwrite(fp,levl,3520);
	bwrite(fp,&moves,2);
	bwrite(fp,&xupstair,1);
	bwrite(fp,&yupstair,1);
	bwrite(fp,&xdnstair,1);
	bwrite(fp,&ydnstair,1);
	for(stmp=fstole;stmp;stmp=stmp->nstole) {
		bwrite(fp,stmp,sizeof(struct stole));
		bwrite(fp,stmp->smon,sizeof(struct monst));
		delmon(stmp->smon);
		for(otmp=stmp->sobj;otmp;otmp=otmp->nobj) {
			bwrite(fp,otmp,sizeof(struct obj));
			mfree(otmp);
		}
		bwrite(fp,nul,sizeof(struct obj));
		mfree(stmp);
	}
	bwrite(fp,nul,sizeof(struct stole));
	for(mtmp=fmon;mtmp;mtmp=mtmp->nmon) {
		bwrite(fp,mtmp,sizeof(struct monst));
		mfree(mtmp);
	}
	bwrite(fp,nul,sizeof(struct monst));
	for(gtmp=fgold;gtmp;gtmp=gtmp->ngen) {
		bwrite(fp,gtmp,sizeof(struct gen));
		mfree(gtmp);
	}
	bwrite(fp,nul,sizeof(struct gen));
	for(gtmp=ftrap;gtmp;gtmp=gtmp->ngen) {
		bwrite(fp,gtmp,sizeof(struct gen));
		mfree(gtmp);
	}
	bwrite(fp,nul,sizeof(struct gen));
	for(otmp=fobj;otmp;otmp=otmp->nobj) {
		bwrite(fp,otmp,sizeof(struct obj));
		mfree(otmp);
	}
	bwrite(fp,nul,sizeof(struct obj));
	fstole=(struct stole *)fobj=(struct obj *)fgold=(struct gen*)fmon=
 (struct monst *)ftrap=0;
}
getlev(fp)
register FILE *fp;
{
	struct monst mbuf;
	struct gen gbuf;
	struct obj obuf;
	struct stole sbuf;
	register unsigned tmoves;
	unsigned omoves;

	if(fp==0 || fread(levl,1,3520,fp)!=3520) return(1);
	mread(fp,&omoves,2);
	mread(fp,&xupstair,1);
	mread(fp,&yupstair,1);
	mread(fp,&xdnstair,1);
	mread(fp,&ydnstair,1);
	mread(fp,&sbuf,sizeof(struct stole));
	while(sbuf.smon) {
		mread(fp,&mbuf,sizeof(struct monst));
		if(!mbuf.data->mlet) {
			if(sbuf.sgold) {
				gbuf.ngen=fgold;
				gbuf.gflag=sbuf.sgold+d(dlevel,30);
				levl[gbuf.gx=mbuf.mx][gbuf.gy=mbuf.my].
 scrsym='$';
				fgold=alloc(sizeof(struct gen));
				*fgold=gbuf;
			}
			mread(fp,&obuf,sizeof(struct obj));
			while(obuf.olet) {
				obuf.nobj=fobj;
				obuf.ox=mbuf.mx;
				obuf.oy=mbuf.my;
				fobj=alloc(sizeof(struct obj));
				*fobj=obuf;
				mread(fp,&obuf,sizeof(struct obj));
			}
		} else {
			sbuf.sobj=0;
			mread(fp,&obuf,sizeof(struct obj));
			while(obuf.olet) {
				obuf.nobj=sbuf.sobj;
				sbuf.sobj=alloc(sizeof(struct obj));
				*(sbuf.sobj)=obuf;
				mread(fp,&obuf,sizeof(struct obj));
			}
			mbuf.nmon=fmon;
			fmon=alloc(sizeof(struct monst));
			*fmon=mbuf;
			sbuf.smon=fmon;
			sbuf.nstole=fstole;
			fstole=alloc(sizeof(struct stole));
			*fstole=sbuf;
		}
		mread(fp,&sbuf,sizeof(struct stole));
	}
	if(omoves) tmoves=moves-omoves;
	mread(fp,&mbuf,sizeof(struct monst));
	while(mbuf.mx){
		if(omoves) {
			if(mbuf.data->mlet) {
				if(index(mregen,mbuf.data->mlet))
					mbuf.mhp+=mbuf.mhp+tmoves;
				else mbuf.mhp+=tmoves/20;
				if(mbuf.mhp>mbuf.orig_hp || mbuf.mhp<1)
					mbuf.mhp=mbuf.orig_hp;
				mbuf.nmon=fmon;
				fmon=alloc(sizeof(struct monst));
				*fmon=mbuf;
			}
		} else {
			mbuf.data= &mon[mbuf.mhp][mbuf.orig_hp];
			if(mbuf.data->mlet) {
				mbuf.sinv=mbuf.mspeed=mbuf.cham=mbuf.invis=
 mbuf.mcan=0;
				if(mbuf.data->mlet=='D')
					mbuf.mhp=mbuf.orig_hp=80;
				else if(mbuf.data->mhd)
					mbuf.orig_hp=mbuf.mhp=d(mbuf.data->mhd,
 8);
				else mbuf.orig_hp=mbuf.mhp=rnd(4);
				if(mbuf.data->mlet==':' && !u.ucham)
					mbuf.cham=1;
				if(mbuf.data->mlet=='I') mbuf.invis=1;
				if(index("p~,M",mbuf.data->mlet))
					mbuf.invis=mbuf.sinv=1;
				mbuf.nmon=fmon;
				fmon=alloc(sizeof(struct monst));
				/*V7*/
				*fmon=mbuf;
			}
		}
		mread(fp,&mbuf,sizeof(struct monst));
	}
	mread(fp,&gbuf,sizeof(struct gen));
	while(gbuf.gx) {
		gbuf.ngen=fgold;
		fgold=alloc(sizeof(struct gen));
		*fgold=gbuf;
		mread(fp,&gbuf,sizeof(struct gen));
	}
	mread(fp,&gbuf,sizeof(struct gen));
	while(gbuf.gx) {
		gbuf.ngen=ftrap;
		ftrap=alloc(sizeof(struct gen));
		*ftrap=gbuf;
		mread(fp,&gbuf,sizeof(struct gen));
	}
	mread(fp,&obuf,sizeof(struct obj));
	while(obuf.olet) {
		obuf.nobj=fobj;
		fobj=alloc(sizeof(struct obj));
		*fobj=obuf;
		mread(fp,&obuf,sizeof(struct obj));
	}
	if(ferror(fp)) pline("error reading file %d",fp);
	return(0);
}
extern errno;
mklev()
{	/* cause temp file in lock to contain a new level... */
	int foo;

	if(dlevel==flags.maze) *buf='b';
	else if(dlevel==flags.maze-1) *buf='n';
	else *buf='a';
	*(buf+1)='\0';
	sprintf(buf+2,"%d",dlevel);
	switch(fork()){
	case 0:
		execl("./mklev","mklev",lock,buf,buf+2,0);
		execl("mklev","mklev",lock,buf,buf+2,0);
		panic("No mklev\n");
		break;
	case -1:
		pline("Cannot fork...Please wait...");
		sleep(5);
		mklev();
		return;
	default:
		while(wait(&foo)== -1 && errno==4) ;/* interrupts can happpen*/
		if(foo&0200) {
			pline("0%o--Core dumped",foo);
			mklev();
			return;
		}
		break;
	}
}
mkobj(let)
register let;
{
	register struct obj *otmp;

	otmp=alloc(sizeof(struct obj));
	otmp->nobj=fobj;
	fobj=otmp;
	otmp->minus=otmp->known=otmp->cursed=otmp->spe=0;
#ifdef MAGIC
	if(flags.magic) otmp->known=1;
#endif
	switch(let?let:rnd(20)) {
	case ')':
	case 1:
	case 2:
		otmp->olet=')';
		otmp->quan=(otmp->otyp=rn2(WEPNUM))<4?rn1(6,6):1;
		if(!rn2(11)) otmp->spe=rnd(3);
		else if(!rn2(10)) {
			otmp->cursed=otmp->minus=1;
			otmp->spe=rnd(3);
		}
		break;
	case '*':
	case 19:
	case 20:
		otmp->olet='*';
		otmp->quan=rn2(6)?2:1;
		otmp->otyp=rn2(GEMNUM);
		break;
	case '[':
	case 3:
	case 4:
		otmp->olet='[';
		otmp->otyp=rn1(ARMNUM,2);
		if(!rn2(10)) otmp->spe=rnd(3);
		else if(!rn2(9)) {
			otmp->spe=rnd(3);
			otmp->cursed=otmp->minus=1;
		}
		otmp->quan=1;
		break;
	case 5:
	case 6:
	case 14:
	case 16:
	case '!':
		otmp->olet='!';
		otmp->otyp=rn2(POTNUM);
		otmp->quan=1;
		break;
	case 7:
	case 8:
	case 15:
	case 17:
	case '?':
		otmp->olet='?';
		otmp->otyp=rn2(SCRNUM);
		otmp->quan=1;
		break;
	default:
#ifndef SMALL
		pline("%d causes food",let);
#endif
	case 9:
	case 10:
	case 11:
	case 18:
	case '%':
		otmp->olet='%';
		otmp->otyp=rn2(6)?0:1;
		otmp->quan=rn2(6)?1:2;
		break;
	case 12:
	case '/':
		otmp->olet='/';
		otmp->otyp=rn2(WANDNUM);
		if(otmp->otyp==WANDNUM-1) otmp->otyp=rn2(WANDNUM);
		if(otmp->otyp<3) otmp->spe=rn1(5,11);
		else otmp->spe=rn1(7,3);
		otmp->quan=1;
		break;
	case 13:
	case '=':
		otmp->olet='=';
		if(!rn2(7)) otmp->otyp=rn1(3,13);
		else otmp->otyp=rn2(RINGNUM);
		otmp->quan=1;
		if(otmp->otyp>12) {
			if(!rn2(3)) {
				otmp->cursed=otmp->minus=1;
				otmp->spe=rnd(2);
			} else otmp->spe=rnd(2);
		} else if(otmp->otyp==1 || otmp->otyp== 8 || otmp->otyp==9)
			otmp->cursed=1;
	}
}
