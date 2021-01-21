/* sleep.c - sleep */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include "lab0.h"
/*------------------------------------------------------------------------
 * sleep  --  delay the calling process n seconds
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
//unsigned long start_time;

SYSCALL	sleep(int n)
{
	unsigned long start_time;
	struct pentry *proc = &proctab[currpid];
	
	if(summary_flag == 1)
	{	
		start_time = ctr1000;
		proc->totalcount++;
		proc->s[sys_sleep].count++;
		proc->s[sys_sleep].name = "sys_sleep";
	
	}
	STATWORD ps;    
	if (n<0 || clkruns==0)
		return(SYSERR);
	if (n == 0) {
	        disable(ps);
		resched();
		restore(ps);
		return(OK);
	}
	while (n >= 1000) {
		sleep10(10000);
		n -= 1000;
	}
	if (n > 0)
		sleep10(10*n);
	
	if(summary_flag == 1)
	{
		proc->s[sys_sleep].time = (proc->s[sys_sleep].time + (ctr1000 - start_time))/ proc->s[sys_sleep].count;
	}
	return(OK);
}
