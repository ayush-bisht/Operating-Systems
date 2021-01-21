/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <date.h>
#include <proc.h>
#include "lab0.h"


extern int getutim(unsigned long *);

/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
SYSCALL	gettime(long *timvar)
{
    /* long	now; */

	/* FIXME -- no getutim */

	unsigned long start_time;
        struct pentry *proc = &proctab[currpid];

        if(summary_flag == 1)
        {
                start_time = ctr1000;
                proc->totalcount++;
                proc->s[sys_gettime].count++;
                proc->s[sys_gettime].name = "sys_gettime";
 		proc->s[sys_gettime].time = (proc->s[sys_gettime].time + (ctr1000 - start_time))/ proc->s[sys_gettime].count;
       
        }
	
	
    return OK;
}
