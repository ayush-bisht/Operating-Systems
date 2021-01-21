/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <paging.h>
/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;
	int i,j;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}

//release all the frames used by this process
	for(i=0; i<NFRAMES; i++)
	{
		if(frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_pid == currpid)
		{
	                frm_tab[i].fr_status = FRM_UNMAPPED;
	                frm_tab[i].fr_pid = -1;
        	        frm_tab[i].fr_vpno = -1;
               	 	frm_tab[i].fr_refcnt = 0;
                	frm_tab[i].fr_type = FR_PAGE;
                	frm_tab[i].fr_dirty = 0;

		}
	}

//release any private bs held by any process
	for(i=0; i< BS_MAX; i++)
	{
		if(bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_pid == currpid)
		{
				
		       	bsm_tab[i].bs_status = BSM_UNMAPPED;
	       		bsm_tab[i].bs_pid = -1;
       	 		bsm_tab[i].bs_vpno = VIR_BASE;
        		bsm_tab[i].bs_npages = 0;
        		bsm_tab[i].bs_type = BSM_NONE;
			bsm_tab[i].bs_pcount = 0;		
			if(bsm_tab[i].bs_type == BSM_SHARE)
			{

		        	bsm_tab[i].bs_pcount--;
                                pq_node *curr = bsm_tab[i].bs_qhead;
                                pq_node *prev = NULL;
                                if(bsm_tab[i].bs_pcount == 0)
                                {
                                	freemem(bsm_tab[i].bs_qhead, sizeof(pq_node));
                                        bsm_tab[i].bs_qhead = NULL;
                                        free_bsm(i);
                                }
                                else
                                {
                      	        	if(curr->pid == pid)    //the head node is to be removed
                                        {
                                        	bsm_tab[i].bs_qhead = curr->next;
                                        	freemem(curr, sizeof(pq_node));
                                                bsm_tab[i].bs_pid = bsm_tab[i].bs_qhead->pid;
                                                bsm_tab[i].bs_vpno = bsm_tab[i].bs_qhead->vpno;
                                                bsm_tab[i].bs_npages = bsm_tab[i].bs_qhead->npages;
					}
                                        while(curr!= NULL && curr->pid == pid)
                                        {
                                        	prev = curr;
                                               	curr = curr->next;
                                        }
                                        prev->next = curr->next;
                                        freemem(curr, sizeof(pq_node));
                                }
	
			} 
		}
	}

	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
