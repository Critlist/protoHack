#include "../compat.h"
#include "hack.h"
#include <stdint.h>

extern char READ[],WRITE[],SAVEF[],nul[];
extern char oiden[20];
int dosave0(int);

/**
 * MODERN ADDITION (2026): Enhanced hangup handler with SIGTERM support
 *
 * WHY: Original code only handled terminal hangup via SIGHUP. Modern terminal
 *      closures often send SIGTERM, bypassing save logic.
 *
 * HOW: Unified handler that attempts a safe save on SIGHUP/SIGTERM.
 *
 * PRESERVES: Original save-on-hangup behavior.
 * ADDS: Modern signal compatibility for common terminal emulators.
 */
void modern_save_handler(int signum)
{
	(void)signum;
	(void)dosave0(1);
	exit(1);
}

void hangup(int signum)
{
	modern_save_handler(signum);
}

/**
 * MODERN ADDITION (2026): Versioned, pointer-safe save file format
 *
 * WHY: The original 1982 save format writes raw pointers and 16-bit fields,
 *      which is fragile on modern 64-bit systems and across builds.
 *
 * HOW: Serialize game state with fixed-width integers, explicit string lengths,
 *      and a versioned header. Keep level data in its native format but
 *      isolate save metadata from raw pointers.
 *
 * PRESERVES: Original game state and behavior when saving/restoring.
 * ADDS: Robust save compatibility and safer restore logic on modern systems.
 */

#define SAVE_MAGIC "FHCK"
#define SAVE_VERSION 1
#define SAVE_ENDIANTAG 0x01020304u

/* Forward declarations */
void panic(char *, ...);

/* I/O helpers */
static void write_exact(FILE *fp, const void *buf, size_t len)
{
	if(fwrite(buf,1,len,fp)!=len) panic("save write failed\n");
}
static int read_exact(FILE *fp, void *buf, size_t len)
{
	return fread(buf,1,len,fp)==len;
}
static void sw_u8(FILE *fp, uint8_t v) { write_exact(fp,&v,1); }
static void sw_u16(FILE *fp, uint16_t v)
{
	uint8_t b[2];
	b[0]=(uint8_t)((v>>8)&0xff);
	b[1]=(uint8_t)(v&0xff);
	write_exact(fp,b,2);
}
static void sw_u32(FILE *fp, uint32_t v)
{
	uint8_t b[4];
	b[0]=(uint8_t)((v>>24)&0xff);
	b[1]=(uint8_t)((v>>16)&0xff);
	b[2]=(uint8_t)((v>>8)&0xff);
	b[3]=(uint8_t)(v&0xff);
	write_exact(fp,b,4);
}
static int sr_u8(FILE *fp, uint8_t *v) { return read_exact(fp,v,1); }
static int sr_u16(FILE *fp, uint16_t *v)
{
	uint8_t b[2];
	if(!read_exact(fp,b,2)) return 0;
	*v=(uint16_t)((b[0]<<8)|b[1]);
	return 1;
}
static int sr_u32(FILE *fp, uint32_t *v)
{
	uint8_t b[4];
	if(!read_exact(fp,b,4)) return 0;
	*v=((uint32_t)b[0]<<24)|((uint32_t)b[1]<<16)|((uint32_t)b[2]<<8)|b[3];
	return 1;
}

static void save_string(FILE *fp, const char *s)
{
	uint16_t len=0;
	if(s) len=(uint16_t)(strlen(s)+1);
	sw_u16(fp,len);
	if(len) write_exact(fp,s,len);
}
static char *load_string(FILE *fp)
{
	uint16_t len=0;
	char *s;

	if(!sr_u16(fp,&len)) return 0;
	if(!len) return 0;
	s=alloc(len);
	if(!read_exact(fp,s,len)) return 0;
	s[len-1]='\0';
	return s;
}

static void save_string_array(FILE *fp, char **arr, int count)
{
	int i;

	sw_u16(fp,(uint16_t)count);
	for(i=0;i<count;i++) save_string(fp,arr[i]);
}
static int load_string_array(FILE *fp, char **arr, int count)
{
	uint16_t n=0;
	int i;

	if(!sr_u16(fp,&n)) return 0;
	if(n!=count) return 0;
	for(i=0;i<count;i++) arr[i]=load_string(fp);
	return 1;
}

