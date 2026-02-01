#include "../compat.h"
#include "hack.h"

extern char nul[],NOTHIN[],IDENT[],WAND[],EMPTY[],RUST[],CURSED[];
extern char DONTH[];
/* rhack.  The procedure that blew the C optimizer.   (That's why it is
in three parts now) */

void rhack(char cmd)
{
	register struct obj *otmp;
	register struct monst *mtmp;
	register int num;
	char zx,zy;
	char *foo;

#ifdef MAGIC
	if(!cmd) return;	/* only happens in magic mode */
#endif
	if(movecm(cmd)) {
		if(multi) flags.mv=1;
		domove();
		return;
	}
	if(movecm(cmd+040)) {
		flags.mv=2;
		multi+=80;
		domove();
		return;
	}
	switch(cmd) {
#ifdef MAGIC
	case '':
		domagic(0);  /* Modern: prototype requires int param */
		flags.move=0;
		return;
#endif
#ifndef SMALL
	case 'f':
		cmd=getchar();
		if(movecm(cmd)) {
			flags.mv=3;
			multi+=80;
			domove();
		} else pline("Bad f direction.");
		return;
#endif
	case 's':
		dosearch();
		break;
	case 'e':
		otmp=getobj("%","eat");
		if(!otmp) {
			multi=flags.move=0;
			return;
		}
#ifndef SMALL
		if(!rn2(7)) {
			pline("Blecch!!  Rotten food!");
			if(!rn2(4)) {
				pline("You feel light headed.");
				u.uconfused+=d(2,4);
			} else if(!rn2(4)&& !u.ublind) {
				pline("Everything suddenly goes dark.");
				u.ublind=d(2,10);
				seeoff(0);
			} else if(!rn2(3)) {
				pline("The world spins and goes dark.");
				nomul(-rnd(10));
			}
			if(otmp->otyp) lesshungry(20);
			else lesshungry(200);
		} else
#endif
		if(otmp->otyp) {
			pline("That was a delicious fruit.");
			lesshungry(100);
		} else {
			pline("That really hit the spot!");
			lesshungry(900);
			multi= -5;
		}
		useup(otmp);
		break;
	case 'Q':
		done1(0);  /* Modern: prototype requires int param */
		flags.move=multi=0;
		break;
	case 'q':
		if(!(otmp=getobj("!","drink"))) multi=flags.move=0;
		else drink1(otmp);
		break;
	case 'w':
		multi=0;
		if(uwep && uwep->cursed) pline(CURSED);
		else if(!(otmp=getobj(")","wield"))) flags.move=0;
		else if(otmp->olet=='[') {
			pline("You can't wield armor.");
			flags.move=0;
		} else {
			uwep=otmp;
			prinv(otmp);
		}
		break;
	case 'r':
		if(!(otmp=getobj("?","read"))) multi=flags.move=0;
		else read1(otmp);
		break;
	case '':
		docrt();
		multi=flags.move=0;
		break;
#ifndef SMALL
	case '?':
		if(fork()) wait(0);
		else {
			cbout();/* so return works in cr3 */
			cls();
			fflush(stdout);
			/* Original 1982: execl("/usr/bin/cr3","cr3","moves",0); */
		execl("/usr/bin/cr3","cr3","moves",(char *)NULL);  /* Modern: 64-bit NULL */
		}
		cbin();	/* guess why */
		getret();
		docrt();
		break;
	case '!':
		switch(fork()) {
		case -1:
			pline("Fork failed.");
			break;
		case 0:
			cbout();
			cls();
			fflush(stdout);
			setuid(getuid());
			chdir(getenv("HOME"));
			/* Original 1982: execl(foo,foo,0); / execl("/bin/sh","sh",0); */
			if(foo=getenv("SHELL")) execl(foo,foo,(char *)NULL);  /* Modern: 64-bit NULL */
			execl("/bin/sh","sh",(char *)NULL);  /* Modern: 64-bit NULL */
			pline("sh: cannot execute.");
			exit(1);
			break;
		default:
			signal(SIGINT,SIG_IGN);
			signal(SIGQUIT,SIG_IGN);
			wait(0);
			cbin();
			signal(SIGINT,done1);
#ifdef MAGIC
			signal(SIGQUIT,getgid()==42?domagic:done2);
#else
			signal(SIGQUIT,done2);
#endif
			docrt();
			break;
		}
		multi=flags.move=0;
		break;
	case 'I':
		if(!invent) pline(EMPTY);
		else {
			pline("what letter?");
			fflush(stdout);
			num=getchar();
			if(num>='A' && num<='Z') num=26+num-'A';
			else num-='a';
			flags.topl=1;
			for(otmp=invent;otmp && num;otmp=otmp->nobj,num--) ;
			if(otmp) {
				doname(otmp,buf);
				pline(buf);
			} else pline(DONTH);
		}
		flags.move=multi=0;
		break;
#endif
	case 'i':
		if(!invent) pline(EMPTY);
		else doinv((char *)0);
		flags.move=multi=0;
		break;
	case 'c':
		if(otmp=getobj("?!=/","call")) docall(otmp);
		flags.move=0;
		break;
	case '>':
		if(u.ustuck || u.ux!=xdnstair || u.uy!=ydnstair) {
			pline("You can't go down now.");
			flags.move=multi=0;
			return;
		}
		movemon();
		seeoff(1);
		dodown();
		setsee();
		docrt();
		break;
	case '<':
		if(u.ustuck || u.ux!=xupstair || u.uy!=yupstair) {
			pline("You can't go up now.");
			flags.move=multi=0;
			return;
		}
		movemon();
		seeoff(1);
		doup();
		setsee();
		docrt();
		break;
	case ' ': break;
	case 't':
		
		if((!(otmp=getobj(")","throw"))) || !getdir()) {
			flags.move=multi=0;
			return; /* sets dx and dy to direction */
		}
		if(otmp==uarm || otmp==uleft || otmp==uright) {
			pline("You are wearing that!");
			return;
		}
		if(otmp==uwep) {
			if(otmp->cursed) {
				pline(CURSED);
				return;
			} else uwep=0;
		}
		if(otmp->olet==')') {
			if(otmp->otyp<3 && uwep && uwep->otyp==otmp->otyp+11)
				num=20;
			else num=6;
		} else num=5;
		mtmp=bhit(dx,dy,num);
		loseone(otmp,zx=dx,zy=dy);	/* separate one out from list */
		if(mtmp) {
			if(otmp->olet==')' && otmp->otyp<3 && (!uwep ||
 uwep->otyp!=otmp->otyp+11)) amon(mtmp,otmp,-4);
			else amon(mtmp,otmp,0);
			if(mtmp->mhp<1) newsym(zx,zy);
		} else newsym(zx,zy);
		break;
#ifndef SMALL
	case '': pline(0);
		break;
	case 'o':
		pline("What option do you want to set? ");
		getlin(buf);
		flags.topl=1;
		set1(buf);
		flags.move=0;
		break;
	case 'S': dosave();
		break;
#endif
	case 'd':
		if(!(otmp=getobj((char *)0,"drop"))) {
			multi=flags.move=0;
			return;
		} else {
			if(otmp->quan>1) {
				pline("Drop how many? (%d max)?",
 otmp->quan);
				getlin(buf);
				num=0;
				foo=buf;
				while(*foo>='0' && *foo<='9')
					num=(num*10)+ *foo++ -'0';
				if(num<1 || num>otmp->quan) {
					pline("You can't drop that many!");
					multi=flags.move=0;
					return;
				} else if(num!=otmp->quan) {
					struct obj *objs;

					objs=alloc(sizeof(struct obj));
					*objs= *otmp;
					otmp->quan-=num;
					objs->quan=num;
					objs->ox=u.ux;
					objs->oy=u.uy;
					objs->nobj=fobj;
					fobj=objs;
				} else dodr1(otmp);
			} else dodr1(otmp);
		}
		strcpy(buf,"You dropped ");
		doname(fobj,&buf[12]);
		pline(buf);
		break;
	case 'p':
		if(!(otmp=getobj("/","zap"))) {
			flags.move=multi=0;
			return;
		}
		if(!otmp->spe) {
			pline(NOTHIN);
			return;
		}
		if(otmp->otyp<3){
			otmp->spe--;
			switch(otmp->otyp){
			case 0: litroom();
				break;
			case 1:
#ifndef SMALL
				if(!findit()) return;
#else
				pline("What a strange wand!");
#endif
				break;
			case 2: makemon(0);
				mnexto(fmon);
				break;
			}
			if(!oiden[otmp->otyp]&WANN) {
				u.urexp+=10;
				oiden[otmp->otyp]|=WANN;
			}
			return;
		}
		if(!getdir()) return;
		otmp->spe--;
		if(otmp->otyp<10) {
			if(mtmp=bhit(dx,dy,rn1(8,6))) {
				if(otmp->otyp==3) {
					if(rnd(20)<10+mtmp->data->ac) {
						hit(WAND,mtmp);
						mtmp->mhp-=d(2,12);
						if(mtmp->mhp<1) killed(mtmp);
					} else miss(WAND,mtmp);
				} else switch(otmp->otyp) {
				case 4: mtmp->mspeed=MSLOW;
					break;
				case 5: mtmp->mspeed=MFAST;
					break;
				case 6: if(index("WVZ&",mtmp->data->mlet)) {
						mtmp->mhp-=rnd(8);
						if(mtmp->mhp<1) killed(mtmp);
						else mtmp->mstat=FLEE;
					}
					break;
				case 7: newcham(mtmp,&mon[rn2(8)][rn2(7)]);
					oiden[7]|=WANN;
					break;
				case 8: mtmp->mcan=1;
					break;
				case 9: rloc(mtmp);
					break;
				}
			}
			return;
		} else if(otmp->otyp==10) {
#ifdef SMALL
			pline("The wand glows slightly.");
#else
			if(dx && dy) {
				if(rn2(2)) dx=0;
				else dy=0;
			}
			zx=u.ux+dx;
			zy=u.uy+dy;
			if(levl[zx][zy].typ==CORR) num=CORR;
			else num=ROOM;
			for(;;) {
				at(zx,zy,'*');
				if(!xdnstair){
					if(zx<3 || zx>76 || zy<3 || zy>18)
						break;
					if(levl[zx][zy].typ==WALL){
						levl[zx][zy].typ=ROOM;
						break;
					}
				} else if(num==ROOM || num==10){
					if(levl[zx][zy].typ!=ROOM &&
 levl[zx][zy].typ) {
						if(levl[zx][zy].typ!=CORR)
							levl[zx][zy].typ=DOOR;
						if(num==10) break;
						num=10;
					} else if(!levl[zx][zy].typ)
						levl[zx][zy].typ=CORR;
				} else {
					if(levl[zx][zy].typ!=CORR &&
 levl[zx][zy].typ) {
						levl[zx][zy].typ=DOOR;
						break;
					} else levl[zx][zy].typ=CORR;
				}
				if(zx==1 || zx==78 || zy==1 || zy==21)
					break;
				newsym(zx,zy);
				zx+=dx;
				zy+=dy;
			}
			newsym(zx,zy);
#endif
		} else buzz(otmp->otyp-11,u.ux,u.uy,dx,dy);
		oiden[otmp->otyp]|=WANN;
		break;
	case 'W':
		multi=0;
		if(uarm) {
			flags.move=0;
			pline("Already wearing armor.");
			return;
		}
		if(!(otmp=getobj("[","wear"))) flags.move=0;
		else {
			movemon();
			movemon();
			uarm=otmp;
			nomul(-3);
			uarm->known=1;
			u.uac-=uarm->otyp;
			if(uarm->minus) u.uac+=uarm->spe;
			else u.uac-=uarm->spe;
			flags.botl|=AC;
			prinv(uarm);
		}
		break;
	case 'P':
		if(uleft && uright) {
			pline("You are wearing two rings.");
			flags.move=0;
		} else if(!(otmp=getobj("=","wear"))) flags.move=0;
		else if(otmp==uleft || otmp==uright) {
			pline("You are already wearing that.");
			flags.move=0;
		} else {
			if(uleft) uright=otmp;
			else if(uright) uleft=otmp;
			else {
				do {
 					pline("Right or Left? ");
					fflush(stdout);
					num=getchar();
					flags.topl=1;
					if(num=='\033') {
						flags.move=0;
						return;
					}
				} while(!index("rl",num));
				if(num=='l') uleft=otmp;
				else uright=otmp;
			}
			switch(otmp->otyp){
			case 0: break;
			case 1: u.utel=1;
				break;
			case 2: u.uregen=1;
				break;
			case 3: u.usearch=1;
				break;
			case 4: u.ucinvis=1;
				break;
			case 5: u.ustelth=1;
				break;
			case 6: u.ufloat=1;
				break;
			case 7: u.upres=1;
				break;
			case 8: u.uagmon=1;
				break;
			case 9: u.ufeed=1;
				break;
			case 10: u.ufireres=1;
				break;
			case 11: u.ucoldres=1;
				break;
			case 12: u.ucham=1;
				rescham();
				break;
			case 13:
				for(num=otmp->spe;num;num--) {
					if(otmp->minus) {
						if(u.ustr>18) {
							u.ustr-=15;
							if(u.ustr<18) u.ustr=18;
						} else {
							if(u.ustr>4) u.ustr--;
							u.ustrmax--;
						}
					} else {
						if(u.ustr>17) {
							u.ustr+=15;
							u.ustrmax+=15;
						} else {
							u.ustr++;
							if(u.ustrmax>17)
								u.ustrmax+=15;
							else u.ustrmax++;
						}
					}
				}
				flags.botl|=STR;
			case 14: /* notice that gain strength falls through */
				ndaminc();
				break;
			case 15:
				if(otmp->minus) u.uac+=otmp->spe;
				/* watch this */
				else u.uac-=otmp->spe;
				flags.botl|=STR;
				break;
			case 16:
				if(otmp->minus) {
					u.uhp-=otmp->spe;
					u.uhpmax-=otmp->spe;
					flags.botl|=(HP|HPM);
#ifndef SMALL
					if(u.uhp<1) killer="cursed ring";
#endif
				} else {
					u.uhp+=otmp->spe;
					u.uhpmax+=otmp->spe;
					flags.botl|=(HP|HPM);
				}
				break;
#ifndef SMALL
			default:
				pline("Bad ring %d",otmp->otyp);
#endif
			}
			prinv(otmp);
		}
		break;
	case 'T':
		multi=0;
		if(!uarm) pline("Not wearing any!");
		else if(uarm->cursed) pline(CURSED);
		else {
			movemon();
			movemon();
			nomul(-3);
			u.uac+=uarm->otyp;
			if(uarm->minus) u.uac-=uarm->spe;
			else u.uac+=uarm->spe;
			flags.botl|=AC;
			otmp=uarm;
			uarm=0;
			were(otmp);
		}
		break;
	case 'R':
		multi=0;
		if(!(otmp=getobj("=","remove"))) {
			flags.move=0;
			return;
		} else if(otmp->cursed) {
			pline(CURSED);
		} else if(otmp==uleft) {
			uleft=0;
			were(otmp);
			ringoff(otmp);
		} else if(otmp==uright) {
			uright=0;
			were(otmp);
			ringoff(otmp);
		} else {
			pline("You can't remove that.");
			flags.move=0;
		}
		break;
	default:
		if(cmd<' ') pline("Unknown command '^%c'.",cmd+'@');
		else pline("Unknown command '%c'",cmd);
		multi=flags.move=0;
	}
}
void were(struct obj *otmp)
{
	strcpy(buf,"You were wearing ");
	doname(otmp,&buf[17]);
	pline(buf);
}
void drink1(struct obj *otmp)
{
	register struct monst *mtmp;
	register int num;

	switch(otmp->otyp){
	case 0: pline("You feel great!");
		if(u.ustr<u.ustrmax) {
			u.ustr=u.ustrmax;
			flags.botl|=STR;
			ndaminc();
		}
		break;
#ifdef SMALL
	case 1: pline("This tastes like liquid fire!");
#else
	case 1: switch(rn2(4)) {
		case 0: pline("This is an excellent (but powerful) wine.");
			break;
		case 1: pline("This is White Lightning!");
			break;
		case 2: pline("Ooph!  120 Proof grain alcohol!");
			break;
		case 3: pline("Gee, a can of Billy Beer!");
			break;
		}
#endif
		u.uconfused+=d(3,8);
		if(u.uhp<u.uhpmax) losehp(-1);
		if(!rn2(4)) {
			pline("You pass out.");
			multi= -rnd(15);
		}
		break;
	case 2: pline("You turn invisible.");
		newsym(u.ux,u.uy);
		u.uinvis+=rn1(15,31);
		break;
#ifdef SMALL
	case 3: pline("This is fruit juice.");
#else
	case 3: pline("Wow! This tastes like watermelon juice.");
#endif
		lesshungry(20);
		break;
	case 4: pline("You begin to feel better.");
		num=u.uhpmax/3;
		if(u.uhp+num>u.uhpmax) {
			u.uhp= ++u.uhpmax;
			flags.botl|=(HP|HPM);
		} else {
			u.uhp+=num;
			flags.botl|=HP;
		}
		if(u.ublind) u.ublind=1;
		break;
	case 5: pline("You are frozen!");
		nomul(-(rn1(10,5)));
		break;
	case 6: if(!fmon) {
			nothin(otmp);
			return;
		} else {
			cls();
			for(mtmp=fmon;mtmp;mtmp=mtmp->nmon)
				at(mtmp->mx,mtmp->my,mtmp->data->mlet);
			flags.topl=0;
			pline("You sense monsters.");
			more();
			flags.topl=0;
			docrt();
		}
		break;
	case 7: if(!fobj) {
			nothin(otmp);
			return;
		} else {
			struct obj *objs;

			cls();
			for(objs=fobj;objs;objs=objs->nobj)
				at(objs->ox,objs->oy,objs->olet);
			flags.topl=0;
			pline("You sense objects.");
			more();
			docrt();
			flags.topl=0;
		}
		break;
	case 8: pline("Yech! Poison!");
		losestr(rn1(4,3));
		losehp(rnd(10));
#ifndef SMALL
		if(u.uhp<1) killer="poison potion";
#endif
		break;
	case 9: pline("What?  Where am I?");
		u.uconfused+=rn1(7,16);
		break;
	case 10: pline("Wow, do you feel strong!");
		if(u.ustr<118) {
			if(u.ustr>17) u.ustr+=rnd(118-u.ustr);
			else u.ustr++;
			if(u.ustr>u.ustrmax) u.ustrmax=u.ustr;
			ndaminc();
			flags.botl|=STR;
		}
		break;
#ifdef SMALL
	case 11: pline("You are moving faster.");
#else
	case 11: pline("You feel yourself speeding up.");
#endif
		u.ufast+=rn1(10,100);
		break;
	case 12: pline("The world goes dark.");
		u.ublind+=rn1(100,250);
		seeoff(0);
		break;
	case 13: 
		pluslvl();
		break;
	case 14: pline("You feel much better.");
		num=u.uhpmax/2+u.uhpmax/4;
		if(u.uhp+num>u.uhpmax) {
			u.uhp=(u.uhpmax+=2);
			flags.botl|=(HP|HPM);
		} else {
			u.uhp+=num;
			flags.botl|=HP;
		}
		if(u.ublind) u.ublind=1;
		break;
	}
	if(!(oiden[otmp->otyp]&POTN)) {
		if(otmp->otyp>1) {
			oiden[otmp->otyp]|=POTN;
			u.urexp+=10;
		} else if(!potcall[otmp->otyp]) docall(otmp);
	}
	useup(otmp);
}
void read1(struct obj *otmp)
{
	register int num;
	register struct monst *mtmp;
	struct permonst *mptmp;
	struct gen *gtmp;
	char *foo;
	char zx,zy;

#ifndef SMALL
	pline("As you read the scroll, it disappears.");
#endif
	switch(otmp->otyp) {
	case 0:
		if(!uarm) {
			nothin(otmp);
			return;
		}
		pline("Your armor glows green.");
		plusone(uarm);
		u.uac--;
		flags.botl|=AC;
		break;
	case 1:
		pline("Your hands start glowing blue.");
		u.umconf=1;
		break;
	case 2:
		pline("This scroll seems blank.");
		break;
	case 3:
		pline("You feel like someone is helping you.");
		if(uleft) uleft->cursed=0;
		if(uright) uright->cursed=0;
		if(uarm) uarm->cursed=0;
		if(uwep) uwep->cursed=0;
		break;
	case 4:
		if(!uwep || uwep->olet!=')') {
			nothin(otmp);
			return;
		}
		chwepon("green");
		plusone(uwep);
		break;
	case 5:
		makemon(0);
		mnexto(fmon);
		break;
	case 6:
		if(!uwep || uwep->olet!=')') {
			nothin(otmp);
			return;
		}
		chwepon("black");
		minusone(uwep);
		break;
	case 7:
		if(u.uac>9 || !uarm) {
			nothin(otmp);
			return;
		}
		u.uac++;
		flags.botl|=AC;
		pline(RUST);
		minusone(uarm);
		break;
	case 8:
		pline("Behold, a scroll of genocide!");
		zy=1;
		do {
 			pline("What monster (Letter)? ");
			flags.topl=1;
			fflush(stdout);
			zx=getchar();
			for(mptmp=mon;mptmp->mname;mptmp++) {
				if(mptmp->mlet==zx) {
					mptmp->mlet=0;
					zy=0;
					break;
				}
			}
		} while(zy);
#ifndef SMALL
		pline("Goodbye to all %ss.",mptmp->mname);
#endif
		for(mtmp=fmon;mtmp;mtmp=mtmp->nmon)
			if(mtmp->data==mptmp){
				delmon(mtmp);
				if(levl[mtmp->mx][mtmp->my].scrsym==zx)
					newsym(mtmp->mx,mtmp->my);
			}
		break;
	case 9:
		litroom();
		break;
	case 10:
		tele();
		break;
	case 11:
		if(!fgold) {
			nothin(otmp);
			return;
		} else {
			cls();
			for(gtmp=fgold;gtmp;gtmp=gtmp->ngen)
				at(gtmp->gx,gtmp->gy,'$');
			flags.topl=0;
			pline("You sense gold!");
			more();
			flags.topl=0;
			docrt();
		}
		break;
	case 12:
		pline("This is an identify scroll.");
		useup(otmp);
		oiden[12]|=SCRN;
		otmp=getobj((char *)0,IDENT);
		if(otmp) {
			switch(otmp->olet) {
			case '!': oiden[otmp->otyp]|=POTN;
				break;
			case '?': oiden[otmp->otyp]|=SCRN;
				break;
			case '[':
			case ')': otmp->known=1;
				break;
			case '/': otmp->known=1;
				oiden[otmp->otyp]|=WANN;
				break;
			case '=': 
				if(otmp->otyp>12) otmp->known=1;
				oiden[otmp->otyp]|=RINN;
				break;
			}
			prinv(otmp);
		}
		return;
	case 13:
		pline("You found a map!");
		{
			struct rm *foo;
			/* Modern: &levl[0][0] for type-safe pointer arithmetic (levl
			   decays to struct rm (*)[22], not struct rm *) */
			struct rm *base = &levl[0][0];

			for(foo=base;foo<&levl[79][21];foo++) {
				if(foo->typ==SDOOR) {
					foo->typ=DOOR;
					foo->scrsym='+';
					foo->new=1;
					on((int)((foo-base)%80),(int)((foo-base)/80));
				} else if((foo->typ==CORR || foo->typ==WALL ||
foo->typ==DOOR) && !foo->seen) foo->new=1,on((int)((foo-base)%80),(int)((foo-base)/80));
			}
		}
		if(!levl[xupstair][yupstair].seen)
			levl[xupstair][yupstair].new=1,
			on(xupstair,yupstair);
		if(!levl[xdnstair][ydnstair].seen)
			levl[xdnstair][ydnstair].new=1,
			on(xdnstair,ydnstair);
		break;
	case 14:
		pline("The scroll erupts in a tower of flame!");
		if(u.ufireres) pline("You are uninjured.");
		else {
			u.uhp-=(num=rnd(6));
#ifndef SMALL
			if(u.uhp<1) killer="scroll of fire";
#endif
			u.uhpmax-=num;
			flags.botl|=(HP|HPM);
		}
		break;
	}
	if(!(oiden[otmp->otyp]&SCRN)) {
		if(otmp->otyp>7) {
			oiden[otmp->otyp]|=SCRN;
			u.urexp+=10;
		} else if(!scrcall[otmp->otyp]) docall(otmp);
	}
	useup(otmp);
}
#ifndef SMALL
void set1(char *str)
{
	register int num;

	if(!strncmp(str,"no",2)) {
		num=0;
		str+=2;
	} else num=1;
	if(!strcmp(str,"one")) flags.one=num;
	else if(!strcmp(str,"step")) flags.step=num;
	else if(!strcmp(str,"flush")) flags.flush=num;
	else if(!strncmp(str,"name=",5)) {
		str+=5;
		if(uname) mfree(uname);
		uname=alloc(strlen(str)+1);
		strcpy(uname,str);
	} else if(!strncmp(str,"term=",5)) {
		startup(str+5);
		docrt();
	} else pline("Unknown option '%s'",str);
}
#endif
