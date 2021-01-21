/* receive.c - receive */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include "lab0.h"
/*------------------------------------------------------------------------
 *  receive  -  wait for a message and return it
 *------------------------------------------------------------------------
 */
extern unsigned long ctr1000;
SYSCALL	receive()
{
	unsigned long start_time;
        struct pentry *proc = &proctab[currpid];

        if(summary_flag == 1)
        {
                start_time = ctr1000;
                proc->totalcount++;
                proc->s[sys_receive].count++;
                proc->s[sys_receive].name = "sys_receive";

        }

	STATWORD ps;    
	struct	pentry	*pptr;
	WORD	msg;

	disable(ps);
	pptr = &proctab[currpid];
	if ( !pptr->phasmsg ) {		/* if no message, wait for one	*/
		pptr->pstate = PRRECV;
		resched();
	}
	msg = pptr->pmsg;		/* retrieve message		*/
	pptr->phasmsg = FALSE;
	restore(ps);
	if(summary_flag == 1)
        {
                proc->s[sys_receive].time = (proc->s[sys_receive].time + (ctr1000 - start_time))/ proc->s[sys_receive].count;
        }

	return(msg);
}
