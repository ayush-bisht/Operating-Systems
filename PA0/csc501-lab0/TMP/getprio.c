/* getprio.c - getprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

#include "lab0.h"
/*------------------------------------------------------------------------
 * getprio -- return the scheduling priority of a given process
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
SYSCALL getprio(int pid)

{
	unsigned long start_time;
        struct pentry *proc = &proctab[currpid];

        if(summary_flag == 1)
        {
                start_time = ctr1000;
                proc->totalcount++;
                proc->s[sys_getprio].count++;
                proc->s[sys_getprio].name = "sys_getprio";

        }
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	restore(ps);

	if(summary_flag == 1)
        {
                proc->s[sys_getprio].time = (proc->s[sys_getprio].time + (ctr1000 - start_time))/ proc->s[sys_getprio].count;
        }

	return(pptr->pprio);
}
