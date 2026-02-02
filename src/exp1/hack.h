/* Original 1982: char *index(),*getlogin(),*getenv(); */
/* Now provided by compat.h includes and macros below */
/* Original 1982: int done1(),done2(); */
/* Now provided by prototypes below */

#include <stdio.h>

#define NONUM

#define WALL 1	/* Level location types */
#define SDOOR 2
#define DOOR 3
#define CORR 4
#define ROOM 5

#define WEPNUM 14 /* numbers of different objects */
#define POTNUM 15
#define SCRNUM 15
#define TRAPNUM 7
#define WANDNUM 16
#define RINGNUM 17
#define ARMNUM 6 /* ac 3-8 */
#define GEMNUM 15

#define cl_end()		fputs(CE,stdout)
#define mfree(ptr)		free((char *)ptr)
#define bwrite(fp,loc,num)	fwrite((char *)loc,1,num,fp)
#define mread(fp,loc,num)	fread((char *)loc,1,num,fp)
#ifdef VTONL
#define cm(x,y)			printf("\033[%d;%dH",cury=y,curx=x)
#endif

#define MNORM 0	/* used in both */

/* these go in mstat */
#define FLEE 1	/* fleeing monster */
#define SLEEP 2 /* sleeping monster... */
#define MFROZ 3 /* frozen */

/* these are in mspeed */
#define MCONF 1 /* confused */
#define MSLOW 2 /* slow monster */
#define MFAST 3 /* speeded monster */

#define BEAR 0 /* different types of traps */
#define ARROW 1
#define DART 2
#define TDOOR 3
#define TELE 4
#define PIT 5
#define SLPTRP 6
#define SEEN 32 /* trap which has been seen */

#define POTN 1 /* bit in identified flags */
#define SCRN 2
#define WANN 4
#define RINN 8

struct lev {
	char lobyt;	/* format = (low to high) 7 bits of scrsym 1 of
	new*/
	char hibyt;	/* format = (low to high) 3 bits of type,seen,lit,
	cansee*/
};
#define setscr(ptr,let) (ptr)->lobyt=let
#define getscr(ptr) ((ptr)->lobyt&0177)

#define setnew(ptr) (ptr)->lobyt|=0200
#define resnew(ptr) (ptr)->lobyt&=~0200
#define getnew(ptr) ((ptr)->lobyt&0200)

#define settyp(ptr,typ) ((ptr)->hibyt&=0270, (ptr)->hibyt|=typ)
#define gettyp(ptr) ((ptr)->hibyt&07)

#define sseen(ptr) (ptr)->hibyt|=010
#define rseen(ptr) (ptr)->hibyt&=~010
#define gseen(ptr) ((ptr)->hibyt&010)

#define setlit(ptr) (ptr)->hibyt|=020
#define reslit(ptr) (ptr)->hibyt&=~020
#define getlit(ptr) ((ptr)->hibyt&020)

#define setcan(ptr) (ptr)->hibyt|=040
#define rescan(ptr) (ptr)->hibyt&=~040
#define getcan(ptr) ((ptr)->hibyt&040)
extern struct lev levl[80][22];

struct permonst {
	char *mname,mlet,mhd,mmove,ac,damn,damd;
};
extern struct permonst mon[8][7];

struct monst {
	struct monst *nmon;
	struct lev *mloc;
	char lob1;/* format is (low to hi) 2 bits speed */
	/* sinv 04,invis 010,cham 020,can 040 */
	char hib1;/* format is low to hi 2 bits stat */
	struct permonst *data;
	char mhp,orig_hp;
};
#define sspeed(ptr,x) (ptr)->lob1&=~03,(ptr)->lob1|=x
#define gspeed(ptr) (ptr->lob1&03)

#define sstat(ptr,x) ptr->hib1&=~03,ptr->hib1|=x
#define gstat(ptr) (ptr->hib1&03)

#define ssinv(ptr) ptr->lob1|=04
#define rsinv(ptr) ptr->lob1&=~04
#define gsinv(ptr) (ptr->lob1&04)

#define sinv(ptr) ptr->lob1|=010
#define rinv(ptr) ptr->lob1&=~010
#define ginv(ptr) (ptr->lob1&010)

#define scham(ptr) ptr->lob1|=020
#define rcham(ptr) ptr->lob1&=~020
#define gcham(ptr) (ptr->lob1&020)

#define scan(ptr) ptr->lob1|=040
#define rcan(ptr) ptr->lob1&=~040
#define gcan(ptr) (ptr->lob1&040)
extern struct monst *fmon;
struct monst *bhit();

struct stole {
	struct stole *nstole;
	struct monst *smon;
	struct obj *sobj;
	unsigned sgold;
};
extern struct stole *fstole;
struct gen {
	struct gen *ngen;
	struct lev *gloc;
	unsigned gflag;
};
extern struct gen *fgold,*ftrap;
struct gen *g_at();

