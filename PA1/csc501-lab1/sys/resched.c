
/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lab1.h"
unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */

extern struct qent q[];


/*void print_rdyqueue1()	function to print all the elements of the ready queue, only used for testing 
{
        int i = rdyhead;
        int j = rdytail;
        kprintf("INSIDE PRINT READY QUEUE.................. \n");
        while (q[i].qnext != rdytail)
        {
                i = q[i].qnext;
                kprintf("the key of the ready process at position %d, is :%d \n", i, q[i].qkey);
        }
}*/

int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */
	int i,j,  prio_sum, random_prio, max_goodness, total_quantumleft, newid;
	int class = getschedclass();
	switch (class)
	{
	case 1:

		prio_sum = 0;
	 	if((currpid == 0) && isempty(rdyhead))                  //null process if the current process and no other ready processes ? return
                {
                        return(OK);
                }
		
		optr = &proctab[currpid];
		if(optr->pstate == PRCURR)
		{
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio);
		}
		//find total priority of ready processes
		i =rdyhead;
		while(q[i].qnext !=rdytail)
		{	
			i = q[i].qnext;
			prio_sum += q[i].qkey;

		}

		if(prio_sum == 0)
		{	
			currpid = 0;
			goto NEXTPROCESS;
		}
		random_prio = rand()%(prio_sum);
		
		j = rdytail;
		while(q[j].qprev != rdyhead)
		{	
			j = q[j].qprev;
			if(q[j].qkey > random_prio)
			{
				currpid = j;
				break;		
			}	
			else	random_prio = random_prio - q[j].qkey;
		}
		
		NEXTPROCESS:
		nptr = &proctab[currpid];
		dequeue(currpid);
		nptr->pstate = PRCURR;
	
       	 	#ifdef  RTCLOCK
        	preempt = QUANTUM;              /* reset preemption counter     */
        	#endif	
		break;
	
	case 2:
		
		max_goodness = 0;
               	total_quantumleft = 0;

		optr = &proctab[currpid]; 

		if(preempt <= 0)
		{
			optr->counter = 0;
			optr->goodness = 0;
		}
		else
		{
			optr->goodness = (optr->goodness - optr->counter) + preempt;// if prio is changed in between epoch the goodness value will still be calculated based on the previous priority
			optr->counter = preempt;
		}
			
		i = q[rdyhead].qnext;
		while(i !=rdytail)
		{
			total_quantumleft += proctab[i].counter;
			i = q[i].qnext;
		}
		
		if(optr->pstate == PRCURR)	total_quantumleft += optr->counter;	//if current process still wants to run, then is quantum left should be take into consideration as well
	
		if(total_quantumleft == 0)	//start a new epoch assign new quantum  
		{
			for(j=0; j <NPROC; j++)
			{
				if(proctab[i].pstate != PRFREE)
				{
					if (proctab[j].counter == proctab[j].quantum || proctab[j].counter == 0)	proctab[j].quantum = proctab[j].pprio;
					else	proctab[j].quantum = proctab[j].pprio + (int)(proctab[j].counter/2);
					proctab[j].counter = proctab[j].quantum;
					proctab[j].goodness = proctab[j].pprio + proctab[j].counter;
				}		
			}
		}
		
				
		//find the ready proc with max goodness to run irrespective of curr epoch or new epoch
		
		i = q[rdyhead].qnext;
		while(i != rdytail)
		{
			if(proctab[i].goodness >= max_goodness)	// >=so that process with same goodness will be executed in round robin fashion, since we are picking up processes from the ready queue
			{
				max_goodness = proctab[i].goodness;		
				newid = i;		
			}	
			i= q[i].qnext;
		}
		
			
		if(max_goodness >= 0 && optr->goodness > max_goodness && optr->pstate == PRCURR)//if optr->goodness == max_goodness, we need to run the other process since we have to use RR for processes of equal goodness
		{
			preempt = optr->counter;
			return(OK);
		}
		else if(max_goodness > 0 && optr->pstate == PRCURR)
		{	
			optr->pstate = PRREADY;
			insert(currpid, rdyhead,optr->pprio);
		}	
		else if (max_goodness == 0 && currpid ==0)
		{
			preempt = optr->counter;
			return(OK);
		}
		else	newid = 0;
		
		currpid = newid;
		dequeue(currpid);
		nptr = &proctab[currpid];
		nptr->pstate = PRCURR;
		preempt = nptr->counter;
		break;			

	default:
	        /* no switch needed if current process priority higher than next*/
		
        	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
           	(lastkey(rdytail)<optr->pprio)) {
                	return(OK);
        	}

        	/* force context switch */
		if (optr->pstate == PRCURR) {
                	optr->pstate = PRREADY;
                	insert(currpid,rdyhead,optr->pprio);
        	}

        	/* remove highest priority process at end of ready list */
        	nptr = &proctab[ (currpid = getlast(rdytail)) ];
        	nptr->pstate = PRCURR;          /* mark it currently running    */
	
		#ifdef  RTCLOCK
        	preempt = QUANTUM;              /* reset preemption counter     */
		#endif
		break;
      	}

       ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
       /* The OLD process returns here when resumed. */
       return OK;
}
