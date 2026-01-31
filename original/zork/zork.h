#include <stdio.h>

#define rspeak(x)	rspsb2(x,0,0)	/* print message x */
#define rspsub(x,y)	rspsb2(x,y,0)	/* print message x with arg y */

extern int prsa,prsi,prso,prswon,prscon; /* parser output */

extern oflag,oact,oslot,oprep,oname;	/* parser state */

extern lastit,act,obj1,obj2,prep1,prep2;	/* more parse stuff */

extern syn[11],sdir,sind,sstd,sflip,sdriv,svmask;  /* ditto */

extern vabit,vrbit,vebit,vfbit,vpmast;  /* and yet more */

extern int winner,here,telflg;	/* game state */
extern moves,deaths,rwscr,mxscr,mxload,ltshft,bolc,mungrm,hs,egscore,egmxsc;

extern fromdir,scolrm,scolac,scoldr[8],scolwl[12]; /* screen of light */

extern cpdr[8],cpwl[8],cpvec[64];	/* puzzle room */

extern mlnt,rtext(1050); /* message index */

extern char *inbuf;
extern char inlnt,inline[78];	/* misc vars */
extern mbase,strbit,vmadj,vmin,vedit,pltim,shour,smin,ssec;
extern batdrp[9];
extern int inpch,outch,dbch;
extern debflg,prsflg,gdtflg;
extern hfactr;

/* rooms */
extern rlnt,rdesc2,rdesc1[200],rexit[200],ractio[200],rval[200],rflag[200];