struct obj {
	struct obj *nobj;
	struct lev *oloc;
	char olet;
	char quanmin; /* quan and minus */
	char spestuff; /* spe known and cursed */
	char otyp;
};
#define squan(obj,num) obj->quanmin&=0200,obj->quanmin|=num
#define gquan(obj) (obj->quanmin&0177)

#define sminus(obj) obj->quanmin|=0200
#define rminus(obj) obj->quanmin&=0177
#define gminus(obj) (obj->quanmin&0200)

#define sospe(obj,num) obj->spestuff&=~077,obj->spestuff|=num
#define gospe(obj) (obj->spestuff&077)

#define sknown(obj) obj->spestuff|=0100
#define gknown(obj) (obj->spestuff&0100)

#define scursed(obj) obj->spestuff|=0200
#define rcursed(obj) obj->spestuff&=0177
#define gcursed(obj) (obj->spestuff&0200)

extern struct obj *fobj,*invent,*uwep,*uarm,*uleft,*uright;
struct obj *getobj();

struct flag {
	int botl;	/* bottom line must be redrawn (all or part)*/
	char topl;	/* top line has been printed */
	char maze;	/* the level of the maze */
	char move;	/* the previous move was real */
	char mv;	/* You are doing a multiple move */
	char mdone;	/* you have actually moved */
	char dscr;	/* some area of the screen */
#ifndef SMALL
	char one;	/* options */
	char step;	/* do rogue one line style inventory */
#endif
};
extern struct flag flags;

struct you {
	struct lev *uloc;
	char ufast;
	char uconfused;
	char uinvis;
	char ulevel;
	char utrap;
	char upit;
	char umconf;
	char ufireres;
	char ucoldres;
	char uswallow;
	char uswldtim;
	char ucham;
	char uhs;
	char utel;
	char upres;
	char ustelth;
	char uagmon;
	char ufeed;
	char usearch;
	char ucinvis;
	char uregen;
	char ufloat;
	char ustr,ustrmax;
	char udaminc;
	char uhp,uhpmax,uac;
	unsigned ugold,uexp,urexp,uhunger,ublind;
	struct monst *ustuck;
};

extern struct you u;

#include "compat.h"

extern char *armnam[],*foodnam[],*wepnam[],*pottyp[],*scrtyp[],*traps[];
extern char *wantyp[],*ringtyp[],*potcol[],*scrnam[],*wannam[],*rinnam[];
extern char wdam[],oiden[];
extern char mlarge[],wsdam[],wldam[];
extern char *potcall[], *scrcall[], *wandcall[], *ringcall[];

extern char curx,cury,savx;	/* cursor location on screen */

extern struct lev *upstair,*dnstair;

extern char seehx,seelx,seehy,seely; /* where to see*/
extern char scrlx,scrhx,scrly,scrhy;	/* where to update */
extern char save_cm;

extern char dlevel; /* dungeon level */

extern char dx,dy,tx,ty; /* used by move and makemon (in .mklv) */
extern int dir;

extern unsigned moves;

extern int multi;

void done1(int signum);
void done2(int signum);

extern char buf[],nul[];

extern char lock[];
extern struct lev *ouloc;

#define ALL 1	/* for flags.botl */
#define GOLD 2
#define HP 4
#define HPM 8
#define STR 16
#define AC 32
#define ULV 64
#define UEX 128
#define DHS 256

#ifdef VTONL
extern char CE[];
#else
extern char *CE;
#endif

extern char vowels[],IT[],It[],dirs[]; /* hack.c */

/* hack.cmd.c */
extern char CURSED[],EMPTY[],NOTHIN[],WAND[],RUST[],IDENT[];

/* hack.cmdsub.c */
extern char *fl[],NOCOLD[],WCLEV[],MORE[],WEARI[];

/* hack.fight.c */
extern char HIT[],NOBLUE[];

/* hack.mon.c */
extern char CRUSH[],hits,mregen[],*hnu[],dirs[];

/* hack.move.c */
int prl(),nosee();
extern char WARROW[],DONTF[],WDART[];

/* hack.obj.c */
extern char OF[],CALL[],DONTH[];

/* hack.screen.c */
#ifdef VTONL
extern char HO[],CL[],CE[],ND[],BC[],UP[];
#else
extern char *HO,*CL,*CE,*ND,*BC,*UP,*CM;
#endif
extern char HUNG[],WEAK[],FAINT[],BLANK[];

/* hack.files */
extern char READ[],WRITE[],ESCAPED[];


extern char QUIT[];

/* large */
#ifndef SMALL
extern char SAVEF[];
extern char *uname,*killer;
extern struct permonst vbat;
#endif
