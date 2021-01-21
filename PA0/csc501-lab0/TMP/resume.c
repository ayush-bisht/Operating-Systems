/* resume.c - resume */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 * resume  --  unsuspend a process, making it ready; return the priority
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
SYSCALL resume(int pid)
{
	unsigned start_time;
	struct pentry *proc = &proctab[currpid];
        if(summary_flag == 1)
        {
		start_time = ctr1000;
                proc->totalcount++;
		proc->s[sys_resume].count++;
                proc->s[sys_resume].name = "sys_resume";
        }

	STATWORD ps;    
	struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
	int	prio;			/* priority to return		*/

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate!=PRSUSP) {
		restore(ps);
		return(SYSERR);
	}
	prio = pptr->pprio;
	ready(pid, RESCHYES);
	restore(ps);
	
	if(summary_flag == 1)
	{
		proc->s[sys_resume].time = (proc->s[sys_resume].time + (ctr1000 - start_time));// proc->s[sys_resume].count;
	}
	return(prio);
}
