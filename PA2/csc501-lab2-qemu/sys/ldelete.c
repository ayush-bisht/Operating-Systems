#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>
#include <sleep.h>

int ldelete(int lockdescriptor)
{
	STATWORD ps;
	struct pentry *pptr;
	struct lentry *lptr;
	int pid, i;
	int r_type = OK;

	disable(ps);
	lptr = &locktab[lockdescriptor];
	pptr = &proctab[currpid];
	if(isbadlock(lockdescriptor) || lptr->lstate == LFREE)
	{
		restore(ps);
		return SYSERR;
	} 
	
	lptr->lstate = LFREE;
        lptr->ltype = NONE;
	lptr->nr = 0;
        lptr->nw = 0;
        lptr->pid_write = -1;
        lptr->prio_write = MININT;
        lptr->t_write = -1;
        lptr->lprio = 0;
	
	for(i=0; i< NPROC; i++)
        {
                if(proctab[i].plock[lockdescriptor] == INT || proctab[i].plock[lockdescriptor] == ACQUIRED)  
			proctab[i].plockret[lockdescriptor] = SYSERR;
	}

	if (nonempty(lptr->lqhead)) {
                while( (pid=getfirst(lptr->lqhead)) != EMPTY)
                  {
                    	proctab[pid].plockret[lockdescriptor] = DELETED;
                    	proctab[pid].plock[lockdescriptor] = NOINT;
		  	ready(pid,RESCHNO);
                  }
                resched();
        }
        restore(ps);
        return(OK);
}
