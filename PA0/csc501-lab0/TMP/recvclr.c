/* recvclr.c - recvclr */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include "lab0.h"
/*------------------------------------------------------------------------
 *  recvclr  --  clear messages, returning waiting message (if any)
 *------------------------------------------------------------------------
 */
extern unsigned long ctr1000;

SYSCALL	recvclr()
{
	unsigned long start_time;
        struct pentry *proc = &proctab[currpid];

        if(summary_flag == 1)
        {
                start_time = ctr1000;
                proc->totalcount++;
                proc->s[sys_recvclr].count++;
                proc->s[sys_recvclr].name = "sys_recvclr";

        }
	STATWORD ps;    
	WORD	msg;

	disable(ps);
	if (proctab[currpid].phasmsg) {
		proctab[currpid].phasmsg = 0;
		msg = proctab[currpid].pmsg;
	} else
		msg = OK;
	restore(ps);
	if(summary_flag == 1)
        {
                proc->s[sys_recvclr].time = (proc->s[sys_recvclr].time + (ctr1000 - start_time))/ proc->s[sys_recvclr].count;
        }

	return(msg);
}
