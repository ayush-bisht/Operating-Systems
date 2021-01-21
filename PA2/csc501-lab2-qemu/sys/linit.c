#include<kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>

struct lentry locktab[NLOCK];
int nextlock;
LOCAL void transitivity();

void linit()
{
	STATWORD ps;
	disable(ps);
        struct lentry *lptr;
        int i;
        nextlock = NLOCK-1;
        kprintf("Initializing locks\n");
        for (i=0 ; i<NLOCK ; i++) {      /* initialize locks */
                (lptr = &locktab[i])->lstate = LFREE;
                lptr->lqtail = 1 + (lptr -> lqhead = newqueue());
               	lptr->ldesc = i;
		lptr->ltype = NONE;
		lptr->nr = 0;
                lptr->nw = 0;
		lptr->pid_write = -1;
                lptr->prio_write = MININT;
		lptr->t_write = -1;	
        	lptr->lprio = 0; //max process priority of the process waiting on the lock 
	}
	enable(ps);

}


void pinheritance(int lock)
{
	struct pentry *pptr;
	struct lentry *lptr = &locktab[lock];
	int maxprio = lptr->lprio;
	int i;
	for(i = 0; i <  NPROC; i++)
	{
		if((pptr=&proctab[i])->plock[lock] == ACQUIRED)
		{
			if(maxprio == 0 && pptr->pinh != 0)	pptr->pinh = 0;
			else if((pptr->pinh != 0 && pptr->pinh == maxprio) || (pptr->pinh == 0 && pptr->pprio >= maxprio))	continue;
			else
			{
				pptr->pinh = maxprio;
				if(pptr->pstate == PRREADY)
				{
					dequeue(i);
					insert(i, rdyhead, pptr->pinh);
				}
				if(pptr->lockid != -1)	//this process is waiting on a lock, for transitivty 				  
				{
					findmaxprio(pptr->lockid);
					pinheritance(pptr->lockid);
				}		
			}
		}	
	}
}

void findmaxprio(int lock)
{
	int maxprio,i ;
	struct lentry *lptr = &locktab[lock];
	struct pentry *pptr;
	i = q[lptr->lqhead].qnext;
	if(isempty(lptr->lqhead))	
	{
		lptr->lprio = 0;
		return;
	}
	lptr->lprio = 0;
	while(i != lptr->lqtail)
	{
		pptr = &proctab[i];
		if(pptr->pinh != 0 && pptr->pinh > lptr->lprio)	
		{	lptr->lprio = pptr->pinh;
		}
		else if(pptr->pinh == 0 && pptr->pprio > lptr->lprio)
		{	lptr->lprio = pptr->pprio;	
		}
		i = q[i].qnext;
	}
	
}

/* Print lock status */
/*
void printlock(int lock)
{
	struct lentry *lptr = &locktab[lock];
	kprintf("*******LOCK STATUS*********\n");
	kprintf("desc --> %d\n", lptr->ldesc);
	kprintf("state --> %d\n", lptr->lstate);
	kprintf("type --> %d\n", lptr->ltype);
	kprintf("nr --> %d\n", lptr->nr);
	kprintf("nw --> %d\n", lptr->nw);
	kprintf("pid_write --> %d\n", lptr->pid_write);
	kprintf("pid_read --> %d\n", lptr->prio_write);
	kprintf("t_write --> %d\n", lptr->t_write);
	kprintf("lqhead --> %d\n", lptr->lqhead);
	kprintf("lqtail --> %d\n", lptr->lqtail);
	kprintf("lprio --> %d\n", lptr->lprio);
	int i = 0;
	struct pentry *pptr;
	for( ; i < NPROC; i++)
	{
		if((pptr = &proctab[i])->plock[lock] != NOINT)
		{
			kprintf("status of process i %d--> %d\n ", i , pptr->plock[lock]);
		}
	} 
	
	i = q[lptr->lqhead].qnext;
	kprintf("waiting queue has --> \t");
	while( i!= lptr->lqtail)
	{	
		kprintf("pid - %d, lock prio - %d \t", i, q[i].qkey);
		i = q[i].qnext;
	}
	kprintf("\n\n");
}
*/
