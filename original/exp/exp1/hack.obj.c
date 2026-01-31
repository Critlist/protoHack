#include "hack.h"
		/* procedures having to do with objects */

doname(obj,buf)
register struct obj *obj;
register char *buf;
{
	switch(obj->olet) {
	case '"':
		strcpy(buf,"The amulet of Frobozz.");
		break;
	case '%':
 if(gquan(obj)>1) sprintf(buf,"%d %ss.",gquan(obj),foodnam[obj->otyp]);
		else sprintf(buf,"a %s.",foodnam[obj->otyp]);
		break;
	case ')':
		if(gknown(obj)) {
			if(gquan(obj)>1) sprintf(buf,"%d %c%d %ss",gquan(obj),
 gminus(obj)?'-':'+',gospe(obj),wepnam[obj->otyp]);
			else sprintf(buf,"a %c%d %s",gminus(obj)?'-':'+',
 gospe(obj),wepnam[obj->otyp]);
		} else {
			if((gquan(obj))>1) sprintf(buf,"%d %ss",gquan(obj),
 wepnam[obj->otyp]);
			else setan(wepnam[obj->otyp],buf);
		}
		while(*++buf) ;
		*buf++='.';
		*buf=0;
		break;
	case '[':
		if(gknown(obj)) sprintf(buf,"a suit of %c%d %s armor",
 gminus(obj)?'-':'+',gospe(obj),armnam[obj->otyp-2]);
		else sprintf(buf,"a suit of %s armor",armnam[obj->otyp-2]);
		if(obj==uarm) strcat(buf,"  (being worn)");
		while(*++buf) ;
		*buf++='.';
		*buf=0;
		break;
	case '!':
		if(oiden[obj->otyp]&POTN || potcall[obj->otyp]) {
			if(gquan(obj)>1) sprintf(buf,"%d potions",gquan(obj));
			else sprintf(buf,"a potion");
			while(*buf) buf++;
			if(!potcall[obj->otyp])
				sprintf(buf,OF,pottyp[obj->otyp]);
			else sprintf(buf,CALL,potcall[obj->otyp]);
		} else {
			if(gquan(obj)>1) sprintf(buf,"%d %s potions.",
 gquan(obj),potcol[obj->otyp]);
			else setan(potcol[obj->otyp],buf),
				strcat(buf," potion.");
		}
		break;
	case '?':
		if(gquan(obj)>1) sprintf(buf,"%d scrolls",gquan(obj));
		else strcpy(buf,"a scroll");
		while(*buf) buf++;
		if(oiden[obj->otyp]&SCRN) sprintf(buf,OF,scrtyp[obj->otyp]);
		else if(scrcall[obj->otyp])
			sprintf(buf,CALL,scrcall[obj->otyp]);
		else sprintf(buf," labeled %s.",scrnam[obj->otyp]);
		break;
	case '/':
		if(oiden[obj->otyp]&WANN)
			sprintf(buf,"a wand of %s.",wantyp[obj->otyp]);
		else if(wandcall[obj->otyp])
			sprintf(buf,"a wand called %s.",wandcall[obj->otyp]);
		else setan(wannam[obj->otyp],buf),
			strcat(buf," wand.");
		if(gknown(obj)) {
			while(*buf) buf++;
			sprintf(buf,"  (%d).",gospe(obj));
		}
		break;
	case '=':
		if(oiden[obj->otyp]&RINN) {
			if(gknown(obj)) sprintf(buf,"a %c%d ring of %s",
 gminus(obj)?'-':'+',gospe(obj),ringtyp[obj->otyp]);
			else sprintf(buf,"a ring of %s",ringtyp[obj->otyp]);
		} else if(ringcall[obj->otyp])
			sprintf(buf,"a ring called %s",ringcall[obj->otyp]);
		else setan(rinnam[obj->otyp],buf),
			strcat(buf," ring");
		if(obj==uright) strcat(buf,"  (on right hand)");
		if(obj==uleft) strcat(buf,"  (on left hand)");
		while(*++buf) ;
		*buf++='.';
		*buf=0;
		break;
	case '*':
		if(gquan(obj)>1) sprintf(buf,"%d %s gems.",gquan(obj),
 potcol[obj->otyp]);
		else {
			setan(potcol[obj->otyp],buf);
			strcat(buf," gem.");
		}
		break;
	default:
		strcpy(buf,"bug!");
	}
	if(obj==uwep) strcat(buf,"  (weapon in hand)");
}
useup(obj)
register struct obj *obj;
{
	register struct obj *otmp;

	if(gquan(obj)>1) {
		obj->quanmin--;
		return;
	}
	if(obj==invent) invent=invent->nobj;
	else {
		for(otmp=invent;otmp->nobj!=obj;otmp=otmp->nobj) ;
		otmp->nobj=obj->nobj;
	}
	mfree(obj);
	if(obj==uwep) uwep=0;
}
struct obj *
getobj(let,word)
register char *let,*word;
{
	register struct obj *otmp;
	register char ilet;
	int foo;

	foo=0;
	for(otmp=invent;otmp;otmp=otmp->nobj)
		if(index(let,otmp->olet)) foo++;
	if(!invent || (let && *let!=')' && !foo)) {
		pline("You don't have anything to %s.",word);
		return(0);
	}
	for(;;) {
		pline("%s what (* for list)?",word);
		flags.topl=1;
#ifndef SMALL
		fflush(stdout);
#endif
		ilet=getchar();
		if(ilet=='\033') return(0);
		if(ilet=='*') {
			if(!let || foo>1) doinv(let);
			else {
				for(otmp=invent;otmp;otmp=otmp->nobj) {
					if(index(let,otmp->olet)) prinv(otmp);
				}
			}
		} else {
			if(ilet>='A' && ilet<='Z') ilet=26+ilet-'A';
			else ilet-='a';
			for(otmp=invent;otmp && ilet;ilet--,otmp=otmp->nobj) ;
			if(!otmp) {
				pline(DONTH);
				continue;
			}
			break;
		}
	}
	if(let && *let!=')' && !index(let,otmp->olet)) {
		pline("You can't %s that.",word);
		return(0);
	} else return(otmp);
}
prinv(obj)
register struct obj *obj;
{
	register struct obj *otmp;
	register char ilet='a';

	for(otmp=invent;otmp!=obj;otmp=otmp->nobj)
		if(++ilet>'z') ilet='A';
	sprintf(buf,"%c - ",ilet);
	doname(otmp,&buf[4]);
	pline(buf);
}
weight(obj)
register struct obj *obj;
{
	switch(obj->olet) {
	case '"': return(2);
	case '[': return(8);
	case '%': if(obj->otyp) return(gquan(obj));
	case '?': return(3*gquan(obj));
	case '!': return(2*gquan(obj));
	case ')': if(obj->otyp==8) return(4);
		if(obj->otyp<4) return((gquan(obj))/2);
		return(3);
	case '=': return(1);
	case '*': return(gquan(obj));
	case '/': return(3);
	}
	return(100);
}
gobj(obj)
register struct obj *obj;
{
	register struct obj *otmp;

	if(obj==fobj) fobj=fobj->nobj;
	else {
		for(otmp=fobj;otmp->nobj!=obj;otmp=otmp->nobj) ;
		otmp->nobj=obj->nobj;
	}
	if(!invent) {
		invent=obj;
		obj->nobj=0;
	} else for(otmp=invent;otmp;otmp=otmp->nobj) {
		if(otmp->otyp==obj->otyp && otmp->olet==obj->olet) {
			if(obj->otyp<4 && obj->olet==')' &&
 obj->spestuff==otmp->spestuff && gminus(obj)==gminus(otmp)) {
				otmp->quanmin+=gquan(obj);
				mfree(obj);
				obj=otmp;
				break;
			} else if(index("%?!*",otmp->olet)) {
				otmp->quanmin+=gquan(obj);
				mfree(obj);
				obj=otmp;
				break;
			}
		}
		if(!otmp->nobj) {
			otmp->nobj=obj;
			obj->nobj=0;
			break;
		}
	}
/* note that this loop never exits normally (We HOPE...) */
	prinv(obj);
	if(u.uinvis) newsym(u.uloc);
}
doinv(str)
register char *str;
{
	register struct obj *otmp=invent;
	register char ilet='a';

#ifndef SMALL
	if(!flags.step) cls();
#else
	cls();
#endif
	while(otmp) {
		if(!str || index(str,otmp->olet)) {
			sprintf(buf,"%c -  ",ilet);
			doname(otmp,&buf[5]);
#ifndef SMALL
			if(flags.step) pline(buf);
			else {
#endif
			fputs(buf,stdout);
			fputs("\r\n",stdout);
#ifndef SMALL
			}
#endif
		}
		otmp=otmp->nobj;
		if(++ilet>'z') ilet='A';
	}
#ifndef SMALL
	if(flags.step) return;
#endif
	getret();
	docrt();
}
