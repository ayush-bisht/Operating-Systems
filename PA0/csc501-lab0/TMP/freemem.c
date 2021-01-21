/* freemem.c - freemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <stdio.h>
#include <proc.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 *  freemem  --  free a memory block, returning it to memlist
 *------------------------------------------------------------------------
 */
extern unsigned long ctr1000;
SYSCALL	freemem(struct mblock *block, unsigned size)
{
// 	kprintf("ENTERING FREEMEM for THE PROC %d\n", currpid);
	unsigned long start_time;
	struct pentry *proc;
	if(summary_flag == 1)
        {
		proc = &proctab[currpid];	
                start_time = ctr1000;
                proc->totalcount++;
                proc->s[sys_freemem].count++;
                proc->s[sys_freemem].name = "sys_freemem";

        }

	
	STATWORD ps;    
	struct	mblock	*p, *q;
	unsigned top;

	if (size==0 || (unsigned)block>(unsigned)maxaddr
	    || ((unsigned)block)<((unsigned) &end))
		return(SYSERR);
	size = (unsigned)roundmb(size);
	disable(ps);
	for( p=memlist.mnext,q= &memlist;
	     p != (struct mblock *) NULL && p < block ;
	     q=p,p=p->mnext )
		;
	if (((top=q->mlen+(unsigned)q)>(unsigned)block && q!= &memlist) ||
	    (p!=NULL && (size+(unsigned)block) > (unsigned)p )) {
		restore(ps);
		return(SYSERR);
	}
	if ( q!= &memlist && top == (unsigned)block )
			q->mlen += size;
	else {
		block->mlen = size;
		block->mnext = p;
		q->mnext = block;
		q = block;
	}
	if ( (unsigned)( q->mlen + (unsigned)q ) == (unsigned)p) {
		q->mlen += p->mlen;
		q->mnext = p->mnext;
	}
	restore(ps);

	if(summary_flag == 1)
        {
		proc = &proctab[currpid];
                proc->s[sys_freemem].time = (proc->s[sys_freemem].time + (ctr1000 - start_time))/ proc->s[sys_freemem].count;
        }


	return(OK);
}
