#include "hack.h"
	/* procedures having to do with seeing things */
litroom()
{
	register struct lev *room;
	register struct lev *min,*maxy,*maxxy;

	if(gettyp((room= u.uloc))==CORR) {
		pline("The corridor glows briefly.");
		return;
	} else pline("The room is lit.");
	if(getlit(room)) return;
/* these set min to the upper left-hand corner of the room */
	for(min=room;gettyp(north(min)) && gettyp(north(min))!=CORR;min--) ;
	for(;gettyp(west(min)) && gettyp(west(min))!=CORR;min-=22) ;

	for(maxy=min;gettyp(maxy) && gettyp(maxy)!=CORR;maxy++) ;
/* note that maxy points to one past the maximium ypos */

	for(maxxy=maxy-1;gettyp(maxxy)&&gettyp(maxxy)!=CORR;maxxy+=22) ;
	maxxy++;
/* ditto for maxxy */
	for(;maxy!=maxxy;maxy+=22,min+=22) {
		for(room=min;room<maxy;room++) {
			setlit(room);
			if(!getcan(room)) prl(room);
		}
	}
}
unsee(loc)
register struct lev *loc;
{
	register struct lev *min,*maxy,*maxxy;
	register struct monst *mtmp;

	newsym(loc);
	if(u.ublind) return;
	if(getlit(loc)) {
		for(min=loc;getlit(north(min));min--)
			;
		for(;getlit(west(min));min-=22)
			;

		for(maxy=min;getlit(maxy);maxy++)
			;
/* note that maxy points to one past the maximium ypos */

		for(maxxy=maxy-1;getlit(maxxy);maxxy+=22)
			;
		maxxy++;
/* ditto for maxxy */
	} else {
		min=loc-23;
		maxy=loc-20;
		maxxy=loc+46;
	}
	for(;maxy!=maxxy;maxy+=22,min+=22) {
		for(loc=min;loc<maxy;loc++) {
			rescan(loc);
			if((mtmp=g_at(loc,fmon)) && (gstat(mtmp))<SLEEP
 && mtmp->data->mlet==getscr(loc)) newsym(loc);
			else if((!getlit(loc)) && getscr(loc)=='.') {
				setscr(loc,' ');
				on(loc);
			}
		}
	}
}
/* 1 to redo @, 0 to leave them */
/* 1 means misc movement, 0 means blindness (Usually) */
seeoff(mode)
{
	register struct lev *room;
	register struct lev *min,*maxy,*maxxy;

	if(mode) newsym(u.uloc);
	if(getlit(u.uloc)) {
		for(min=u.uloc;getlit(min-1);min--) ;
		for(;getlit(min-22);min-=22) ;

		for(maxy=min;getlit(maxy);maxy++) ;

		for(maxxy=maxy-1;getlit(maxxy);maxxy+=22) ;
		maxxy++;

	} else {
		min=u.uloc-23;
		maxy=u.uloc-20;
		maxxy=u.uloc+46;
	}
	for(;maxy!=maxxy;maxy+=22,min+=22) {
		for(room=min;room<maxy;room++) {
			rescan(room);
			if(mode && getscr(room)=='.') rseen(room);
			else if(getscr(room)=='.')
				rseen(room);
		}
	}
}
setsee()
{
	register struct lev *room;
	register struct lev *min,*maxy,*maxxy;
	struct monst *mtmp;

	room=u.uloc;
	if(u.ublind) {
		setcan(room);
		if(u.uinvis) newsym(room);
		else atl(room,'@');
		return;
	} else if(getlit(room)) {
/* these set min to the upper left-hand corner of the room */
		for(min=room;getlit(min-1);min--) ;
		for(;getlit(min-22);min-=22) ;

		for(maxy=min;getlit(maxy);maxy++) ;
/* note that maxy points to one past the maximium ypos */

		for(maxxy=maxy-1;getlit(maxxy);maxxy+=22) ;
		maxxy++;
/* ditto for maxxy */

	} else {
		min=room-23;
		maxy=room-20;
		maxxy=room+46;
	}
	for(;maxy!=maxxy;maxy+=22,min+=22) {
		for(room=min;room<maxy;room++) {
			if(!gettyp(room)) continue;
			if((mtmp=g_at(room,fmon)) && !(ginv(mtmp) ||
 u.ucinvis)) {
				atl(room,mtmp->data->mlet);
			}
			else if(!u.uinvis && room==u.uloc) atl(room,'@');
			else if(index("-|",getscr(room)) && 
 gettyp(u.uloc)==CORR) continue;
			else if(!gseen(room)) on(room);
			setcan(room);
		}
	}
}
