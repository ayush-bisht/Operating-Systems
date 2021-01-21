#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

    /* requests a new mapping of npages with ID map_id */
	STATWORD ps;
    	disable(ps);

    	if(npages == 0 || npages >= BS_PG || bs_id >= BS_MAX || bs_id < 0)
   	{
		restore(ps);
		return SYSERR;
    	}

    	bs_map_t *bsptr= &bsm_tab[bs_id];
	

	if(bsptr->bs_status == BSM_MAPPED && bsptr->bs_type == BSM_PRIV)
	{
		kprintf("The BS is being used as a private BS");
		restore(ps);
		return SYSERR;	
	}   

	else if(bsptr->bs_type == BSM_SHARE && bsptr->bs_status == BSM_MAPPED)	//shared, but mapped to another process
	{	
		restore(ps);
		return bsptr->bs_npages;
	}
	//not created i.e. still unmapped, it can be mapped
    	else if(bsptr->bs_status == BSM_UNMAPPED)
    	{	
    		restore(ps);
    		return npages; 
    	}
	
	restore(ps);
	return SYSERR;
 
}


