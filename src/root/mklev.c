#include <stdio.h>
#include <signal.h>
#include "hack.h"
#include "../compat.h"

/* Modern: declarations for rnd.c functions (no shared header in original) */
extern int rn1(int, int);
extern int rn2(int);
extern int rnd(int);
extern int d(int, int);

/* Modern: level file header for versioned save format */
#define LEVEL_MAGIC "FLEV"
#define LEVEL_VERSION 1

#define MAZX (2*rnd(37)+1)
#define MAZY (2*rnd(8)+1)

#define somex() ((rand()%(croom->hx-croom->lx))+croom->lx)
#define somey() ((rand()%(croom->hy-croom->ly))+croom->ly)
#define bwrite(fd,loc,num) fwrite(loc,1,num,fd)

struct rm levl[80][22];
struct monst *fmon;
struct obj *fobj;
struct gen *fgold,*ftrap;

struct mkroom {
	char lx,hx,ly,hy;
} room[15],*croom,*troom;

/* forward declarations */
int comp(const void *, const void *);
void mkpos(void);
void dodoor(int, int);
void newloc(void);
void makemaz(void);
void move(int *, int *, int);
int okay(int, int, int);
int mktrap(int, int);
void mkgold(int, int, int);
void makemon(void);
void savelev(void);
int maker(char, char, char, char);
void makecor(int, int);
void panic(char *, ...);
int mkmim(int);
void mkobj(void);
#ifdef DEBUG
void out(char *, ...);
#endif

char dlevel;
char nul[20];	/* contains zeros */
/* Modern: coordinate variables changed to unsigned char for safe array subscript use */
unsigned char nxcor,x,y,tx,ty; /* for corridors and other things... */
char dx,dy; /* directional deltas — must stay signed (original 1982 comment) */
int nroom;
int rnum;

unsigned char xdnstair,xupstair,ydnstair,yupstair;
char *tfile,*tspe,**args;

#ifdef DEBUG
FILE *tfoo;
/* Original 1982: FILE *fopen(); — now provided by <stdio.h> */
#endif

/* Original 1982: int abort(); — now provided by <stdlib.h> */

/* Modern: wrapper for abort() as signal handler */
static void sig_abort(int signum) { (void)signum; abort(); }

