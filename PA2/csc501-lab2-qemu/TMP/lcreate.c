#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

int lcreate()
{
	STATWORD ps;
	int i;
	int lock;

	disable(ps);
	for(i= 0; i < NLOCK; i++)
	{	
		lock = nextlock--;
		if(nextlock < 0)
			nextlock = NLOCK-1;
		
		if (locktab[lock].lstate == LFREE && locktab[lock].ltype == NONE)
		{
			locktab[lock].lstate = LUSED;
			restore(ps);
			return(locktab[lock].ldesc);
		}
	}

	restore(ps);
	return(SYSERR);
}