static void save_call_array(FILE *fp, char **arr, int count)
{
	int i;

	sw_u16(fp,(uint16_t)count);
	for(i=0;i<count;i++) {
		if(arr[i]) save_string(fp,arr[i]);
		else sw_u16(fp,0);
	}
}
static int load_call_array(FILE *fp, char **arr, int count)
{
	uint16_t n=0;
	int i;

	if(!sr_u16(fp,&n)) return 0;
	if(n!=count) return 0;
	for(i=0;i<count;i++) {
		uint16_t len=0;

		if(!sr_u16(fp,&len)) return 0;
		if(!len) { arr[i]=0; continue; }
		arr[i]=alloc(len);
		if(!read_exact(fp,arr[i],len)) return 0;
		arr[i][len-1]='\0';
	}
	return 1;
}

static void save_flags(FILE *fp)
{
#ifdef MAGIC
	sw_u8(fp,(uint8_t)flags.magic);
	sw_u8(fp,(uint8_t)flags.wmag);
#endif
	sw_u8(fp,(uint8_t)flags.topl);
	sw_u32(fp,(uint32_t)flags.botl);
	sw_u8(fp,(uint8_t)flags.maze);
	sw_u8(fp,(uint8_t)flags.move);
	sw_u8(fp,(uint8_t)flags.mv);
	sw_u8(fp,(uint8_t)flags.mdone);
	sw_u8(fp,(uint8_t)flags.dscr);
#ifndef SMALL
	sw_u8(fp,(uint8_t)flags.one);
	sw_u8(fp,(uint8_t)flags.step);
	sw_u8(fp,(uint8_t)flags.flush);
#endif
}
static int load_flags(FILE *fp)
{
	uint8_t v=0;
	uint32_t botl=0;

#ifdef MAGIC
	if(!sr_u8(fp,&v)) return 0;
	flags.magic=v;
	if(!sr_u8(fp,&v)) return 0;
	flags.wmag=v;
#endif
	if(!sr_u8(fp,&v)) return 0;
	flags.topl=v;
	if(!sr_u32(fp,&botl)) return 0;
	flags.botl=(int)botl;
	if(!sr_u8(fp,&v)) return 0;
	flags.maze=v;
	if(!sr_u8(fp,&v)) return 0;
	flags.move=v;
	if(!sr_u8(fp,&v)) return 0;
	flags.mv=v;
	if(!sr_u8(fp,&v)) return 0;
	flags.mdone=v;
	if(!sr_u8(fp,&v)) return 0;
	flags.dscr=v;
#ifndef SMALL
	if(!sr_u8(fp,&v)) return 0;
	flags.one=v;
	if(!sr_u8(fp,&v)) return 0;
	flags.step=v;
	if(!sr_u8(fp,&v)) return 0;
	flags.flush=v;
#endif
	return 1;
}

static void save_you(FILE *fp)
{
	sw_u8(fp,(uint8_t)u.ux);
	sw_u8(fp,(uint8_t)u.uy);
	sw_u8(fp,(uint8_t)u.ufast);
	sw_u8(fp,(uint8_t)u.uconfused);
	sw_u8(fp,(uint8_t)u.uinvis);
	sw_u8(fp,(uint8_t)u.ulevel);
	sw_u8(fp,(uint8_t)u.utrap);
	sw_u8(fp,(uint8_t)u.upit);
	sw_u8(fp,(uint8_t)u.umconf);
	sw_u8(fp,(uint8_t)u.ufireres);
	sw_u8(fp,(uint8_t)u.ucoldres);
	sw_u8(fp,(uint8_t)u.uswallow);
	sw_u8(fp,(uint8_t)u.uswldtim);
	sw_u8(fp,(uint8_t)u.ucham);
	sw_u8(fp,(uint8_t)u.uhs);
	sw_u8(fp,(uint8_t)u.utel);
	sw_u8(fp,(uint8_t)u.upres);
	sw_u8(fp,(uint8_t)u.ustelth);
	sw_u8(fp,(uint8_t)u.uagmon);
	sw_u8(fp,(uint8_t)u.ufeed);
	sw_u8(fp,(uint8_t)u.usearch);
	sw_u8(fp,(uint8_t)u.ucinvis);
	sw_u8(fp,(uint8_t)u.uregen);
	sw_u8(fp,(uint8_t)u.ufloat);
	sw_u8(fp,(uint8_t)u.ustr);
	sw_u8(fp,(uint8_t)u.ustrmax);
	sw_u8(fp,(uint8_t)u.udaminc);
	sw_u8(fp,(uint8_t)u.uhp);
	sw_u8(fp,(uint8_t)u.uhpmax);
	sw_u8(fp,(uint8_t)u.uac);
	sw_u32(fp,(uint32_t)u.ugold);
	sw_u32(fp,(uint32_t)u.uexp);
	sw_u32(fp,(uint32_t)u.urexp);
	sw_u32(fp,(uint32_t)u.uhunger);
	sw_u32(fp,(uint32_t)u.ublind);
}

