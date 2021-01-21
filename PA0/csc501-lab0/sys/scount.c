/* scount.c - scount */

#include <conf.h>
#include <kernel.h>
#include <sem.h>
#include <proc.h>
#include "lab0.h"
/*------------------------------------------------------------------------
 *  scount  --  return a semaphore count
 *------------------------------------------------------------------------
 */
extern unsigned long ctr1000;

SYSCALL scount(int sem)
{
extern	struct	sentry	semaph[];

	unsigned long start_time;
        struct pentry *proc = &proctab[currpid];

        if(summary_flag == 1)
        {
                start_time = ctr1000;
                proc->totalcount++;
                proc->s[sys_scount].count++;
                proc->s[sys_scount].name = "sys_scount";

        }
	if (isbadsem(sem) || semaph[sem].sstate==SFREE)
		return(SYSERR);
        if(summary_flag == 1)
        {
                proc->s[sys_scount].time = (proc->s[sys_scount].time + (ctr1000 - start_time))/ proc->s[sys_scount].count;
        }

	return(semaph[sem].semcnt);
}
