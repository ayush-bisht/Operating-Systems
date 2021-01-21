/* setdev.c - setdev */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "lab0.h"
/*------------------------------------------------------------------------
 *  setdev  -  set the two device entries in the process table entry
 *------------------------------------------------------------------------
 */
extern unsigned long ctr1000;
SYSCALL	setdev(int pid, int dev1, int dev2)
{
	unsigned long start_time;
        struct pentry *proc = &proctab[currpid];

        if(summary_flag == 1)
        {
                start_time = ctr1000;
                proc->totalcount++;
                proc->s[sys_setdev].count++;
                proc->s[sys_setdev].name = "sys_setdev";

        }
	short	*nxtdev;

	if (isbadpid(pid))
		return(SYSERR);
	nxtdev = (short *) proctab[pid].pdevs;
	*nxtdev++ = dev1;
	*nxtdev = dev2;

     if(summary_flag == 1)
        {
                proc->s[sys_setdev].time = (proc->s[sys_setdev].time + (ctr1000 - start_time))/ proc->s[sys_setdev].count;
        }

	return(OK);
}
