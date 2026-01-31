#include "hack.h"
		/* user commands */

/* rhack.  The procedure that blew the C optimizer.   (That's why it is
in three parts now) */

rhack(cmd)
register char cmd;
{
	register struct lev *loc;
	register struct obj *otmp,*ot1;
	register struct monst *mtmp;
	register num;
	char *foo;

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
#ifndef SMALL
	case 'f':
#ifndef NUNUMS
	case ',':
#endif
		cmd=getchar();
		if(movecm(cmd)) {
			flags.mv=3;
			multi+=80;
			domove();
		} else pline("Bad f direction.");
		return;
		break;
	case '?':
		if(fork()) wait(0);
		else {
			cbout();/* so return works in cr3 */
			cls();
			fflush(stdout);
			execl("/usr/bin/cr3","cr3","moves",0);
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
			if(foo=getenv("SHELL")) execl(foo,foo,0);
			execl("/bin/sh","sh",0);
			pline("sh: cannot execute.");
			exit(1);
			break;
		default:
			signal(SIGINT,SIG_IGN);
			signal(SIGQUIT,SIG_IGN);
			wait(0);
			cbin();
			signal(SIGINT,done1);
			signal(SIGQUIT,done2);
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
	case 's':
		dosearch();
		break;
	case 'e':
		otmp=getobj("%","eat");
		if(!otmp) {
			multi=flags.move=0;
			return;
		}
		if(otmp->otyp) {
			pline("That was a delicious fruit.");
			lesshungry(160);
		} else {
			pline("That really hit the spot!");
			lesshungry(900);
			multi= -5;
		}
		useup(otmp);
		break;
	case 'Q':
		pline("Really quit?");
#ifndef SMALL
		fflush(stdout);
#endif
		if(getchar()=='y') done("quit");
		flags.move=multi=0;
		break;
	case 'q':
		if(!(otmp=getobj("!","drink"))) multi=flags.move=0;
		else drink1(otmp);
		break;
	case 'w':
		multi=0;
		if(uwep && gcursed(uwep)) pline(CURSED);
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
		if(u.ustuck || u.uloc!=dnstair) {
			pline("You can't go down now.");
			flags.move=multi=0;
			return;
		}
	case 'Z':
		movemon();
		seeoff(1);
		dodown();
		setsee();
		docrt();
		break;
	case '<':
		if(u.ustuck || u.uloc!=upstair) {
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
#ifndef NONUM
	case '5':
#endif
	case ' ': break;
	case 't':
		
		if((!(otmp=getobj(")","throw"))) || !getdir()) {
			flags.move=multi=0;
			return; /* sets dir to direction */
		}
		if(otmp==uarm || otmp==uleft || otmp==uright) {
			pline("You are wearing that!");
			return;
		}
		if(otmp==uwep) {
			if(gcursed(otmp)) {
				pline(CURSED);
				return;
			} else uwep=0;
		}
		if(otmp->olet==')') {
			if(otmp->otyp<3 && uwep && uwep->otyp==otmp->otyp+11)
				num=20;
			else num=6;
		} else num=5;
		mtmp=bhit(dir,num);
		loc=(struct lev *)dir;
		if(gquan(otmp)>1) {
			otmp->quanmin--;
			if((ot1=g_at(loc,fobj)) && ot1->otyp==otmp->otyp &&
 ot1->olet==otmp->olet && otmp->spestuff==ot1->spestuff) ot1->quanmin++;
			else {
				ot1=(struct obj *)alloc(sizeof(struct obj));
				/*V7*/
				*ot1= *otmp;
				squan(ot1,1);
				ot1->nobj=fobj;
				fobj=ot1;
				ot1->oloc=loc;
			}
		} else {
			if(otmp==invent) invent=invent->nobj;
			else {
				for(ot1=invent;ot1->nobj!=otmp;ot1=ot1->nobj) ;
				ot1->nobj=otmp->nobj;
			}
			otmp->nobj=fobj;
			fobj=otmp;
			otmp->oloc=loc;
		}
		if(mtmp) {
			if(otmp->olet==')' && otmp->otyp<3 && (!uwep ||
 uwep->otyp!=otmp->otyp+11)) amon(mtmp,otmp,-4);
			else amon(mtmp,otmp,0);
			if(mtmp->mhp<1) newsym(loc);
		} else newsym(loc);
		break;
	case 'd':
		if(!(otmp=getobj((char *)0,"drop"))) {
			multi=flags.move=0;
			return;
		}
		if(gquan(otmp)>1) {
			pline("Drop how many? (%d max)?",gquan(otmp));
			getlin(buf);
			num=0;
			foo=buf;
			while(*foo>='0' && *foo<='9')
				num=(num*10)+ *foo++ -'0';
			if(num<1 || num>gquan(otmp)) {
				pline("You can't drop that many!");
				multi=flags.move=0;
				return;
			} else if(num!=gquan(otmp)) {
				struct obj *objs;

				objs=(struct obj *)alloc(sizeof(struct obj));
				*objs= *otmp;	/*V7*/
				otmp->quanmin-=num;
				squan(objs,num);
				objs->oloc=u.uloc;
				objs->nobj=fobj;
				fobj=objs;
				strcpy(buf,"You dropped ");
				doname(fobj,&buf[12]);
				pline(buf);
			} else dodr1(otmp);
		} else dodr1(otmp);
		break;
	case 'p':
		if(!(otmp=getobj("/","zap"))) {
			flags.move=multi=0;
			return;
		}
		if(!gospe(otmp)) {
			pline(NOTHIN);
			return;
		}
		if(otmp->otyp<3){
			otmp->spestuff--;
			switch(otmp->otyp){
			case 0: litroom();
				break;
			case 1:
				{
					struct gen *gtmp;
					char tmp;

					for(gtmp=ftrap;gtmp;gtmp=gtmp->ngen) {
						gtmp->gflag|=SEEN;
						tmp=getscr(gtmp->gloc);
						if(tmp=='.' || tmp=='#')
							newsym(gtmp->gloc);
					}
				}
				break;
			case 2: makemon((struct permonst *)0);
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
		otmp->spestuff--;
		if(otmp->otyp<11) {
			if(mtmp=bhit(dir,rn1(8,6))) {
				if(otmp->otyp==3) {
					if(rnd(20)<10+mtmp->data->ac) {
						hit(WAND,mtmp);
						mtmp->mhp-=d(2,12);
						if(mtmp->mhp<1) killed(mtmp);
					} else miss(WAND,mtmp);
				} else switch(otmp->otyp) {
				case 4: sspeed(mtmp,MSLOW);
					break;
				case 5: sspeed(mtmp,MFAST);
					break;
				case 6: if(index("WVZ&",mtmp->data->mlet)) {
						mtmp->mhp-=rnd(8);
						if(mtmp->mhp<1) killed(mtmp);
						else sstat(mtmp,FLEE);
					}
					break;
				case 7: newcham(mtmp,&mon[rn2(8)][rn2(7)]);
					oiden[7]|=WANN;
					break;
				case 8: scan(mtmp);
					break;
				case 9: rloc(mtmp);
					break;
				case 10: sinv(mtmp);
					if(getcan(mtmp->mloc))
						newsym(mtmp->mloc);
					break;
				}
			}
		} else {
			buzz(otmp->otyp-11,u.uloc,dir);
			oiden[otmp->otyp]|=WANN;
		}
		break;
	case 'W':
		multi=0;
		if(uarm) {
			pline("Already wearing armor.");
			return;
		}
		if(!(otmp=getobj("[","wear"))) flags.move=0;
		else {
			movemon();
			movemon();
			uarm=otmp;
			multi= -3;
			sknown(uarm);
			u.uac-=uarm->otyp;
			if(gminus(uarm)) u.uac+=gospe(uarm);
			else u.uac-=gospe(uarm);
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
#ifndef SMALL
					fflush(stdout);
#endif
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
			case 1:
				u.utel=1;
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
				{
					struct monst *mtmp;

					for(mtmp=fmon;mtmp;mtmp=mtmp->nmon) {
						if(gcham(mtmp)) {
						       newcham(mtmp,&mon[6][6]);
						}
					}
				}
				break;
			case 13:
				for(num=gospe(otmp);num;num--) {
					if(gminus(otmp)) {
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
				if(gminus(otmp)) u.uac+=gospe(otmp);
				/* watch this */
				else u.uac-=gospe(otmp);
				flags.botl|=AC;
				break;
			case 16:
				flags.botl|=(HP|HPM);
				if(gminus(otmp)) {
					u.uhp-=gospe(otmp);
					u.uhpmax-=gospe(otmp);
#ifndef SMALL
					if(u.uhp<1) killer="cursed ring";
#endif
				} else {
					u.uhp+=gospe(otmp);
					u.uhpmax+=gospe(otmp);
				}
				break;
			}
			prinv(otmp);
		}
		break;
	case 'T':
		multi=0;
		if(!uarm) pline("Not wearing any!");
		else if(gcursed(uarm)) pline(CURSED);
		else {
			movemon();
			movemon();
			nomul(-3);
			u.uac+=uarm->otyp;
			if(gminus(uarm)) u.uac-=gospe(uarm);
			else u.uac+=gospe(uarm);
			flags.botl|=AC;
			otmp=uarm;
			uarm=0;
			were(otmp);
		}
		break;
	case 'R':
		multi=0;
		if(!(otmp=getobj("=","remove"))) flags.move=0;
		else if(gcursed(otmp)) pline(CURSED);
		else if(otmp==uleft) {
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
	default:
		if(cmd<' ') pline("Unknown command '^%c'",cmd+'@');
		else pline("Unknown command '%c'",cmd);
		multi=flags.move=0;
	}
}
drink1(otmp)
register struct obj *otmp;
{
	register struct monst *mtmp;
	register num;

	switch(otmp->otyp){
	case 0: pline("You feel great!");
		if(u.ustr<u.ustrmax) {
			u.ustr=u.ustrmax;
			flags.botl|=STR;
			ndaminc();
		}
		break;
	case 1: pline("This tastes like liquid fire!");
		u.uconfused+=d(3,8);
		if(u.uhp<u.uhpmax) u.uhp++,flags.botl|=HP;
		if(!rn2(4)) {
			pline("You pass out.");
			multi= -rnd(15);
		}
		break;
	case 2: pline("You turn invisible.");
		newsym(u.uloc);
		u.uinvis+=rn1(15,31);
		break;
	case 3: pline("This is fruit juice.");
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
		nomul(-(rn1(10,25)));
		break;
	case 6: if(!fmon) {
			nothin(otmp);
			return;
		} else {
			int x,y;

			cls();
			for(mtmp=fmon;mtmp;mtmp=mtmp->nmon) {
				getxy(&x,&y,mtmp->mloc-levl);
				at(x,y+2,mtmp->data->mlet);
			}
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
			int x,y;

			cls();
			for(objs=fobj;objs;objs=objs->nobj) {
				getxy(&x,&y,objs->oloc-levl);
				at(x,y+2,objs->olet);
			}
			flags.topl=0;
			pline("You sense objects.");
			more();
			docrt();
			flags.topl=0;
		}
		break;
	case 8: pline("Yech! This is poison!");
		losestr(rn1(4,3));
		u.uhp-=rnd(10);
		flags.botl|=HP;
#ifndef SMALL
		if(u.uhp<1) killer="poison potion";
#endif
		break;
	case 9: pline("What?  Where am I?");
		u.uconfused+=rn1(7,16);
		break;
	case 10: pline("Wow do you feel strong!");
		if(u.ustr<118) {
			if(u.ustr>17) u.ustr+=rnd(118-u.ustr);
			else u.ustr++;
			if(u.ustr>u.ustrmax) u.ustrmax=u.ustr;
			ndaminc();
			flags.botl|=STR;
		}
		break;
	case 11: pline("You are moving faster.");
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
read1(otmp)
register struct obj *otmp;
{
	register num;
	register struct lev *loc;
	register struct monst *mtmp;
	struct gen *gtmp;

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
		if(uleft) rcursed(uleft);
		if(uright) rcursed(uright);
		if(uarm) rcursed(uarm);
		if(uwep) rcursed(uwep);
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
		makemon((struct permonst *)0);
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
		{
			struct permonst *mptmp;
			int found;

			found=1;
			do {
 				pline("What monster (Letter)? ");
				flags.topl=1;
#ifndef SMALL
				fflush(stdout);
#endif
				num=getchar();
				for(mptmp=mon;mptmp->mname;mptmp++) {
					if(mptmp->mlet==num) {
						mptmp->mlet=0;
						found=0;
						break;
					}
				}
			} while(found);
			for(mtmp=fmon;mtmp;mtmp=mtmp->nmon)
				if(mtmp->data==mptmp){
					delmon(mtmp);
					if(getscr(mtmp->mloc)==num)
						newsym(mtmp->mloc);
				}
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
			int x,y;

			cls();
			for(gtmp=fgold;gtmp;gtmp=gtmp->ngen) {
				getxy(&x,&y,gtmp->gloc-levl);
				at(x,y+2,'$');
			}
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
			case ')': sknown(otmp);
				break;
			case '/': sknown(otmp);
				oiden[otmp->otyp]|=WANN;
				break;
			case '=': 
				if(otmp->otyp>12) sknown(otmp);
				oiden[otmp->otyp]|=RINN;
				break;
			}
			prinv(otmp);
		}
		return;
	case 13:
		pline("You found a map!");
		for(loc=levl;loc<&levl[79][21];loc++) {
			if(gettyp(loc)==SDOOR) {
				settyp(loc,DOOR);
				setscr(loc,'+');
				on(loc);
			} else if(gettyp(loc) && gettyp(loc)<ROOM &&
 !gseen(loc)) on(loc);
		}
		if(!gseen(upstair)) on(upstair);
		if(!gseen(dnstair)) on(dnstair);
		break;
	case 14:
		pline("The scroll erupts in a tower of flame!");
		if(u.ufireres) pline("You are uninjured.");
		else {
			u.uhp-=(num=rnd(6));
			u.uhpmax-=num;
			flags.botl|=(HP|HPM);
#ifndef SMALL
			if(u.uhp<1) killer="scroll of flame";
#endif
		}
		break;
	}
	if(!(oiden[otmp->otyp]&SCRN)) {
		if(otmp->otyp>6) {
			oiden[otmp->otyp]|=SCRN;
			u.urexp+=10;
		} else if(!scrcall[otmp->otyp]) docall(otmp);
	}
	useup(otmp);
}
dosearch()
{
	register struct gen *tgen;
	register struct lev *loc,*min;

	for(min=u.uloc-23;min<u.uloc+43;min+=22) {
		for(loc=min;loc<min+3;loc++) {
			if(gettyp(loc)==SDOOR && !rn2(7)) {
				settyp(loc,DOOR);
				atl(loc,'+');
				nomul(0);
			} else {
				for(tgen=ftrap;tgen;tgen=tgen->ngen)
					if(tgen->gloc==loc && (!rn2(8) ||
 ((!u.usearch) && tgen->gflag&SEEN))) {
						nomul(0);
						pline("You find a%s",
 traps[tgen->gflag&037]);
						if(!(tgen->gflag&SEEN)) {
							tgen->gflag|=SEEN;
							atl(loc,'^');
						}
					}
			}
		}
	}
}
