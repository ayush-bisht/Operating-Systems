/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */


SYSCALL init_bsm()
{
	STATWORD ps;
	disable(ps);
	int i, j;
	for(i=0; i< BS_MAX; i++)
	{
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_vpno = VIR_BASE;
		bsm_tab[i].bs_npages = 0;
		bsm_tab[i].bs_sem = 0;
		bsm_tab[i].bs_type = BSM_NONE;
		bsm_tab[i].bs_qhead = NULL;
		bsm_tab[i].bs_pcount = 0;
	}
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	STATWORD ps;
	disable(ps);
	int i;
	for(i =0; i<BS_MAX; i++)
	{
		if(bsm_tab[i].bs_status == BSM_UNMAPPED)
		{
			*avail = i;
			restore(ps);
			return OK;	
		}
	}
	
	kprintf("BACKING STORE NOT AVAILABLE \n");
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
//	kprintf("freeing the bsm mapping %d\n", i); 
	STATWORD ps;
	disable(ps);

	bsm_tab[i].bs_status = BSM_UNMAPPED;
        bsm_tab[i].bs_pid = -1;
        bsm_tab[i].bs_vpno = VIR_BASE;
        bsm_tab[i].bs_npages = 0;
	bsm_tab[i].bs_type = BSM_NONE;
	bsm_tab[i].bs_qhead =NULL;
	bsm_tab[i].bs_pcount = 0;
	
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	STATWORD ps;
	disable(ps);
	int i;
	for(i = 0; i < BS_MAX; i++)
	{
		if(bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_type == BSM_PRIV && bsm_tab[i].bs_pid == pid)
		{
			*store = i;
			*pageth = vaddr/NBPG - bsm_tab[i].bs_vpno;
			restore(ps);
			return OK;
		}
		else if(bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_type == BSM_SHARE)
		{
			pq_node* node = bsm_tab[i].bs_qhead;
			int tpid = -1;
			while(node!=NULL)
			{
				if(node->pid == pid)
				{
					*store = i;
                        		*pageth = vaddr/NBPG - node->vpno; 
					restore(ps);
					return OK;
				}
				node = node->next;
			}	
		}
	}
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	STATWORD ps;
	disable(ps);

	if(bsm_tab[source].bs_status == BSM_MAPPED && bsm_tab[source].bs_type == BSM_PRIV)
	{	
		restore(ps);
		return SYSERR;
	}	
	
	else if(bsm_tab[source].bs_status == BSM_MAPPED && bsm_tab[source].bs_type == BSM_SHARE)
	{
		bsm_tab[source].bs_pcount++;
		pq_node* new_node = NULL;
                new_node = (struct pq_node*)getmem(sizeof(pq_node));
                new_node->pid = pid;
                new_node->vpno = vpno;
                new_node->npages = npages;
                new_node->next = NULL;

		pq_node* head = bsm_tab[source].bs_qhead;
		while(head->next != NULL)
		{
			head = head->next;
		}  
		
		head->next = new_node;
	}
	else if(bsm_tab[source].bs_status == BSM_UNMAPPED)
	{
		bsm_tab[source].bs_status = BSM_MAPPED;
		bsm_tab[source].bs_type =  BSM_SHARE;
		bsm_tab[source].bs_pid = pid;
		bsm_tab[source].bs_vpno = vpno;
		bsm_tab[source].bs_npages = npages;
		
		bsm_tab[source].bs_pcount = 1;
		pq_node* new_node = NULL;
		new_node = (struct pq_node*)getmem(sizeof(pq_node));
	 	new_node->pid = pid;
		new_node->vpno = vpno;
		new_node->npages = npages;
		new_node->next = NULL;
		bsm_tab[source].bs_qhead = new_node;
	}
	
	enable(ps);
	return OK;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	STATWORD ps;
	disable(ps);
	int i, bs, page, j, free=1;	
	unsigned int vaddr = vpno*NBPG;
	
	for(i=0; i < NFRAMES; i++)
	{
		if(frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_PAGE && frm_tab[i].fr_status == FRM_MAPPED)
		{
			if(bsm_lookup(pid, vaddr, &bs, &page) == OK)
			{
				write_bs((i+FRAME0)*NBPG, bs, page);
			}
		}
	}		
	if(bsm_tab[bs].bs_type == BSM_PRIV)	free_bsm(bs);
	else if(bsm_tab[bs].bs_type == BSM_SHARE)
	{
		bsm_tab[bs].bs_pcount--;
		pq_node *curr = bsm_tab[bs].bs_qhead;
		pq_node *prev = NULL;
		if(bsm_tab[bs].bs_pcount == 0)
		{
			freemem(bsm_tab[bs].bs_qhead, sizeof(pq_node));
			bsm_tab[bs].bs_qhead = NULL;
			free_bsm(bs);
		}
		else
		{
			if(curr->pid == pid)	//the head node is to be removed
			{
				bsm_tab[bs].bs_qhead = curr->next;
				freemem(curr, sizeof(pq_node));	
				bsm_tab[bs].bs_pid = bsm_tab[bs].bs_qhead->pid;
                		bsm_tab[bs].bs_vpno = bsm_tab[bs].bs_qhead->vpno;
                		bsm_tab[bs].bs_npages = bsm_tab[bs].bs_qhead->npages;
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

	enable(ps);
	return OK;
}

void printbs(i)
{
	kprintf("bs status: %d\n", bsm_tab[i].bs_status); 
	kprintf("bs pid: %d\n", bsm_tab[i].bs_pid);
	kprintf("bs vpno: %d\n", bsm_tab[i].bs_vpno);
	kprintf("bs npages: %d\n", bsm_tab[i].bs_npages);
	kprintf("bs type: %d\n", bsm_tab[i].bs_type);
	kprintf("bs bs_count: %d\n", bsm_tab[i].bs_pcount);
	kprintf("bs list: ");
	pq_node *node = bsm_tab[i].bs_qhead;
	while(node!= NULL)
	{
	kprintf("q node pid: %d\n", node->pid);
	kprintf("q node vpno: %d\n", node->vpno);
	kprintf("q node npages: %d\n", node->npages);
	node = node->next;
	}
	
}
