/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
SYSCALL chprio(int pid, int newprio)
{
	unsigned long start_time;
	struct pentry *proc = &proctab[currpid];
	 if(summary_flag == 1)
        {
                start_time = ctr1000;
		proc->totalcount++;
                proc->s[sys_chprio].count++;
                proc->s[sys_chprio].name = "sys_chprio";

        }

	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	restore(ps);

	if(summary_flag == 1)
        {
	        proc->s[sys_chprio].time = (proc->s[sys_chprio].time + (ctr1000 - start_time))/ proc->s[sys_chprio].count;
        }

	return(newprio);
}
