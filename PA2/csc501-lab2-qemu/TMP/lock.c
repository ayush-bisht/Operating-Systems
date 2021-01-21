#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>
#include <sleep.h>

extern int ctr1000;
int lock(int ldes1, int type, int priority)
{
	int i;
	struct lentry *lptr;
	struct pentry *pptr;
	STATWORD ps;
	lptr = &locktab[ldes1];
        pptr = &proctab[currpid];


	disable(ps);
	if(isbadlock(ldes1) || lptr->lstate == LFREE || pptr->plockret[ldes1] == SYSERR)
	{	
		restore(ps);
		return SYSERR;
	}	
		
	if(lptr->lstate == LUSED && lptr->ltype == NONE)
	{
		if(type == READ)
		{
			lptr->ltype = READ;
			lptr->nr++;
			pptr->plock[ldes1] = ACQUIRED;
			pptr->plockret[ldes1] = OK;

		}
		else if(type == WRITE)
		{
			lptr-> nw = 1;
			lptr->ltype = WRITE;
			pptr->plock[ldes1] = ACQUIRED;
			pptr->plockret[ldes1] = OK;
		}	
	}
	else if(lptr->lstate == LUSED && lptr->ltype == READ && type == READ)
	{
		int cur_time = ctr1000;
		if(lptr->nw == 0  || (lptr->nw == 1 && (priority > lptr->prio_write || (priority == lptr->prio_write && (cur_time - lptr->t_write) <= 40))))
		{
			lptr->nr++;
			pptr->plock[ldes1] = ACQUIRED;
			pptr->plockret[ldes1] = OK; 
		}		
		else
		{
//priority inheritance
			pptr->pstate = PRLOCKWAIT;
			pptr->plock[ldes1] = WAITING;
			pptr->lockid = ldes1; 
			pptr->plockret[ldes1] = OK;
			insert(currpid, lptr->lqhead, priority);

			findmaxprio(ldes1);
	                pinheritance(ldes1);
			
			resched();
		}
	
	}
	else if(lptr->lstate == LUSED && lptr->ltype == READ && type == WRITE)
	{

		
		lptr->nw = 1;
		lptr->pid_write = currpid;
		lptr->prio_write = priority;
		lptr->t_write = ctr1000;
		pptr->pstate = PRLOCKWAIT;
		pptr->plock[ldes1] = WAITING;
		pptr->lockid = ldes1;
		pptr->plockret[ldes1] = OK;
		insert(currpid, lptr->lqhead, priority);

		findmaxprio(ldes1);
                pinheritance(ldes1);

		resched();
	}
	else if(lptr->lstate == LUSED && lptr->ltype == WRITE)
	{

				
		pptr->pstate = PRLOCKWAIT;
		pptr->plock[ldes1] = WAITING;
		pptr->lockid = ldes1;
		pptr->plockret[ldes1] = OK;
		insert(currpid, lptr->lqhead, priority);

		findmaxprio(ldes1);
                pinheritance(ldes1);
	
		resched();
	
	}
       	restore(ps);
	if(pptr->plockret[ldes1] == DELETED)	return DELETED;
	return OK;	
}
