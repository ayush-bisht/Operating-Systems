#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
   STATWORD ps;
   disable(ps);
//    kprintf("To be implemented!\n");
 
   int i;
   if(bs_id < 0 || bs_id >= BS_MAX)
   {
	restore(ps);
	return SYSERR;	
   }

   bs_map_t *bsptr =  &bsm_tab[bs_id];

  
   if(bsptr->bs_status == BSM_MAPPED)
   {
	if(bsptr->bs_type == BSM_PRIV)
		free_bsm(bs_id);
	else if(bsptr->bs_type == BSM_SHARE)
	{
		if(bsptr->bs_pcount == 1)
		{
			freemem(bsptr->bs_qhead, sizeof(pq_node));
			bsptr->bs_qhead = NULL;
			free_bsm(bs_id);
		}
		else if(bsptr->bs_pcount > 1)
		{
			restore(ps);
			return SYSERR;
		}
	}
   } 

   restore(ps);
   return OK;

}

