/* Original 1982: char *index(),*getlogin(),*getenv(); */
/* Original 1982: int done1(),done2(); */
/* Now provided by compat.h includes and prototypes below */

#include <stdio.h>  /* Modern: needed for FILE * in prototypes below */

#define LOCKNUM 2
#define MAGIC

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

#ifdef VTONL
#define startup(x) ;
#endif

#define mfree(ptr) free(ptr)

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
#define SCRN 2 /* in oiden */
#define WANN 4
#define RINN 8

/* Modern: use 16-bit bitfields to preserve 1982 struct rm size (3520-byte levels) */
struct rm {
	unsigned short scrsym:7;
	unsigned short typ:3;
	unsigned short new:1;
	unsigned short seen:1;
	unsigned short lit:1;
	unsigned short cansee:1;
};
extern struct rm levl[80][22];

struct permonst {
	char *mname,mlet,mhd,mmove,ac,damn,damd;
};
extern struct permonst mon[8][7];

struct monst {
	struct monst *nmon;
	char mx,my;
	int sinv:1;	/* special invisible */
	int invis:1;	/* invisible */
	int cham:1;	/* shape-changer */
	int mspeed:2;
	int mstat:2;
	int mcan:1;	/* has been canceled */
	int mstuck:1;	/* you are stuck to this */
	struct permonst *data;
	char mhp,orig_hp;
};
extern struct monst *fmon;
struct monst *bhit(int, int, int);

struct stole {
	struct stole *nstole;
	struct monst *smon;
	struct obj *sobj;
	unsigned sgold;
};
extern struct stole *fstole;
struct gen {
	struct gen *ngen;
	char gx,gy;
	unsigned gflag;
};
extern struct gen *fgold,*ftrap;
struct gen *g_at(int, int, struct gen *);

struct obj {
	struct obj *nobj;
	char ox,oy,olet;
	int spe:6;
	int quan:7;
	int minus:1;
	int known:1;
	int cursed:1;
	char otyp;
};
extern struct obj *fobj,*invent,*uwep,*uarm,*uleft,*uright;
struct obj *getobj(char *, char *);

struct flag {
#ifdef MAGIC
	char magic;	/* in magic mode */
	char wmag;	/* was in magic mode */
#endif
	char topl;	/* top line has been printed */
	int botl;	/* bottom line must be redrawn */
	char maze;	/* the level of the maze */
	char move;	/* the previous move was real */
	char mv;	/* You are doing a multiple move */
	char mdone;	/* you have actually moved */
	char dscr;	/* some area of the screen */
#ifndef SMALL
	char one;	/* option to move 1 space after doors is on */
	char step;	/* option to do inventories 1 line at a time is on */
	char flush;	/* flush input every monster move (Battle round) */
#endif
};
extern struct flag flags;

struct you {
	char ux;
	char uy;
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

extern char *armnam[],*foodnam[],*wepnam[],*pottyp[],*scrtyp[],*traps[];
extern char *wantyp[],*ringtyp[],*potcol[],*scrnam[],*wannam[],*rinnam[];
extern char oiden[];
extern char mlarge[],wsdam[],wldam[];
extern char *potcall[], *scrcall[], *wandcall[], *ringcall[];

extern char curx,cury,savx;	/* cursor location on screen */

extern char xdnstair, ydnstair, xupstair, yupstair; /* stairs up and down. */

extern char seehx,seelx,seehy,seely; /* where to see*/
extern char scrlx,scrhx,scrly,scrhy;	/* where to update */
extern char save_cm;

extern char dlevel; /* dungeon level */

extern char dx,dy,tx,ty; /* used by move and makemon (in .mklv) */

extern unsigned moves;

extern int multi;

extern char buf[],nul[];

extern char lock[];
extern char oldux,olduy;

#ifndef SMALL
extern char *uname,*killer;	/* your name for the record */
#endif

#define ALL 1	/* for flags.botl */
#define GOLD 2
#define HP 4
#define HPM 8
#define STR 16
#define AC 32
#define ULV 64
#define UEX 128
#define DHS 256

/* hack.c */
void unsee(void);
void seeoff(int);
int movecm(char);
void domove(void);
void delmon(struct monst *);
int hack_pow(int);  /* Original 1982: pow() — renamed for math.h conflict */
void tele(void);
void doname(struct obj *, char *);
int parse(void);
#ifdef MAGIC
void domagic(int);
void tellall(void);
#endif
void done1(int);
void done(char *);
void setsee(void);
void nomul(int);
int abon(void);
int sgn(int);
void losexp(void);
void useup(struct obj *);
void cbout(void);
void cbin(void);
void cl_end(void);
void setan(char *, char *);
void prinv(struct obj *);
int weight(struct obj *);
void getlin(char *);
void done2(int);
void gobj(struct obj *);
void ndaminc(void);
int amon(struct monst *, struct obj *, int);
int attmon(struct monst *, struct obj *);

/* hack.main.c */
void shufl(char *[], int);
char *alloc(int);
void losestr(int);
void getret(void);
void glo(int);
#ifdef LOCKNUM
void leave(int);
void lockcheck(void);
void getlock(void);
int modern_lock_game(void);
void modern_unlock_game(void);
int modern_lock_record(void);
void modern_unlock_record(void);
void modern_cleanup_locks(void);
int check(int);
#endif
#ifdef MAGIC
void fooexit(int);
#endif

/* hack.lev.c — savelev/mkobj have different signatures in mklev.c */
#ifndef MKLEV_BINARY
void savelev(FILE *fp);
int getlev(FILE *fp);
void mklev(void);
void mkobj(int let);
#endif

/* hack.mon.c — makemon has different signature in mklev.c */
#ifndef MKLEV_BINARY
void movemon(void);
void justswld(struct monst *);
void youswld(struct monst *, int, int);
void dochug(struct monst *);
void mhit(char *);
void inrange(struct monst *);
int m_move(struct monst *);
int r_free(int, int);
void mnexto(struct monst *);
int test(int, int);
void poisoned(char *);
void steal(struct monst *);
void killed(struct monst *);
void kludge(char *, char *);
void k1(char *, char *);
void rescham(void);
void newcham(struct monst *, struct permonst *);
int makemon(struct permonst *);
int hitu(int, int, char *);
void rloc(struct monst *);
#endif

/* hack.do.c */
void rhack(char);
void were(struct obj *);
void drink1(struct obj *);
void read1(struct obj *);
#ifndef SMALL
void set1(char *);
#endif

/* hack.do1.c */
void ringoff(struct obj *);
void hit(char *, struct monst *);
void miss(char *, struct monst *);
#ifndef SMALL
int findit(void);
void dosave(void);
void dorecover(FILE *fp);
#endif
void buzz(int, int, int, int, int);
void zhit(struct monst *, int);
void dosearch(void);
void loseone(struct obj *, int, int);
int getdir(void);
void chwepon(char *);
void litroom(void);
void pluslvl(void);
void nothin(struct obj *);
void lesshungry(int);
void plusone(struct obj *);
void minusone(struct obj *);
void doinv(char *);
void dodown(void);
void doup(void);
void docall(struct obj *);
void more(void);
void dodr1(struct obj *);