int main(int argc, char *argv[])
{
	unsigned char lowy,lowx; /* Modern: unsigned char for safe array subscript use */
	register unsigned tries =0;
	/* Original 1982: register struct gen *gtmp; — unused, suppressed */

#ifdef DEBUG
	if(getenv("OUT")) {
		tfoo=fopen("mkout","a");
		setbuf(tfoo,(char *)NULL);
	}
#endif
	signal(SIGINT,sig_abort);
	signal(SIGQUIT,SIG_DFL);
	args=argv;
	tfile=argv[1];
	tspe=argv[2];
	dlevel=atoi(argv[3]);
	if(argc<4) panic("Too few arguments!!");
	else if(argc==5) {
		srand(atoi(argv[4]));
		rnum=atoi(argv[4]);
#ifdef DEBUG
		out("begins at %s(%d)...\n",argv[4],getpid());
#endif
	} else {
		srand(getpid());
		rnum=getpid();
	}
#ifdef DEBUG
	out("Making %s %s %d\n",tfile,tspe,dlevel);
#endif
	if(dlevel>30) {/* bugtrap*/
		fprintf(stderr,"Fuck you %s!\r\n\n",getlogin());
		kill(0,9);
	}
	if(*tspe=='b') {
		makemaz();
		savelev();
	}
	croom=room;
	if(!maker(3,rn1(5,5),4,rn1(5,7))) maker(3,5,4,7);
	if(!maker(rn1(9,59),70,4,rn1(4,7))) maker(67,70,4,7);
	if(!maker(3,rn1(5,5),rn1(5,9),16)) maker(3,5,13,16);
	if(!maker(rn1(9,59),70,rn1(5,9),16)) maker(67,70,13,16);
	while(nroom<6) {
#ifdef DEBUG
		out("loop on %d\n",nroom);
#endif
		for(lowy=rn1(3,3);lowy<15;lowy+=rn1(2,4))
			for(lowx=rn1(3,4);lowx<70;lowx+=rn1(2,7)) {
				if((lowy+=(rand()%5)-2)<3) lowy=3;
				else if(lowy>16) lowy=16;
				if(levl[lowx][lowy].typ) continue;
				if(maker(lowx,rn1(10,lowx+2),lowy,
 rn1(4,lowy+2)) && nroom>13) goto jumpout;
			}
	}
jumpout: croom->hx= -1;	/* the only goto in hack */
#ifdef DEBUG
	out("nroom %d\n",nroom);
#endif
	if(*tspe=='n'){
		do {
			croom= &room[rand()%(nroom)];
			xdnstair=somex();
			ydnstair=somey();
		} while(!(xdnstair%2) || !(ydnstair%2) ||
g_at(xdnstair,ydnstair,ftrap));
	} else {
		do {
			croom= &room[rand()%(nroom)];
			xdnstair=somex();
			ydnstair=somey();
		} while(g_at(xdnstair,ydnstair,ftrap));
	}
	levl[xdnstair][ydnstair].scrsym='>';
#ifdef DEBUG
	out("dn @%d,%d\n",xdnstair,ydnstair);
#endif
	troom=croom;
	do {
		croom= &room[rand()%(nroom)];
		xupstair=somex();
		yupstair=somey();
	} while(croom==troom);
	levl[xupstair][yupstair].scrsym='<';
#ifdef DEBUG
	out("up @%d,%d\n",xupstair,yupstair);
#endif
	for(croom=room;croom->hx>0;croom++) {
#ifdef DEBUG
		out("Room %d (%d %d--%d %d): ",croom-room,croom->lx,croom->hx,
 croom->ly,croom->hy);
#endif
		if(!rn2(3)) {
			makemon();
			do {
				fmon->mx=somex();
				fmon->my=somey();
			} while(xupstair==fmon->mx && yupstair==fmon->my);
#ifdef DEBUG
			out("Monster @%d %d\n",fmon->mx,fmon->my);
#endif
			fmon->mstat=SLEEP;
		}
		tries=0;
		while(!rn2(8-(dlevel/6))) {
			int tx,ty;

			do {
				tx=somex();
				ty=somey();
			} while((xdnstair==tx && ydnstair==ty) ||
 g_at(tx,ty,ftrap)) ;
			tries+=mktrap(tx,ty);
#ifdef DEBUG
			out("TRAP %d %d\n",tx,ty);
#endif
		}
		if(!tries && !rn2(3)) {
			mkgold(0,somex(),somey());
#ifdef DEBUG
			out("GOLD\n");
#endif
		}
		if(!rn2(3)) {
			mkobj();
#ifdef DEBUG
			out("OBJ\n");
#endif
			levl[fobj->ox=somex()][fobj->oy=somey()].
 scrsym=fobj->olet;
			while(!rn2(5)) {
				mkobj();
				levl[fobj->ox=somex()][fobj->oy=somey()].
 scrsym=fobj->olet;
#ifdef DEBUG
				out("OBJ1\n");
#endif
			}
		}
	}
#ifdef DEBUG
	out("Past loop\n");
#endif
	qsort(room,nroom,sizeof(struct mkroom),comp);
	croom=room;
	troom=croom+1;
	nxcor=0;
#ifdef DEBUG
	out("begin mkpos\n");
#endif
	mkpos();
	do makecor(x+dx,y+dy);
	while (croom->hx>0 && troom->hx>0);
	savelev();
}
void mkobj(void)
{
	register struct obj *otmp;

	otmp=(struct obj *)malloc(sizeof(struct obj));
	otmp->nobj=fobj;
	fobj=otmp;
	otmp->minus=otmp->known=otmp->cursed=otmp->spe=0;
	switch(rnd(20)) {
	case 1:
	case 2:
		otmp->olet=')';
		otmp->otyp=rn2(WEPNUM);
		if(otmp->otyp<4) otmp->quan=rn1(6,6);
		else otmp->quan=1;
		if(!rn2(12)) otmp->spe=rnd(3);
		else if(!rn2(11)) {
			otmp->cursed=otmp->minus=1;
			otmp->spe=rnd(3);
		}
		break;
	case 19:
	case 20:
		otmp->olet='*';
		otmp->quan=rn2(6)?1:2;
		otmp->otyp=rn2(GEMNUM);
		break;
	case 3:
	case 4:
		otmp->olet='[';
		otmp->otyp=rn1(ARMNUM,2);
		if(otmp->otyp>4 && rn2(3)) otmp->otyp=rn1(ARMNUM,2);
		if(!rn2(10)) {
			otmp->spe=rnd(3);
			if(!rn2(3)) otmp->cursed=otmp->minus=1;
		}
		otmp->quan=1;
		break;
	case 5:
	case 6:
	case 14:
	case 16:
		otmp->olet='!';
		otmp->otyp=rn2(POTNUM);
		if(otmp->otyp>9 && !rn2(3)) otmp->otyp=rn2(POTNUM);
		otmp->quan=1;
		break;
	case 7:
	case 8:
	case 15:
	case 17:
		otmp->olet='?';
		otmp->otyp=rn2(SCRNUM);
		if(otmp->otyp>5 && otmp->otyp<9) {
			if(!rn2(4)) otmp->otyp=12;
			else if(rn2(2)) otmp->otyp=rn2(SCRNUM);
		}
		otmp->quan=1;
		break;
	default:
		printf("Ungood mkobj.\n\n");
	case 9:
	case 10:
	case 11:
	case 18:
		otmp->olet='%';
		otmp->otyp=rn2(6)?0:1;
		otmp->quan=rn2(6)?1:2;
		break;
	case 12:
		otmp->olet='/';
		otmp->otyp=rn2(WANDNUM);
		if(!rn2(5)) otmp->spe=0;
		else if(otmp->otyp<3) otmp->spe=rn1(15,20);
		else otmp->spe=rn1(5,4);
		otmp->quan=1;
		break;
	case 13:
		otmp->olet='=';
		if(!rn2(8)) otmp->otyp=rn1(4,13);
		else otmp->otyp=rn2(RINGNUM);
		otmp->quan=1;
		if(otmp->otyp>12) {
			if(!rn2(3)) otmp->cursed=otmp->minus=1;
			if(otmp->otyp==16) otmp->spe=rn1(3,2);
			else otmp->spe=rnd(3);
		} else if(otmp->otyp==1 || otmp->otyp==8 || otmp->otyp==9)
			otmp->cursed=1;
	}
}
/* Original 1982: comp(x,y) register struct mkroom *x,*y; */
/* Modern: qsort comparator requires (const void *, const void *) signature */
int comp(const void *a, const void *b)
{
	const struct mkroom *x = a;
	const struct mkroom *y = b;
	if(x->lx<y->lx) return(-1);
	return(x->lx>y->lx);
}
void mkpos(void)
{
	if(troom->hx<0||croom->hx<0) return;
	if(troom->lx>croom->hx) {
		x=croom->hx+1;
		dx=1;
		y=rn1(croom->hy-croom->ly,croom->ly);
		dy=0;
		tx=troom->lx-1;
		ty=troom->ly+rnd(troom->hy-troom->ly)-1;
	} else if(troom->hy<croom->ly) {
		y=croom->ly-1;
		dy= -1;
		dx=0;
		x=croom->lx+rnd(croom->hx-croom->lx)-1;
		tx=troom->lx+rnd(troom->hx-troom->lx)-1;
		ty=troom->hy+1;
	} else if(troom->hx<croom->lx) {
		x=croom->lx-1;
		dx= -1;
		dy=0;
		tx=troom->hx+1;
		y=croom->ly+rnd(croom->hy-croom->ly)-1;
		ty=troom->ly+rnd(troom->hy-troom->ly)-1;
	} else {
		y=croom->hy+1;
		dy=1;
		dx=0;
		x=croom->lx+rnd(croom->hx-croom->lx)-1;
		tx=troom->lx+rnd(troom->hx-troom->lx)-1;
		ty=troom->ly-1;
	}
#ifdef DEBUG
	if(tfoo) fprintf(tfoo,"%d,%d to %d,%d by %d,%d\n",x,y,tx,ty,
 dx,dy);
#endif
	if(levl[x+dx][y+dy].typ) {
		if(nxcor) newloc();
		else {
			dodoor(x,y);
			x+=dx;
			y+=dy;
		}
		return;
	}
	dodoor(x,y);
}
void dodoor(int x, int y)
{
	if(levl[x-1][y].typ==DOOR || levl[x+1][y].typ==DOOR ||
levl[x][y+1].typ==DOOR || levl[x][y-1].typ==DOOR || levl[x-1][y].typ==SDOOR ||
levl[x+1][y].typ==SDOOR || levl[x][y-1].typ==SDOOR || levl[x][y+1].typ==SDOOR)
		return;
	if(levl[x][y].typ!=WALL) return;
	if(!rn2(8)) levl[x][y].typ=SDOOR;
	else {
		levl[x][y].scrsym='+';
		levl[x][y].typ=DOOR;
	}
}
void newloc(void)
{
	register int a,b;

	++croom;
	++troom;
	if(nxcor||croom->hx<0||troom->hx<0) {
		if(nxcor++>rn1(nroom,4)) {
			croom= &room[nroom];
#ifdef DEBUG
			out("newloc ends\n");
#endif
			return;
		}
		do {
			a=rn2(nroom);
			b=rn2(nroom);
			croom= &room[a];
			troom= &room[b];
		} while(croom==troom || (troom==(croom+1) && !rn2(3)));
#ifdef DEBUG
		if(tfoo) fprintf(tfoo,"croom %d troom %d\n",a,b);
	} else if(tfoo) fputs("newloc.\n",tfoo);
#else
	}
