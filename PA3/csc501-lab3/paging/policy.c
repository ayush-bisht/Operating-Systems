/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>


extern int page_replace_policy;
extern node* front;
extern node* rear;
extern int debug_mode;

/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
  /* sanity check ! */

//  kprintf("To be implemented!\n");

	STATWORD ps;
	disable(ps);

	debug_mode = 1;		
	if(policy == SC)
	{
		page_replace_policy = SC;
		front = rear = NULL;
	}	
	else if(policy == LFU)
	{
		int i;
		page_replace_policy = LFU;
		for(i=0; i++; i<NFRAMES)
			frm_tab[i].fr_counter = 0;
	}
	else
	{
		kprintf("No such page replacement policy exsist\n");
		restore(ps);
		return SYSERR;
	}
	restore(ps);
  	return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
  return page_replace_policy;
}
