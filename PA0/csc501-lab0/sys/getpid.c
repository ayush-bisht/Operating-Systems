/* getpid.c - getpid */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "lab0.h"
/*------------------------------------------------------------------------
 * getpid  --  get the process id of currently executing process
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
SYSCALL getpid()
{	
	unsigned long start_time;
	struct pentry *proc = &proctab[currpid];
	if(summary_flag == 1)
	{
		start_time = ctr1000;
		proc->s[sys_getpid].count++;
		proc->totalcount++;
		proc->s[sys_getpid].name = "sys_getpid";
		proc->s[sys_getpid].time = (proc->s[sys_getpid].time + (ctr1000 - start_time))/proc->s[sys_getpid].count;
	}
	return(currpid);
}
