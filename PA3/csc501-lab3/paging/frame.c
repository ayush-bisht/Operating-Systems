/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  //kprintf("To be implemented!\n");
	
	STATWORD ps;
	disable(ps);
	int i;
	for(i=0;i<NFRAMES; i++)
	{
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = -1;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = FR_PAGE;
		frm_tab[i].fr_dirty = 0;
		frm_tab[i].fr_counter = 0;
	}
	
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  //kprintf("To be implemented!\n");
  	STATWORD ps;
	disable(ps);

	int i, frm_no;
	for(i=0; i<NFRAMES; i++)
	{
		if(frm_tab[i].fr_status == FRM_UNMAPPED)
		{
			*avail = i;
			restore(ps);
			return OK;
		}
	}

	if(page_replace_policy == SC)
	{
		frm_no = remove_SC();
		if(frm_no != SYSERR)
		{
			free_frm(frm_no);
			*avail = frm_no;
			restore(ps);
			return OK;	
		}
	}
	else if(page_replace_policy == LFU)
	{
		frm_no = remove_LFU();
		if(frm_no != SYSERR)
		{
			free_frm(frm_no);
			*avail = frm_no;
			restore(ps);
			return OK;
		}
	}
	restore(ps);	
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

//  kprintf("To be implemented!\n");
//	kprintf("FREEING FRAME %d\n", i);
 	STATWORD ps;
	disable(ps);
	unsigned int vaddr = frm_tab[i].fr_vpno*NBPG;
	int frm_pid = frm_tab[i].fr_pid;
	int bs, page;
	unsigned int pd_offset, pt_offset;
	virt_addr_t *vp;
	fr_map_t *frmptr;
	pd_t *pd_entry;
	pt_t *pt_entry;
	if(frm_tab[i].fr_type == FR_TBL || frm_tab[i].fr_type == FR_DIR)
	{
		kprintf("Frame can not be freed \n");
		restore(ps);
		return SYSERR;
	}
	if(frm_tab[i].fr_type == FR_PAGE)
	{
		if(bsm_lookup(frm_pid, vaddr, &bs, &page) == OK)
		{	
			write_bs((i+FRAME0)*NBPG, bs, page);
		}
		vp = (virt_addr_t *)&vaddr;
        
        	pt_offset = vp->pt_offset;
               	pd_offset = vp->pd_offset;
		pd_entry = proctab[frm_pid].pdbr + pd_offset*sizeof(pd_t);
		pt_entry = pd_entry->pd_base + pt_offset*sizeof(pt_t);
		
		pt_entry -> pt_pres = 0;	//the particular page is no longer present, reset the page table entry
		pt_entry -> pt_write = 1;
		pt_entry -> pt_user = 0;
    		pt_entry -> pt_pwt = 0;
    		pt_entry -> pt_pcd = 0;
    		pt_entry -> pt_acc = 0;
    		pt_entry -> pt_dirty = 0;
    		pt_entry -> pt_mbz = 0;
    		pt_entry -> pt_global = 0;
    		pt_entry -> pt_avail = 0;
    		pt_entry -> pt_base = 0;

		frmptr = &frm_tab[pd_entry->pd_base - FRAME0];
		frmptr->fr_refcnt--;

		if(frmptr->fr_refcnt == 0)	//the page table no longer contains any present page,remove from the memory
		{
			frmptr->fr_status = FRM_UNMAPPED;
			frmptr->fr_pid = -1;
			frmptr->fr_vpno = -1;
			frmptr->fr_type = FR_PAGE;
			frmptr->fr_dirty = 0;
			
			pd_entry->pd_pres = 0;	//page table is no longer present
		} 	
	

	        //free the frame
        	frm_tab[i].fr_status = FRM_UNMAPPED;
        	frm_tab[i].fr_pid = -1;
        	frm_tab[i].fr_vpno = -1;
		frm_tab[i].fr_refcnt = 0; 
        	frm_tab[i].fr_type = FR_PAGE;
        	frm_tab[i].fr_dirty = 0;
	
	}
	restore(ps); 
	return OK;
}


void insert_SC(int frame)
{
	node* new_node = (struct node*)getmem(sizeof(node));
	new_node->frm_no = frame;
	new_node->next = NULL;
	if(front == NULL)
	{
		front = rear = new_node;
	}	
	else
	{
		rear->next = new_node;
		rear = new_node;
	}
	rear->next = front;
}

int remove_SC()
{
//	kprintf("inside remove SC");
	virt_addr_t *vp;
	unsigned int pt_offset, pd_offset, vaddr;
	unsigned long pdbr;
	pd_t *pd_entry;
	pt_t *pt_entry;
	int frm_no, pid;
	int frm_found = 0;
	int i;
	if(front == NULL)
	{
		kprintf("frame list is empty");		
		return SYSERR;
	}
	if(front == rear)	//only 1 frame in the queue, no need to check the reference count as it will be removed after rfcnt -- 
	{
		frm_no = front->frm_no;
		freemem(front, sizeof(node));
		front = end = NULL;
		return frm_no;
	}
	else
	{
		node* curr = front;
		node* prev = NULL;
		while(frm_found == 0)
		{
			frm_no = curr->frm_no;
			vaddr = frm_tab[frm_no].fr_vpno*NBPG;
			vp = (virt_addr_t*)&vaddr;
			pid = frm_tab[frm_no].fr_pid;
			pdbr = proctab[pid].pdbr;
			pd_offset = vp->pd_offset;
			pt_offset = vp->pt_offset;
			
			pd_entry = pdbr + pd_offset*sizeof(pd_t);
			pt_entry = (pd_entry->pd_base*NBPG) + pt_offset*sizeof(pt_t);
			
			if(pt_entry->pt_acc == 1)
			{
				pt_entry->pt_acc = 0;
				prev = curr;
				curr = curr->next;
			}
			else if(pt_entry->pt_acc == 0)
			{
				frm_found = 1;
				if(curr == front)
				{
					front = front->next;
					rear->next = front;
				}
				else if(curr == rear)
				{
					rear = prev;
					rear->next = front;
				}
				else
				{
					prev->next = curr->next;
					curr->next = NULL;
				}
				freemem(curr, sizeof(node));		
			
		       	if(debug_mode== 1)
				kprintf("REMOVING FRAME: %d\n",frm_no);
               		return frm_no;
			}
		}
	}
	
	return SYSERR;
}


int remove_LFU()
{
	int i=0,max_vpno= 0, min_counter= MAXINT;	
	int fr_counter, fr_vpno, frm_no = -1;
	for(i = 0; i< NFRAMES; i++)
	{	
		fr_counter = frm_tab[i].fr_counter;
		fr_vpno = frm_tab[i].fr_vpno;
		if(frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_type == FR_PAGE)
		{
			if(fr_counter < min_counter || (fr_counter == min_counter && fr_vpno > max_vpno))
			{
				min_counter = fr_counter;
				max_vpno = fr_vpno;
				frm_no = i;
			}
		}
	}
	
	if(frm_no != -1)
	{
		if(debug_mode == 1)
			kprintf("REMOVING FRAME: %d\n",frm_no);
		return frm_no;
	}
	return SYSERR;
}
