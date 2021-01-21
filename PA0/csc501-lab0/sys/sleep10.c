/* sleep10.c - sleep10 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include "lab0.h"
/*------------------------------------------------------------------------
 * sleep10  --  delay the caller for a time specified in tenths of seconds
 *------------------------------------------------------------------------
 */
extern unsigned long ctr1000;
SYSCALL	sleep10(int n)
{
	unsigned long start_time;
        struct pentry *proc = &proctab[currpid];

        if(summary_flag == 1)
        {
                start_time = ctr1000;
                proc->totalcount++;
                proc->s[sys_sleep10].count++;
                proc->s[sys_sleep10].name = "sys_sleep10";

        }

	STATWORD ps;    
	if (n < 0  || clkruns==0)
	         return(SYSERR);
	disable(ps);
	if (n == 0) {		/* sleep10(0) -> end time slice */
	        ;
	} else {
		insertd(currpid,clockq,n*100);
		slnempty = TRUE;
		sltop = &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
        restore(ps);
     	if(summary_flag == 1)
        {
                proc->s[sys_sleep10].time = (proc->s[sys_sleep10].time + (ctr1000 - start_time))/ proc->s[sys_sleep10].count;
        }

	return(OK);
}
