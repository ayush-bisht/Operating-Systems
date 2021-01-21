/* sleep100.c - sleep100 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include "lab0.h"
/*------------------------------------------------------------------------
 * sleep100  --  delay the caller for a time specified in 1/100 of seconds
 *------------------------------------------------------------------------
 */
extern unsigned long ctr1000;
SYSCALL sleep100(int n)
{
	unsigned long start_time;
        struct pentry *proc = &proctab[currpid];

        if(summary_flag == 1)
        {
                start_time = ctr1000;
                proc->totalcount++;
                proc->s[sys_sleep100].count++;
                proc->s[sys_sleep100].name = "sys_sleep100";

        }

	STATWORD ps;    

	if (n < 0  || clkruns==0)
	         return(SYSERR);
	disable(ps);
	if (n == 0) {		/* sleep100(0) -> end time slice */
	        ;
	} else {
		insertd(currpid,clockq,n*10);
		slnempty = TRUE;
		sltop = &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
        restore(ps);

     	if(summary_flag == 1)
        {
                proc->s[sys_sleep100].time = (proc->s[sys_sleep100].time + (ctr1000 - start_time))/ proc->s[sys_sleep100].count;
        }

	return(OK);
}
