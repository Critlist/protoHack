#ifndef rnd
rn1(x,y)
register x,y;
{
	return((rand()%x)+y);
}
rn2(x)
register x;
{
	return(rand()%x);
}
rnd(x)
register x;
{
	return((rand()%x)+1);
}
#endif
d(n,x)
register n,x;
{
	register tmp=0;

	while(n--) tmp+=(rand()%x)+1;
	return(tmp);
}
