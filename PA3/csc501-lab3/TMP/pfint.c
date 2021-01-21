/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{

 // 	kprintf("PAGE FAULT OCCURED!\n");
  	SYSCALL ps;
	disable(ps);
	virt_addr_t *vp;	//virtual page number
 	pd_t *pd_entry;	
	pt_t *pt_entry;	
	unsigned long vaddr, pdbr;
	unsigned int pd_offset, pg_offset, pt_offset;
	int bs, page, i, j, avail;
	fr_map_t *frmptr;	

	vaddr = read_cr2();	//faulted address
	if(bsm_lookup(currpid, vaddr, &bs, &page) == SYSERR)
	{
		kill(currpid);
		restore(ps);
		return SYSERR;
	}	
	vp = (virt_addr_t *)&vaddr;
        pg_offset = vp->pg_offset;
        pt_offset = vp->pt_offset;
        pd_offset = vp->pd_offset;

        pdbr = proctab[currpid].pdbr;	//current page directory 
        pd_entry = pdbr + pd_offset*sizeof(pd_t); //each entry is of size pd_t, page table we are looking for 	
	
	if(pd_entry->pd_pres == 0)	//page table in the directory is not present
	{
		i = create_pt(currpid);		//i= frame number where pt is created	
		if( i == SYSERR)	
		{
			restore(ps);
			return SYSERR;
		}
		pd_entry->pd_pres = 1;
		pd_entry->pd_write = 1;
		pd_entry->pd_base = i + FRAME0;
	}

		
	pt_entry = (pd_entry->pd_base*NBPG) + pt_offset*sizeof(pt_t); 
	
	frm_tab[pd_entry->pd_base - FRAME0].fr_refcnt += 1;
	
	if(get_frm(&avail) == SYSERR)
	{	
		restore(ps);
		return SYSERR;
	}
	
	frmptr = &frm_tab[avail];
	frmptr->fr_status = FRM_MAPPED;
	frmptr->fr_pid = currpid;
	frmptr->fr_refcnt = 1;
	frmptr->fr_type = FR_PAGE;
	frmptr->fr_vpno = vaddr/NBPG;
	frmptr->fr_dirty = 1;


	if(page_replace_policy == SC)
		insert_SC(avail);

	else if(page_replace_policy == LFU)
		frmptr->fr_counter++;
	
	if(bsm_lookup(currpid, vaddr, &bs, &page) == SYSERR)
        {
//               kprintf("Not a legal address, killing the process \n");
                kill(currpid);
                restore(ps);
                return SYSERR;
        }

	read_bs((avail+FRAME0)*NBPG, bs, page);
	
	pt_entry->pt_pres = 1;
	pt_entry->pt_write = 1;
	pt_entry->pt_base = avail+FRAME0;
		
	write_cr3(proctab[currpid].pdbr);
	
	restore(ps);
	return OK;
}


int create_pt(int pid)
{
        int avail,i;
        if(get_frm(&avail) == SYSERR)
                return SYSERR;

        fr_map_t *frmptr = &frm_tab[avail];
        frmptr->fr_status = FRM_MAPPED;
        frmptr->fr_pid = pid;
        frmptr->fr_type = FR_TBL;
        frmptr->fr_vpno = -1;
        frmptr->fr_dirty = 0;
        frmptr->fr_refcnt = 0;

        pt_t *pt_entry;
        pt_entry = (avail+FRAME0)*NBPG;

        for(i=0; i < NENTRY; i++)
        {
                pt_entry->pt_pres = 0;
                pt_entry->pt_write = 0;
                pt_entry->pt_user = 0;
                pt_entry->pt_pwt = 0;
                pt_entry->pt_pcd =0;
                pt_entry->pt_acc = 0;
                pt_entry->pt_dirty = 0;
                pt_entry->pt_mbz = 0;
                pt_entry->pt_global = 0;
                pt_entry->pt_avail = 0;
                pt_entry->pt_base = 0;
                pt_entry++;
        }

        return avail;   //return the frame where page table exists
}

