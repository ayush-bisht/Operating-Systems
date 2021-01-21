/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  //kprintf("xmmap - to be implemented!\n");
  
	STATWORD ps;
 	disable(ps);
  	if(virtpage < VIR_BASE || source < 0 || source >= BS_MAX || npages < 0 || npages >= BS_PG)
  	{
		restore(ps);
		return SYSERR;
 	}
  
  	bsm_map(currpid, virtpage, source, npages);
  	restore(ps);
  	return OK;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
//  kprintf("To be implemented!");
 	STATWORD ps;
 	disable(ps);
 	if(virtpage < VIR_BASE)
 	{
		restore(ps);
		return SYSERR;
 	} 
 	bsm_unmap(currpid, virtpage);
 	
	restore(ps);
	return OK;
}
