#include "hack.h"
#include "hack.vars"
		/* main() and variable declarations */

main()
{
#ifndef SMALL
	register char *sfoo,*tmp;
	FILE *fp;
#endif

#ifdef LOCKS
	lockcheck();
#else
	sprintf(lock,"%d",getppid());
	/* try to prevent too many dead temp files  (By using ppid) */
#endif
 	srand(getpid());
	cbin();
	signal(SIGQUIT,done2);
	signal(SIGINT,done1);
#ifndef SMALL
	setbuf(stdout,obuf);
	sfoo=getenv("HACKOPTS");
	if(sfoo) {
		do {
			tmp=sfoo;
			if(sfoo=index(sfoo,',')) *sfoo++=0;
			set1(tmp);
		} while(sfoo);
	}
#ifndef VTONL
	startup(getenv("TERM"));
#endif
	strcat(SAVEF,getlogin());
	if(fp=fopen(SAVEF,READ)) {
		puts("Restoring old save file.\n");
		fflush(stdout);
		{
			int tmp;
			struct obj tmpo,*otmp,*oend;
			struct monst *mtmp;
			int bar;
			char **foo;
			FILE *nfp;

			unlink(SAVEF);
			getlev(fp);
			mread(fp,&tmpo,sizeof(struct obj));
			oend=0;
			while(tmpo.olet) {
				otmp=alloc(sizeof(struct obj));
				/*V7*/
				*otmp=tmpo;
				if(oend) {
					oend->nobj=otmp;
					oend=otmp;
				} else invent=oend=otmp;
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
			oend->nobj=0;
			mread(fp,&flags,sizeof(struct flag));
			mread(fp,&dlevel,1);
			mread(fp,&moves,2);
			mread(fp,&u,sizeof(struct you));
			if(u.ustuck) {
				struct monst mbuf;

				mread(fp,&mbuf,sizeof(struct monst));
				mtmp=malloc(sizeof(struct monst));
				/*V7*/
				*mtmp=mbuf;
				u.ustuck=mtmp;
			} else mtmp=0;
			mread(fp,mon,sizeof(struct permonst)*8*7);
			mread(fp,oiden,20);
			mread(fp,potcol,POTNUM*2);
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
				if(!(nfp=fopen(lock,WRITE))) {
					printf("Cannot open %s for writing.\n",
 lock);
					exit(4);
				}
				savelev(nfp);
				fclose(nfp);
			}
			if(tmp!=2) { /* 2 means still on first level */
				rewind(fp);
				getlev(fp);
			}
			fclose(fp);
			if(mtmp) {
				mtmp->nmon=fmon;
				fmon=mtmp;
			}
			docrt();
		}
	} else {
#endif /* of non-small stuff */
	shufl(wannam,WANDNUM);
	shufl(potcol,GEMNUM);
	shufl(rinnam,RINGNUM);
	shufl(scrnam,SCRNUM);
	invent=alloc(sizeof(struct obj));
	invent->olet='%';
	invent->otyp=0;
	invent->quanmin=2;
	invent->nobj=uwep=alloc(sizeof(struct obj));
	uwep->olet=')';
	uwep->otyp=4;
	uwep->spestuff=0101;
	uwep->quanmin=1;
	uwep->nobj=uarm=alloc(sizeof(struct obj));
	uarm->olet='[';
	uarm->otyp=3;
	uarm->spestuff=0101;
	uarm->quanmin=1;
	uarm->nobj=0;
	u.uac=6;
	u.ulevel=1;
	u.uhunger=900;
	u.uhpmax=u.uhp=12;
	if(!rn2(20)) u.ustrmax=u.ustr=rn1(20,14);
	else u.ustrmax=u.ustr=16;
	ndaminc();
	flags.maze=rn1(6,25);
	mklev();
	cls();
	setsee();
#ifndef SMALL
	}
#endif
	flags.botl=flags.move=1;
	for(;;) {
		if(flags.move) {
			if(!u.ufast || moves%2==0) {
				if(fmon) movemon();
				if(!rn2(60)) {
					makemon((struct permonst *)0);
					fmon->mloc=0;
					rloc(fmon);
				}
			}
			if(u.ufast && !--u.ufast)
				pline("You slow down.");
			if(u.uconfused && !--u.uconfused)
				pline("You feel less confused now.");
			if(u.ublind && !--u.ublind) {
				pline("You can see again.");
				setsee();
			}
			if(u.uinvis && !--u.uinvis) {
				pru();
				pline("You are no longer invisible.");
			}
			++moves;
			--u.uhunger;
			if((u.uregen || u.ufeed) && moves%2) u.uhunger--;
			if(u.uhp<1) {
				pline("You die...");
				done("died");
			}
			if(u.uhp<u.uhpmax) {
				if(u.ulevel>9) {
					if(u.uregen || !(moves%3)) {
						flags.botl|=HP;
						u.uhp+=rnd(u.ulevel-9);
						if(u.uhp>u.uhpmax)
							u.uhp=u.uhpmax;
					}
				} else if(u.uregen || (!(moves%(22-u.ulevel
 *2)))) {
					flags.botl|=HP;
					u.uhp++;
				}
			}
			if(u.utel && !rn2(85)) tele();
			if(u.usearch) dosearch();
			if(u.uhunger<151 && u.uhs==0) {
				pline("You are beginning to feel hungry.");
				u.uhs=1;
				flags.botl|=DHS;
			} else if(u.uhunger<51 && u.uhs==1) {
				pline("You are beginning to feel weak.");
				u.uhs=2;
				losestr(1);
				flags.botl|=DHS;
			} else if(u.uhunger<1) {
				pline("You faint from lack of food.");
				if(u.uhs!=3) {
					u.uhs=3;
					flags.botl|=DHS;
				}
				nomul(-20);
				u.uhunger=rn1(4,22);
			}
		}
		flags.move=1;
		if(!multi) {
			if(flags.dscr) nscr();
			if(flags.botl) bot();
			if(flags.mv) flags.mv=0;
			rhack(parse());
		} else if(multi<0) {
			if(!++multi) pline("You can move again.");
		} else {
			if(flags.mv) {
				if(multi<80) --multi;
				domove();
			} else {
				--multi;
				rhack(save_cm);
			}
		}
	}
}