static int load_you(FILE *fp)
{
	uint8_t v=0;
	uint32_t w=0;

	if(!sr_u8(fp,&v)) return 0; u.ux=v;
	if(!sr_u8(fp,&v)) return 0; u.uy=v;
	if(!sr_u8(fp,&v)) return 0; u.ufast=v;
	if(!sr_u8(fp,&v)) return 0; u.uconfused=v;
	if(!sr_u8(fp,&v)) return 0; u.uinvis=v;
	if(!sr_u8(fp,&v)) return 0; u.ulevel=v;
	if(!sr_u8(fp,&v)) return 0; u.utrap=v;
	if(!sr_u8(fp,&v)) return 0; u.upit=v;
	if(!sr_u8(fp,&v)) return 0; u.umconf=v;
	if(!sr_u8(fp,&v)) return 0; u.ufireres=v;
	if(!sr_u8(fp,&v)) return 0; u.ucoldres=v;
	if(!sr_u8(fp,&v)) return 0; u.uswallow=v;
	if(!sr_u8(fp,&v)) return 0; u.uswldtim=v;
	if(!sr_u8(fp,&v)) return 0; u.ucham=v;
	if(!sr_u8(fp,&v)) return 0; u.uhs=v;
	if(!sr_u8(fp,&v)) return 0; u.utel=v;
	if(!sr_u8(fp,&v)) return 0; u.upres=v;
	if(!sr_u8(fp,&v)) return 0; u.ustelth=v;
	if(!sr_u8(fp,&v)) return 0; u.uagmon=v;
	if(!sr_u8(fp,&v)) return 0; u.ufeed=v;
	if(!sr_u8(fp,&v)) return 0; u.usearch=v;
	if(!sr_u8(fp,&v)) return 0; u.ucinvis=v;
	if(!sr_u8(fp,&v)) return 0; u.uregen=v;
	if(!sr_u8(fp,&v)) return 0; u.ufloat=v;
	if(!sr_u8(fp,&v)) return 0; u.ustr=v;
	if(!sr_u8(fp,&v)) return 0; u.ustrmax=v;
	if(!sr_u8(fp,&v)) return 0; u.udaminc=v;
	if(!sr_u8(fp,&v)) return 0; u.uhp=v;
	if(!sr_u8(fp,&v)) return 0; u.uhpmax=v;
	if(!sr_u8(fp,&v)) return 0; u.uac=v;
	if(!sr_u32(fp,&w)) return 0; u.ugold=w;
	if(!sr_u32(fp,&w)) return 0; u.uexp=w;
	if(!sr_u32(fp,&w)) return 0; u.urexp=w;
	if(!sr_u32(fp,&w)) return 0; u.uhunger=w;
	if(!sr_u32(fp,&w)) return 0; u.ublind=w;
	u.ustuck=0;
	return 1;
}

static void save_obj(FILE *fp, struct obj *otmp)
{
	sw_u8(fp,(uint8_t)otmp->ox);
	sw_u8(fp,(uint8_t)otmp->oy);
	sw_u8(fp,(uint8_t)otmp->olet);
	sw_u8(fp,(uint8_t)otmp->otyp);
	sw_u8(fp,(uint8_t)otmp->spe);
	sw_u8(fp,(uint8_t)otmp->quan);
	sw_u8(fp,(uint8_t)otmp->minus);
	sw_u8(fp,(uint8_t)otmp->known);
	sw_u8(fp,(uint8_t)otmp->cursed);
}

