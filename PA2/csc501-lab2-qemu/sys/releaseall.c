#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>
#include <q.h>

int releaseall(int numlocks, int ldes1, ...)
{
	unsigned long *a; 	//pointer to arguments
	STATWORD ps;
	int i, j, curr_type;
	int nextpid;
	int r_type = OK;	
	struct pentry *pptr;
	struct lentry *lptr;
	pptr = &proctab[currpid];
	disable(ps);
	
	a = (unsigned long*)(&ldes1) + (numlocks-1);
	for( ; numlocks>0; numlocks--)
	{
		int curr_lock = (int)*a--; 
		lptr = &locktab[curr_lock];
		
		if(pptr->plock[curr_lock] != ACQUIRED || lptr->lstate == LFREE || lptr->ltype == NONE || isbadlock(curr_lock) || pptr->plockret[curr_lock] == SYSERR)
		{
			r_type = SYSERR;
		}
		else
		{
			pptr->plock[curr_lock] = INT;	//this process might used this lock in the future
			pptr->plockret[curr_lock] = OK;
			pptr->pinh = 0;
			if(isempty(lptr->lqhead)) 	//no process is waiting for the lock
			{
				if(lptr->ltype == READ)
				{	
					lptr->nr--;
					if(lptr->nr == 0)
						lptr->ltype = NONE;
						lptr->lprio = 0;	
				}				
				else if(lptr->ltype == WRITE)
				{
					lptr->ltype = NONE;
					lptr->nw = 0;
					lptr->prio_write = MININT;
					lptr->pid_write = -1;
					lptr->t_write = -1;
					lptr->lprio = 0;
				}
			}
			else
			{
				if(lptr->ltype == WRITE && nonempty(lptr->lqhead))
				{	
					
					lptr->ltype = READ;
		 	               	lptr->nw = 0;
					i = q[lptr->lqtail].qprev;
					while(nonempty(lptr->lqhead))
					{
						lptr->nr++;
						proctab[i].plock[curr_lock] = ACQUIRED;
						proctab[i].lockid = -1;
						proctab[i].plockret[curr_lock] = OK;	//may not be necessary
						i = q[i].qprev;
						ready(getlast(lptr->lqtail), RESCHNO);
					}

					resched();

				}
				else if(lptr->ltype == READ)
				{
					lptr->nr--;
					if(lptr->nr == 0)
					{
						lptr->ltype = WRITE;
						lptr->nw = 1;
						proctab[lptr->pid_write].plock[curr_lock] = ACQUIRED;
						proctab[lptr->pid_write].lockid = -1;
						proctab[lptr->pid_write].plockret[curr_lock] = OK;	//may not be required
						
						findmaxprio(lptr->ldesc);	//if any reader has a higher priority than the writer which will run next, have to inc the prio of writer to that reader.
						pinheritance(lptr->ldesc);
	
          					lptr->pid_write = -1;   //reset all time parameters since the writer is no longer waiting 
                                                lptr->prio_write = MININT;
                                                lptr->t_write = -1;
						ready(getlast(lptr->lqtail), RESCHYES);
					}
				}
				
		
			} 
		}


	}
	
	
	int templock;
	for(j = 0 ; j < NLOCK; j++)
	{	
		if(pptr->plock[j] == ACQUIRED)
		{	
	                findmaxprio(locktab[j].ldesc);
        	        templock = locktab[j].lprio;
			if(templock != 0){
				if((pptr-> pinh == 0 && pptr->pprio < templock) || (pptr->pinh!= 0 && pptr->pinh < templock))	pptr->pinh = templock;
			}
		}
	}
	

	restore(ps);
	return r_type;
	
}

