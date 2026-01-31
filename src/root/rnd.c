#include "../compat.h"

#ifndef rnd
int rn1(int x, int y)
{
	return((rand()%x)+y);
}
int rn2(int x)
{
	return(rand()%x);
}
int rnd(int x)
{
	return((rand()%x)+1);
}
#endif
int d(int n, int x)
{
	register int tmp=0;

	while(n--) tmp+=(rand()%x)+1;
	return(tmp);
}
