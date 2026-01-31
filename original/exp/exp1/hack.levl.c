#include <stdio.h>
#include <signal.h>
#include "hack.h"

#define MAZLOC (&levl[(2*rnd(37))+1][(2*rnd(8))+1])
#define MAZX 2*rnd(37)+1
#define MAZY 2*rnd(8)+1

#define somex() ((rand()%(croom->hx-croom->lx))+croom->lx)
#define somey() ((rand()%(croom->hy-croom->ly))+croom->ly)
#define someloc() (&levl[(rand()%(croom->hx-croom->lx))+croom->lx][(rand()%(croom->hy-croom->ly))+croom->ly]) /* Sorry */

struct mkroom {
	char lx,hx,ly,hy;
};

struct mkroom *croom,*troom,*froom;
char nxcor,tx,ty;
int nroom; /* if I make it a char I get Register Overflows */

int comp();

mklev()
{
	char lowy,lowx;
	register struct lev *crm;
	register dix,diy;
	register unsigned tries =0;
	int *foo;
	struct mkroom xroom[15];

	cls();
	nroom=0;
	for(foo=(int *)levl;foo<(int *)&levl[79][21];*foo++=0)
		;	/* zero array */
	if(flags.maze==dlevel) {
		makemaz();
		return;
	}
	for(froom=croom=xroom;croom<&xroom[15];croom++) croom->hx= -1;
	croom=xroom;
	if(!maker(3,rn1(5,5),4,rn1(5,7))) maker(3,5,4,7);
	if(!maker(rn1(11,57),70,4,rn1(5,7))) maker(67,70,4,7);
	if(!maker(3,rn1(5,5),rn1(5,9),16)) maker(3,5,13,16);
	if(!maker(rn1(11,57),70,rn1(4,10),16)) maker(67,70,13,16);
	while(nroom<6) {
		for(lowy=rn1(3,3);lowy<15;lowy+=rn1(2,4)) 
			for(lowx=rn1(3,4);lowx<70;lowx+=rn1(2,7)) {
				if((lowy+=(rand()%5)-2)<3) lowy=3;
				else if(lowy>16) lowy=16;
				if(gettyp((&levl[lowx][lowy]))) continue;
				if(maker(lowx,rn1(10,lowx+2),lowy,
 rn1(4,lowy+2)) && nroom>13) goto jumpout;
			}
	}
jumpout: croom->hx= -1;	/* the only goto in hack */
	if(dlevel==flags.maze-1){
		int x,y;

		do {
			croom= &xroom[rand()%(nroom)];
			dnstair=someloc();
			getxy(&x,&y,dnstair-levl);
		/*FOO(Is this right?*/
		} while(!(x%2) || !(y%2) || g_at(dnstair,ftrap));
	} else {
		do {
			croom= &xroom[rand()%(nroom)];
			dnstair=someloc();
		} while(g_at(dnstair,ftrap));
	}
	setscr(dnstair,'>');
	troom=croom;
	do {
		dix=rand()%(nroom);
		croom= &xroom[dix];
		upstair=someloc();
	} while(croom==troom || g_at(upstair,ftrap));
	setscr(upstair,'<');
	for(croom=xroom;croom->hx>0;croom++) {
		if(!rn2(3)) {
			mmon();
			do {
				fmon->mloc=someloc();
			} while(upstair==fmon->mloc);
			sstat(fmon,SLEEP);
		}
		tries=0;
		while(!rn2(8-(dlevel/6))) {
			do crm=someloc();
			while(crm==dnstair || g_at(crm,ftrap)) ;
			tries+=mktrap(crm);
		}
		if(!tries && !rn2(3)) mkgold(someloc());
		if(!rn2(3)) {
			mkobj();
			setscr(fobj->oloc=someloc(),fobj->olet);
			while(!rn2(5)) {
				mkobj();
				setscr(fobj->oloc=someloc(),fobj->olet);
			}
		}
	}
	qsort(xroom,nroom,sizeof(struct mkroom),comp);
	croom=xroom;
	troom=croom+1;
	nxcor=0;
	mkpos();
	do {
		lowx=buf[0]+dx;
		lowy=buf[1]+dy;
		if(nxcor && !rn2(35)) {
			newloc();
			continue;
		}
		dix=abs(lowx-tx);
		diy=abs(lowy-ty);
		if(lowx==79 || lowx==0 || lowy==0 || lowy==21) {
			if(nxcor) {
				newloc();
				continue;
			} else {
				mklev();
				return;
			}
		}
		if(dy && dix>diy) {
			dy=0;
			dx= lowx>tx?-1:1;
		} else if(dx && diy>dix) {
			dx=0;
			dy=lowy>ty?-1:1;
		} 
		crm= &levl[lowx][lowy];
		if(!(gettyp(crm))) {
			settyp(crm,CORR);
			setscr(crm,'#');
			buf[0]=lowx;
			buf[1]=lowy;
			continue;
		}
		if(gettyp(crm)==CORR) {
			buf[0]=lowx;
			buf[1]=lowy;
			continue;
		}
		if(lowx==tx && lowy==ty) {
			dodoor(lowx,lowy);
			newloc();
			continue;
		}
		if(buf[0]+dx!=lowx || buf[1]+dy!=lowy) continue;
		if(dx) {
			if(ty<lowy) dy= -1;
			else dy=gettyp((&levl[lowx+dx][lowy-1]))==ROOM?1:-1;
			dx=0;
		} else {
			if(tx<lowx) dx= -1;
			else dx=gettyp((&levl[lowx-1][lowy+dy]))==ROOM?1:-1;
			dy=0;
		}
	} while (croom->hx>0 && troom->hx>0);
	u.uloc=upstair;
}
mkobj()
{
	register struct obj *otmp;

	otmp=(struct obj *)alloc(sizeof(struct obj));
	otmp->nobj=fobj;
	fobj=otmp;
	otmp->spestuff=otmp->quanmin=0;
	switch(rnd(20)) {
	case 1:
	case 2:
		otmp->olet=')';
		otmp->quanmin=((otmp->otyp=rn2(WEPNUM))<4?rn1(6,6):1);
		if(!rn2(11)) {
			otmp->spestuff=rnd(3);
			if(!rn2(3)) {
				scursed(otmp);
				sminus(otmp);
			}
		}
		break;
	case 19:
	case 20:
		otmp->olet='*';
		otmp->quanmin=rn2(6)?1:2;
		otmp->otyp=rn2(GEMNUM);
		break;
	case 3:
	case 4:
		otmp->olet='[';
		otmp->otyp=rn1(ARMNUM,2);
		otmp->quanmin=1;
		if(otmp->otyp>4 && rn2(3)) otmp->otyp=rn1(ARMNUM,2);
		if(!rn2(10)) {
			otmp->spestuff=rnd(3);
			if(!rn2(3)) {
				scursed(otmp);
				sminus(otmp);
			}
		}
		break;
	case 5:
	case 6:
	case 14:
	case 16:
		otmp->olet='!';
		otmp->quanmin=1;
		otmp->otyp=rn2(POTNUM);
		if(otmp->otyp>9 && !rn2(3)) otmp->otyp=rn2(POTNUM);
		break;
	case 7:
	case 8:
	case 15:
	case 17:
		otmp->olet='?';
		otmp->quanmin=1;
		otmp->otyp=rn2(SCRNUM);
		if(otmp->otyp>5 && otmp->otyp<9) {
			if(!rn2(4)) otmp->otyp=12;
			else if(rn2(2)) otmp->otyp=rn2(SCRNUM);
		}
		break;
	case 9:
	case 10:
	case 11:
	case 18:
		otmp->olet='%';
		otmp->otyp=rn2(6)?0:1;
		otmp->quanmin=rn2(6)?1:2;
		break;
	case 12:
		otmp->olet='/';
		otmp->quanmin=1;
		otmp->otyp=rn2(WANDNUM);
		if(!rn2(5)) otmp->spestuff=0;
		else if(otmp->otyp<3) otmp->spestuff=rn1(15,20);
		else otmp->spestuff=rn1(5,4);
		break;
	case 13:
		otmp->olet='=';
		if(!rn2(8)) otmp->otyp=rn1(4,13);
		else otmp->otyp=rn2(RINGNUM);
		otmp->quanmin=1;
		if(otmp->otyp>12) {
			if(otmp->otyp==16) otmp->spestuff=rn1(4,2);
			else otmp->spestuff=rnd(3);
			if(!rn2(3)) {
				scursed(otmp);
				sminus(otmp);
			}
		} else if(otmp->otyp==1 || otmp->otyp==8 || otmp->otyp==9)
			scursed(otmp);
	}
}
comp(x,y)
register struct mkroom *x,*y;
{
	if(x->lx<y->lx) return(-1);
	return(x->lx>y->lx);
}
mkpos()
{
	if(troom->hx<0||croom->hx<0) return;
	if(troom->lx>croom->hx) {
		buf[0]=croom->hx+1;
		dx=1;
		buf[1]=rn1(croom->hy-croom->ly,croom->ly);
		dy=0;
		tx=troom->lx-1;
		ty=troom->ly+rnd(troom->hy-troom->ly)-1;
	} else if(troom->hy<croom->ly) {
		buf[1]=croom->ly-1;
		dy= -1;
		dx=0;
		buf[0]=croom->lx+rnd(croom->hx-croom->lx)-1;
		tx=troom->lx+rnd(troom->hx-troom->lx)-1;
		ty=troom->hy+1;
	} else if(troom->hx<croom->lx) {
		buf[0]=croom->lx-1;
		dx= -1;
		dy=0;
		tx=troom->hx+1;
		buf[1]=croom->ly+rnd(croom->hy-croom->ly)-1;
		ty=troom->ly+rnd(troom->hy-troom->ly)-1;
	} else {
		buf[1]=croom->hy+1;
		dy=1;
		dx=0;
		buf[0]=croom->lx+rnd(croom->hx-croom->lx)-1;
		tx=troom->lx+rnd(troom->hx-troom->lx)-1;
		ty=troom->ly-1;
	}
	if(gettyp((&levl[buf[0]+dx][buf[1]+dy]))) {
		if(nxcor) newloc();
		else {
			dodoor(buf[0],buf[1]);
			buf[0]+=dx;
			buf[1]+=dy;
		}
		return;
	}
	dodoor(buf[0],buf[1]);
}
dodoor(x,y)
register x,y;
{
	register struct lev *room;

	room= &levl[x][y];
	if(gettyp(room)!=WALL) return;
	if(!rn2(18-(dlevel/3))) settyp(room,SDOOR);
	else {
		setscr(room,'+');
		settyp(room,DOOR);
	}
}
newloc()
{
	register a,b;

	++croom;
	++troom;
	if(nxcor || croom->hx<0 || troom->hx<0) {
		if(nxcor++>rn1(nroom,4)) {
			croom= (froom+nroom);
			return;
		}
		do {
			a=rn2(nroom);
			b=rn2(nroom);
			croom=(froom+a);
			troom=(froom+b);
		} while(croom==troom || (troom==croom+1 && !rn2(3)));
	}
	mkpos();
}