static int load_obj(FILE *fp, struct obj *otmp)
{
	uint8_t v=0;

	if(!sr_u8(fp,&v)) return 0; otmp->ox=v;
	if(!sr_u8(fp,&v)) return 0; otmp->oy=v;
	if(!sr_u8(fp,&v)) return 0; otmp->olet=v;
	if(!sr_u8(fp,&v)) return 0; otmp->otyp=v;
	if(!sr_u8(fp,&v)) return 0; otmp->spe=v;
	if(!sr_u8(fp,&v)) return 0; otmp->quan=v;
	if(!sr_u8(fp,&v)) return 0; otmp->minus=v;
	if(!sr_u8(fp,&v)) return 0; otmp->known=v;
	if(!sr_u8(fp,&v)) return 0; otmp->cursed=v;
	return 1;
}

static void save_invent(FILE *fp)
{
	struct obj *otmp;
	uint16_t count=0;

	for(otmp=invent;otmp;otmp=otmp->nobj) count++;
	sw_u16(fp,count);
	for(otmp=invent;otmp;otmp=otmp->nobj) {
		uint8_t worn=0;

		save_obj(fp,otmp);
		if(otmp==uarm) worn|=0x01;
		if(otmp==uwep) worn|=0x02;
		if(otmp==uleft) worn|=0x04;
		if(otmp==uright) worn|=0x08;
		sw_u8(fp,worn);
	}
}

static int load_invent(FILE *fp)
{
	uint16_t count=0;
	uint16_t i;
	struct obj *otmp=0,*last=0;

	if(!sr_u16(fp,&count)) return 0;
	invent=0;
	uwep=uarm=uleft=uright=0;
	for(i=0;i<count;i++) {
		uint8_t worn=0;

		otmp=alloc(sizeof(struct obj));
		memset(otmp,0,sizeof(*otmp));
		if(!load_obj(fp,otmp)) return 0;
		if(!sr_u8(fp,&worn)) return 0;
		otmp->nobj=0;
		if(!invent) invent=otmp;
		else last->nobj=otmp;
		last=otmp;
		if(worn&0x01) uarm=otmp;
		if(worn&0x02) uwep=otmp;
		if(worn&0x04) uleft=otmp;
		if(worn&0x08) uright=otmp;
	}
	return 1;
}

static int ustuck_index(void)
{
	int idx=0;
	struct monst *mtmp;

	if(!u.ustuck) return -1;
	for(mtmp=fmon;mtmp;mtmp=mtmp->nmon) {
		if(mtmp==u.ustuck) return idx;
		idx++;
	}
	return -1;
}

static void restore_ustuck(int idx)
{
	int i=0;
	struct monst *mtmp;

	u.ustuck=0;
	if(idx<0) return;
	for(mtmp=fmon;mtmp;mtmp=mtmp->nmon) {
		if(i==idx) { u.ustuck=mtmp; return; }
		i++;
	}
}

static void write_save_header(FILE *fp)
{
	write_exact(fp,SAVE_MAGIC,4);
	sw_u16(fp,(uint16_t)SAVE_VERSION);
	sw_u32(fp,(uint32_t)SAVE_ENDIANTAG);
}

static int read_save_header(FILE *fp)
{
	char magic[4];
	uint16_t version=0;
	uint32_t endtag=0;

	if(!read_exact(fp,magic,4)) return 0;
	if(memcmp(magic,SAVE_MAGIC,4)!=0) return 0;
	if(!sr_u16(fp,&version)) return 0;
	if(!sr_u32(fp,&endtag)) return 0;
	if(version!=SAVE_VERSION) return 0;
	if(endtag!=SAVE_ENDIANTAG) return 0;
	return 1;
}

