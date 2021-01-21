/* setnok.c - setnok */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include "lab0.h"
/*------------------------------------------------------------------------
 *  setnok  -  set next-of-kin (notified at death) for a given process
 *------------------------------------------------------------------------
 */
extern unsigned long ctr1000;
SYSCALL	setnok(int nok, int pid)
{
	unsigned long start_time;
        struct pentry *proc = &proctab[currpid];

        if(summary_flag == 1)
        {
                start_time = ctr1000;
                proc->totalcount++;
                proc->s[sys_setnok].count++;
                proc->s[sys_setnok].name = "sys_setnok";

        }

	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid)) {
		restore(ps);
		return(SYSERR);
	}
	pptr = &proctab[pid];
	pptr->pnxtkin = nok;
	restore(ps);

     if(summary_flag == 1)
        {
                proc->s[sys_setnok].time = (proc->s[sys_setnok].time + (ctr1000 - start_time))/ proc->s[sys_setnok].count;
        }

	return(OK);
}
