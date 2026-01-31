#include "hack.h"
		/* lock files, temp files, etc */

glo(foo)
register foo;
{
	register char *tf;

	tf=lock;
	while(*tf && *tf!='.') tf++;
	*tf++='.';
	sprintf(tf,"%d",foo);
}
dodown()
{
	register FILE *fp;

	glo(dlevel);
	fp=fopen(lock,WRITE);
	savelev(fp);
	fclose(fp);
	glo(++dlevel);
	if((fp=fopen(lock,READ))==0) mklev();
	else {
		getlev(fp);
		fclose(fp);
	}
	u.uloc=upstair;
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
	u.uloc=dnstair;
}
/* Note that lint hates this (Moral: don't lint it) */
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
	bwrite(fp,&upstair,2);
	bwrite(fp,&dnstair,2);
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

	if(fp==0 || fread(levl,1,3520,fp)!=3520) return;
	mread(fp,&omoves,2);
	mread(fp,&upstair,2);
	mread(fp,&dnstair,2);
	mread(fp,&sbuf,sizeof(struct stole));
	while(sbuf.smon) {
		mread(fp,&mbuf,sizeof(struct monst));
		if(!mbuf.data->mlet) {
			do mread(fp,&obuf,sizeof(struct obj));
			while(obuf.olet);
		} else {
			sbuf.sobj=0;
			mread(fp,&obuf,sizeof(struct obj));
			while(obuf.olet) {
				obuf.nobj=sbuf.sobj;
				sbuf.sobj=alloc(sizeof(struct obj));
				/*V7*/
				*(sbuf.sobj)=obuf;
				mread(fp,&obuf,sizeof(struct obj));
			}
			mbuf.nmon=fmon;
			fmon=alloc(sizeof(struct monst));
			/*V7*/
			*fmon=mbuf;
			sbuf.smon=fmon;
			sbuf.nstole=fstole;
			fstole=alloc(sizeof(struct stole));
			/*V7*/
			*fstole=sbuf;
		}
		mread(fp,&sbuf,sizeof(struct stole));
	}
	tmoves=moves-omoves;
	mread(fp,&mbuf,sizeof(struct monst));
	while(mbuf.mloc){
		if(mbuf.data->mlet) {
			if(index(mregen,mbuf.data->mlet))
				mbuf.mhp+=mbuf.mhp+tmoves;
			else mbuf.mhp+=tmoves/20;
			if(mbuf.mhp>mbuf.orig_hp || mbuf.mhp<1)
				mbuf.mhp=mbuf.orig_hp;
			mbuf.nmon=fmon;
			fmon=alloc(sizeof(struct monst));
			/*V7*/
			*fmon=mbuf;
		}
		mread(fp,&mbuf,sizeof(struct monst));
	}
	mread(fp,&gbuf,sizeof(struct gen));
	while(gbuf.gflag) {
		gbuf.ngen=fgold;
		fgold=alloc(sizeof(struct gen));
		/*V7*/
		*fgold=gbuf;
		mread(fp,&gbuf,sizeof(struct gen));
	}
	mread(fp,&gbuf,sizeof(struct gen));
	while(gbuf.gloc) {
		gbuf.ngen=ftrap;
		ftrap=alloc(sizeof(struct gen));
		/*V7*/
		*ftrap=gbuf;
		mread(fp,&gbuf,sizeof(struct gen));
	}
	mread(fp,&obuf,sizeof(struct obj));
	while(obuf.olet) {
		obuf.nobj=fobj;
		fobj=alloc(sizeof(struct obj));
		/*V7*/
		*fobj=obuf;
		mread(fp,&obuf,sizeof(struct obj));
	}
}