#endif
	mkpos();
}

void makemaz(void)
{	/* this is all Kenny's fault.  He seems to have his x and y reversed */
	int x,y,a,q,sp;
	int dir,dirs[5];
	register int zx,zy;
	int stack[200];
	struct obj *otmp;

	for(x=2; x<19; x++)
		for(y=2; y<77; y++) levl[y][x].typ= (x%2 && y%2) ? 0 : 1 ;
	zx=MAZY;
	zy=MAZX;
	sp=1;
	stack[1]=100*zx +zy;
	while(sp){
		x=stack[sp]/100;
		y=stack[sp]%100;
		levl[y][x].typ=2;
		q=0;
		for(a=0; a<4; a++)
			if(okay(x,y,a)) dirs[q++]=a;
		if(q) {
			dir = dirs[rn2(q)];
			move(&x,&y,dir);
			levl[y][x].typ=0;
			move(&x,&y,dir);
			stack[++sp]=100*x +y;
		} else sp--;
	}
	for(x=2;x<77;x++)	/* this is mine */
		for(y=2;y<19;y++) {
			if(levl[x][y].typ==WALL) levl[x][y].typ=0;
			else {
				levl[x][y].typ=CORR;
				levl[x][y].scrsym='#';
			}
		}
	for(x=rn1(8,11);x;x--) {
		mkobj();
		levl[(fobj->ox=MAZX)][(fobj->oy=MAZY)].scrsym=fobj->olet;
	}
	for(x=rn1(5,7);x;x--) {
		makemon();
		fmon->mx=MAZX;
		fmon->my=MAZY;
		fmon->mstat=SLEEP;
	}
	for(x=rn1(6,7);x;x--) mkgold(0,MAZX,MAZY);
	for(x=rn1(6,7);x;x--) mktrap(MAZX,MAZY);
	while(g_at(xupstair=MAZX,yupstair=MAZY,ftrap)) ;
	levl[xupstair][yupstair].scrsym='<';
	levl[zy][zx].scrsym='\"';
	otmp=(struct obj *)malloc(sizeof(struct obj));
	otmp->nobj=fobj;
	fobj=otmp;
	otmp->ox=zy;
	otmp->oy=zx;
	otmp->olet='\"';
	xdnstair=ydnstair=0;
}
void move(int *x, int *y, int dir)
{
	switch(dir){
		case 0: --(*x); break;
		case 1: (*y)++; break;
		case 2: (*x)++; break;
		case 3: --(*y); break;
	}
}
int okay(int x, int y, int dir)
{
	move(&x,&y,dir);
	move(&x,&y,dir);
	if(x<3 || y<3 || x>17 || y>75 || levl[y][x].typ!=0)
		return(0);
	else
		return(1);
}
int mktrap(int x, int y)
{
	register struct gen *gtmp;
	register int num;

	if(dlevel>8 && !rn2(8)) return(mkmim(rn2(2)));
	num=rn2(TRAPNUM);
	gtmp=(struct gen *)malloc(sizeof(struct gen));
	gtmp->gflag=num;
	gtmp->gx=x;
	gtmp->gy=y;
	gtmp->ngen=ftrap;
	ftrap=gtmp;
	return(0);
}
void mkgold(int num, int x, int y)
{
	register struct gen *gtmp;

	gtmp=(struct gen *)malloc(sizeof(struct gen));
	gtmp->ngen=fgold;
	levl[gtmp->gx=x][gtmp->gy=y].scrsym='$';
	gtmp->gflag=num ? num : 1+((1+(rand()%(dlevel+2)))*(1+(rand()%30)));
	fgold=gtmp;
}
struct gen *
g_at(int x, int y, struct gen *ptr)
{
	while(ptr) {
		if(ptr->gx==x && ptr->gy==y) return(ptr);
		ptr=ptr->ngen;
	}
	return(0);
}
/*VARARGS1*/
/* Original 1982: panic(str,a1,a2,...) — converted to stdarg */
void panic(char *str, ...)
{
	va_list ap;

	write(1," ERROR:  ",9);
	va_start(ap, str);
	vprintf(str, ap);
	va_end(ap);
#ifdef DEBUG
	if(tfoo) {
		fputs("ERROR: ",tfoo);
		va_start(ap, str);
		vfprintf(tfoo, str, ap);
		va_end(ap);
		fputs("end\n",tfoo);
		fclose(tfoo);
	}
#endif
	sprintf(nul,"%d",getpid()+2+(rand()%12));
	execl("./mklev",args[0],tfile,tspe,args[3],nul,(char *)NULL); /* Modern: execl() list terminator for 64-bit safety */
}
void makemon(void)
{
	register struct monst *mtmp;
	register int tmp;

	mtmp=(struct monst *)malloc(sizeof(struct monst));
	mtmp->nmon=fmon;
	fmon=mtmp;
	mtmp->mstat=SLEEP;
	mtmp->mhp=((tmp=rn2(dlevel/3+1))>7?rn2(8):tmp);
	mtmp->orig_hp=rn2(7);
}
void savelev(void)
{
	FILE *fd;
	register struct monst *mtmp;
	register struct gen *gtmp;
	register struct obj *otmp;
	unsigned short lver=LEVEL_VERSION;
	struct permonst *monbegin=0;
	unsigned tmpmoves=0;

	if((fd=fopen(tfile,"w"))==0) panic("Cannot create %s\n",tfile);
	bwrite(fd,LEVEL_MAGIC,4);
	bwrite(fd,&lver,sizeof(lver));
	bwrite(fd,&monbegin,sizeof(monbegin));
	bwrite(fd,levl,sizeof(levl));
	bwrite(fd,&tmpmoves,sizeof(tmpmoves));
	bwrite(fd,&xupstair,1);
	bwrite(fd,&yupstair,1);
	bwrite(fd,&xdnstair,1);
	bwrite(fd,&ydnstair,1);
	bwrite(fd,nul,sizeof(struct stole));
	for(mtmp=fmon;mtmp;mtmp=mtmp->nmon)
		bwrite(fd,mtmp,sizeof(struct monst));
	bwrite(fd,nul,sizeof(struct monst));
	for(gtmp=fgold;gtmp;gtmp=gtmp->ngen)
		bwrite(fd,gtmp,sizeof(struct gen));
	bwrite(fd,nul,sizeof(struct gen));
	for(gtmp=ftrap;gtmp;gtmp=gtmp->ngen)
		bwrite(fd,gtmp,sizeof(struct gen));
	bwrite(fd,nul,sizeof(struct gen));
	for(otmp=fobj;otmp;otmp=otmp->nobj)
		bwrite(fd,otmp,sizeof(struct obj));
	bwrite(fd,nul,sizeof(struct obj));
	fclose(fd);
	exit(0);
}
int maker(char lowx, char hix, char lowy, char hiy)
{
	register int tmpx,tmpy;
	register struct rm *ltmp,*lmax;

	if(hix>75) hix=75;
	if(hiy>18) hiy=18;
#ifdef DEBUG
	if(tfoo) fprintf(tfoo,"%d %d - %d %d ",lowx,hix,lowy,hiy);
#endif
	for(tmpx=lowx-2;tmpx<hix+3;tmpx++)
		for(tmpy=lowy-2;tmpy<hiy+3;tmpy++)
			if(levl[tmpx][tmpy].typ) {
#ifdef DEBUG
			out("@%d,%d",tmpx,tmpy);
			out("X\n");
#endif
			return(0);
		}
	if(dlevel<rn2(14)) {
#ifdef DEBUG
		out("L\n");
#endif
		for(tmpx=lowx-1;tmpx<hix+2;tmpx++)
			for(ltmp= &levl[tmpx][lowy-1],lmax= &levl[tmpx][hiy+2];
 ltmp!=lmax;ltmp++) ltmp->lit=1;
	}
#ifdef DEBUG
	else out("D\n");
#endif
	croom->lx=lowx;
	croom->hx=hix;
	croom->ly=lowy;
	croom++->hy=hiy;
	tmpx=lowx-1;
	ltmp= &levl[tmpx][lowy-1];
	lmax= &levl[tmpx][hiy+1];
	ltmp->scrsym='-';
	ltmp->typ=WALL;
	while(++ltmp!=lmax) {
		ltmp->scrsym='|';
		ltmp->typ=WALL;
	}
	ltmp->scrsym='-';
	ltmp->typ=WALL;
	while(++tmpx<=hix) {
		ltmp= &levl[tmpx][lowy-1];
		lmax= &levl[tmpx][hiy+1];
		ltmp->scrsym='-';
		ltmp->typ=WALL;
		while(++ltmp!=lmax) {
			ltmp->scrsym='.';
			ltmp->typ=ROOM;
		}
		ltmp->scrsym='-';
		ltmp->typ=WALL;
	}
	ltmp= &levl[tmpx][lowy-1];
	lmax= &levl[tmpx][hiy+1];
	ltmp->scrsym='-';
	ltmp->typ=WALL;
	while(++ltmp!=lmax) {
		ltmp->scrsym='|';
		ltmp->typ=WALL;
	}
	ltmp->scrsym='-';
	ltmp->typ=WALL;
	++nroom;
	return(1);
}
void makecor(int nx, int ny)
{
	register struct rm *crm;
	register int dix,diy;

#ifdef DEBUG
	out("corr %d %d ",nx,ny);
#endif
	if(nxcor && !rn2(35)) {
		newloc();
#ifdef DEBUG
		out("dead end\n");
#endif
		return;
	}
	dix=abs(nx-tx);
	diy=abs(ny-ty);
	if(nx==79 || nx==0 || ny==0 || ny==21) {
#ifdef DEBUG
		out("edge!\n");
#endif
		if(nxcor) {
			newloc();
			return;
		} else {
#ifdef DEBUG
			out("try again.\n\n");
			fclose(tfoo);
#endif
			sprintf(nul,"%d",getpid()+2+(rand()%12));
			execl("./mklev",args[0],tfile,tspe,args[3],nul,(char *)NULL); /* Modern: execl() list terminator for 64-bit safety */
		}
	}
	if(dy && dix>diy) {
		dy=0;
		dx= nx>tx?-1:1;
#ifdef DEBUG
		out("dx=%d ",dx);
#endif
	} else if(dx && diy>dix) {
		dx=0;
		dy=ny>ty?-1:1;
#ifdef DEBUG
		out("dy=%d ",dy);
#endif
	}
	crm= &levl[nx][ny];
	if(!(crm->typ)) {
		crm->typ=CORR;
		crm->scrsym='#';
		x=nx;
		y=ny;
#ifdef DEBUG
		out("ok.\n");
#endif
		return;
	}
	if(crm->typ==CORR) {
		x=nx;
		y=ny;
#ifdef DEBUG
		out("overlay\n");
#endif
		return;
	}
	if(nx==tx && ny==ty) {
		dodoor(nx,ny);
		newloc();
#ifdef DEBUG
		out("mkcor done\n");
#endif
		return;
	}
	if(x+dx!=nx || y+dy!=ny) {
#ifdef DEBUG
		out("stop.\n");
#endif
		return;
	}
	if(dx) {
		if(ty<ny) dy= -1;
		else dy=levl[nx+dx][ny-1].typ==ROOM?1:-1;
		dx=0;
	} else {
		if(tx<nx) dx= -1;
		else dx=levl[nx-1][ny+dy].typ==ROOM?1:-1;
		dy=0;
	}
#ifdef DEBUG
	out("--> %d %d\n",dx,dy);
#endif
}
#ifdef DEBUG
/* Original 1982: out(str,a1,a2,...) — converted to stdarg */
void out(char *str, ...)
{
	va_list ap;

	if(tfoo) {
		va_start(ap, str);
		vfprintf(tfoo, str, ap);
		va_end(ap);
	}
}
#endif
int mkmim(int num)
{
	register struct monst *mtmp;

	mtmp=(struct monst *)malloc(sizeof(struct monst)); /* Modern: explicit malloc cast to match restored K&R intent */
	mtmp->nmon=fmon;
	fmon=mtmp;
	if(!num) {
		mtmp->mhp=2;
		mtmp->orig_hp=3;
		if(xdnstair) {
			mtmp->mx=somex();
			mtmp->my=somey();
		} else {
			mtmp->mx=MAZX;
			mtmp->my=MAZY;
		}
		mtmp->invis=1;
	} else if((!xdnstair) || rn2(2)) {
		mtmp->mhp=5;
		mtmp->orig_hp=2;
		if(xdnstair)
			levl[mtmp->mx=somex()][mtmp->my=somey()].scrsym='$';
		else levl[mtmp->mx=MAZX][mtmp->my=MAZY].scrsym='$';
		return(1);
	} else {
		if(rn2(2)){
			if(rn2(2)) mtmp->mx=croom->hx+1;
			else mtmp->mx=croom->lx-1;
			mtmp->my=somey();
		} else {
			if(rn2(2)) mtmp->my=croom->hy+1;
			else mtmp->my=croom->ly-1;
			mtmp->mx=somex();
		}
		levl[mtmp->mx][mtmp->my].scrsym='+';
		mtmp->mhp=5;
		mtmp->orig_hp=2;
	}
	return(0);
}
/* Modern: level file header for versioned save format */
#define LEVEL_MAGIC "FLEV"
#define LEVEL_VERSION 1
