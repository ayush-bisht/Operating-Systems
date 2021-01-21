/* signal.c - signal */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include "lab0.h"
/*------------------------------------------------------------------------
 * signal  --  signal a semaphore, releasing one waiting process
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;

SYSCALL signal(int sem)
{

	unsigned long start_time;
        struct pentry *proc = &proctab[currpid];

        if(summary_flag == 1)
        {
                start_time = ctr1000;
                proc->totalcount++;
                proc->s[sys_signal].count++;
                proc->s[sys_signal].name = "sys_signal";

        }
	STATWORD ps;    
	register struct	sentry	*sptr;

	disable(ps);
	if (isbadsem(sem) || (sptr= &semaph[sem])->sstate==SFREE) {
		restore(ps);
		return(SYSERR);
	}
	if ((sptr->semcnt++) < 0)
		ready(getfirst(sptr->sqhead), RESCHYES);
	restore(ps);
     	if(summary_flag == 1)
        {
                proc->s[sys_signal].time = (proc->s[sys_signal].time + (ctr1000 - start_time))/ proc->s[sys_signal].count;
        }

	return(OK);
}