makemaz()
{	/* this is all Kenny's fault.  He seems to have his x and y reversed */
	int x,y,a,q,sp;
	int dir,dirs[5];
	register zx,zy;
	int stack[200];
	struct obj *otmp;
	register struct lev *room;

	for(x=2; x<19; x++)
		for(y=2; y<77; y++) settyp((&levl[y][x]), (x%2 && y%2) ? 0 : 1);
	zx=MAZY;
	zy=MAZX;
	sp=1;
	stack[1]=100*zx +zy;
	while(sp){
		x=stack[sp]/100;
		y=stack[sp]%100;
		settyp((&levl[y][x]),2);
		q=0;
		for(a=0; a<4; a++)
			if(okay(x,y,a)) dirs[q++]=a;
		if(q) {
			dir = dirs[rn2(q)];
			move(&x,&y,dir);
			settyp((&levl[y][x]),0);
			move(&x,&y,dir);
			stack[++sp]=100*x +y;
		} else sp--;
	}
	for(x=2;x<77;x++) {	/* this is mine */
		for(y=2;y<19;y++) {
			if(gettyp((room= &levl[x][y]))==WALL) settyp(room,0);
			else {
				settyp(room,CORR);
				setscr(room,'#');
			}
		}
	}
	for(x=rn1(8,11);x;x--) {
		mkobj();
		setscr(fobj->oloc=MAZLOC,fobj->olet);
	}
	for(x=rn1(5,7);x;x--) {
		mmon();
		fmon->mloc=MAZLOC;
		sstat(fmon,SLEEP);
	}
	for(x=rn1(6,7);x;x--) mkgold(MAZLOC);
	for(x=rn1(6,7);x;x--) mktrap(MAZLOC);
	while(g_at(upstair=MAZLOC,ftrap)) ;
	setscr(upstair,'<');
	setscr((&levl[zy][zx]),'\"');
	otmp=(struct obj *)alloc(sizeof(struct obj));
	otmp->nobj=fobj;
	fobj=otmp;
	otmp->oloc= &levl[zy][zx];
	otmp->olet='\"';
	dnstair=0;
}
move(x,y,dir)
register int *x, *y;
register int dir;
{
	switch(dir){
		case 0: --(*x); break;
		case 1: (*y)++; break;
		case 2: (*x)++; break;
		case 3: --(*y); break;
	}
}
okay(x,y,dir)
int x,y;
register int dir;
{
	move(&x,&y,dir);
	move(&x,&y,dir);
	if(x<3 || y<3 || x>17 || y>75 || gettyp((&levl[y][x])))
		return(0);
	else
		return(1);
}
mktrap(loc)
register struct lev *loc;
{
	register struct gen *gtmp;
	register num;

	if(dlevel>8 && !rn2(8)) return(mkmim(rn2(2)));
	num=rn2(TRAPNUM);
	gtmp=(struct gen *)alloc(sizeof(struct gen));
	gtmp->gflag=num;
	gtmp->gloc=loc;
	gtmp->ngen=ftrap;
	ftrap=gtmp;
	return(0);
}
mkgold(loc)
register struct lev *loc;
{
	register struct gen *gtmp;

	gtmp=(struct gen *)alloc(sizeof(struct gen));
	gtmp->ngen=fgold;
	setscr(gtmp->gloc=loc,'$');
	gtmp->gflag= 1+((1+(rand()%(dlevel+2)))*(1+(rand()%30)));
	fgold=gtmp;
}
mmon()
{
	register tmp;

	makemon(&mon[((tmp=rn2(dlevel/3+1))>7?rn2(8):tmp)][rn2(7)]);
	sstat(fmon,SLEEP);
}
maker(lowx,hix,lowy,hiy)
char lowx,hix,lowy,hiy;
{
	register tmpx;
	register struct lev *ltmp,*lmax;

	if(hix>77) hix=77;
	if(hiy>19) hiy=19;
	for(tmpx=lowx-2;tmpx<hix+3;tmpx++) {
		for(ltmp= &levl[tmpx][lowy-2],lmax= &levl[tmpx][hiy+3];
 ltmp!=lmax;ltmp++) {
			if(gettyp(ltmp)) return(0);
		}
	}
	if(dlevel<rn2(14)) {
		for(tmpx=lowx-1;tmpx<hix+2;tmpx++)
			for(ltmp= &levl[tmpx][lowy-1],lmax= &levl[tmpx][hiy+2];
 ltmp!=lmax;ltmp++) setlit(ltmp);
	}
	croom->lx=lowx;
	croom->hx=hix;
	croom->ly=lowy;
	croom++->hy=hiy;
	tmpx=lowx-1;
	ltmp= &levl[tmpx][lowy-1];
	lmax= &levl[tmpx][hiy+1];
	setscr(ltmp,'-');
	settyp(ltmp,WALL);
	while(++ltmp!=lmax) {
		setscr(ltmp,'|');
		settyp(ltmp,WALL);
	}
	setscr(ltmp,'-');
	settyp(ltmp,WALL);
	while(++tmpx<=hix) {
		ltmp= &levl[tmpx][lowy-1];
		lmax= &levl[tmpx][hiy+1];
		setscr(ltmp,'-');
		settyp(ltmp,WALL);
		while(++ltmp!=lmax) {
			setscr(ltmp,'.');
			settyp(ltmp,ROOM);
		}
		setscr(ltmp,'-');
		settyp(ltmp,WALL);
	}
	ltmp= &levl[tmpx][lowy-1];
	lmax= &levl[tmpx][hiy+1];
	setscr(ltmp,'-');
	settyp(ltmp,WALL);
	while(++ltmp!=lmax) {
		setscr(ltmp,'|');
		settyp(ltmp,WALL);
	}
	setscr(ltmp,'-');
	settyp(ltmp,WALL);
	++nroom;
	return(1);
}
mkmim(num)
register num;
{
	if(!num) {
		makemon(&mon[2][3]);
		if(dnstair) fmon->mloc=someloc();
		else fmon->mloc=MAZLOC;
		sinv(fmon);
	} else if((!dnstair) || rn2(2)) {
		makemon(&mon[5][2]);
		if(dnstair) setscr(fmon->mloc=someloc(),'$');
		else setscr(fmon->mloc=MAZLOC,'$');
		return(1);
	} else {
		makemon(&mon[5][2]);
		if(rn2(2)){
			if(rn2(2)) fmon->mloc= &levl[croom->hx+1][somey()];
			else fmon->mloc= &levl[croom->lx-1][somey()];
		} else {
			if(rn2(2)) fmon->mloc= &levl[somex()][croom->hy+1];
			else fmon->mloc= &levl[somex()][croom->ly-1];
		}
		setscr(fmon->mloc,'+');
	}
	return(0);
}