int dosave0(int hu)
{
	char tmpfile[64];
	FILE *fp,*ofp;
	int tmp;
	int ustuck_idx;

	(void)hu;
	if((fp=fopen("nosave",READ))!=0) {
		pline("Save is down right now.");
		fclose(fp);
#ifdef MAGIC
		if(!flags.magic) return 0;
#endif
	}
	pline("Save begins...");
	fflush(stdout);
	signal(SIGINT,SIG_IGN);
	signal(SIGALRM,SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	snprintf(tmpfile,sizeof(tmpfile),"%s.tmp",SAVEF);
	if((fp=fopen(tmpfile,WRITE))==0) {
		pline("Cannot open save file.");
		return 0;
	}
	write_save_header(fp);
	ustuck_idx=ustuck_index();
	savelev(fp);
	save_invent(fp);
	save_flags(fp);
	sw_u8(fp,(uint8_t)dlevel);
	sw_u32(fp,(uint32_t)moves);
	save_you(fp);
	sw_u32(fp,(uint32_t)ustuck_idx);
	write_exact(fp,oiden,sizeof(oiden));
	save_string_array(fp,potcol,POTNUM);
	save_string_array(fp,scrnam,SCRNUM);
	save_string_array(fp,wannam,WANDNUM);
	save_string_array(fp,rinnam,RINGNUM);
	save_call_array(fp,potcall,POTNUM);
	save_call_array(fp,scrcall,SCRNUM);
	save_call_array(fp,wandcall,WANDNUM);
	save_call_array(fp,ringcall,RINGNUM);
	for(tmp=1;;tmp++) {
		glo(tmp);
		if(tmp==dlevel) continue;
		if((ofp=fopen(lock,READ))==0) break;
		getlev(ofp);
		fclose(ofp);
		sw_u8(fp,(uint8_t)tmp);
		savelev(fp);
		unlink(lock);
	}
	sw_u8(fp,0);
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
	if(rename(tmpfile,SAVEF)!=0) {
		unlink(tmpfile);
		pline("Error finalizing save file.");
		return 0;
	}
	cls();
	if(lock[1]=='l') {
		*index(lock,'.')=0;
		unlink(lock);
	}
	puts("Be seeing you...\n");
	cbout();
	exit(1);
	return 1;
}

void dosave(void)
{
	(void)dosave0(0);
}

void dorecover(FILE *fp)
{
	uint8_t lvl=0;
	uint32_t tmp=0;

	unlink(SAVEF);
	if(!read_save_header(fp)) {
		pline("Save file header invalid.");
		fclose(fp);
		return;
	}
	if(getlev(fp)) {
		fclose(fp);
		pline("Save file level corrupt.");
		return;
	}
	if(!load_invent(fp)) {
		fclose(fp);
		pline("Save file inventory corrupt.");
		return;
	}
	if(!load_flags(fp)) { fclose(fp); return; }
	if(!sr_u8(fp,&lvl)) { fclose(fp); return; }
	dlevel=lvl;
	if(!sr_u32(fp,&tmp)) { fclose(fp); return; }
	moves=tmp;
	if(!load_you(fp)) { fclose(fp); return; }
	if(!sr_u32(fp,&tmp)) { fclose(fp); return; }
	if(!read_exact(fp,oiden,sizeof(oiden))) { fclose(fp); return; }
	if(!load_string_array(fp,potcol,POTNUM)) { fclose(fp); return; }
	if(!load_string_array(fp,scrnam,SCRNUM)) { fclose(fp); return; }
	if(!load_string_array(fp,wannam,WANDNUM)) { fclose(fp); return; }
	if(!load_string_array(fp,rinnam,RINGNUM)) { fclose(fp); return; }
	if(!load_call_array(fp,potcall,POTNUM)) { fclose(fp); return; }
	if(!load_call_array(fp,scrcall,SCRNUM)) { fclose(fp); return; }
	if(!load_call_array(fp,wandcall,WANDNUM)) { fclose(fp); return; }
	if(!load_call_array(fp,ringcall,RINGNUM)) { fclose(fp); return; }
	{
		int ustuck_idx=(int)tmp;

		restore_ustuck(ustuck_idx);
		while(sr_u8(fp,&lvl) && lvl) {
			FILE *nfp;

			if(getlev(fp)) break;
			glo(lvl);
			if((nfp=fopen(lock,WRITE))==0)
				panic("No temp file %s\n",lock);
			savelev(nfp);
			fclose(nfp);
		}
		rewind(fp);
		if(!read_save_header(fp)) { fclose(fp); return; }
		if(!getlev(fp)) restore_ustuck(ustuck_idx);
	}
	fclose(fp);
	docrt();
}
