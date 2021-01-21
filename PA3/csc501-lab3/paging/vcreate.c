/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	//kprintf("To be implemented!\n");
	if(hsize >= BS_PG)
	{
		return SYSERR;	
	}
	STATWORD ps;
	disable(ps);
	int pid;
	struct pentry *pptr;
	struct mblock *bs_base;
	int i, avail;
	pid = create(procaddr,ssize,priority,name,nargs,args);
	if(pid == SYSERR)
	{
		restore(ps);
		return SYSERR;
	}
	pptr = &proctab[pid];

	if(get_bsm(&avail) == SYSERR)
	{
		restore(ps);
		return SYSERR;		//backing store not available
	}
		
	bsm_map(pid, VIR_BASE, avail, hsize);
	bsm_tab[avail].bs_type = BSM_PRIV;	//this is a private BS used as virtual heap for this process
	bsm_tab[avail].bs_pid = pid;
	bsm_tab[avail].bs_vpno = VIR_BASE;
	bsm_tab[avail].bs_npages = hsize;

	//no need to  maintain a list for private bs
	freemem(bsm_tab[avail].bs_qhead, sizeof(pq_node));	
	bsm_tab[avail].bs_qhead = NULL;
	
	//in addition to mapping with a backing store, give it a private heap	
	pptr->store = avail;
	pptr->vhpno = VIR_BASE;
	pptr->vhpnpages = hsize;
	pptr->vmemlist->mnext = VIR_BASE*NBPG;
	pptr->vmemlist->mlen = hsize*NBPG;


	//create the memory block
	bs_base = BACKING_STORE_BASE + (avail * BACKING_STORE_UNIT_SIZE);
	bs_base->mlen = hsize * NBPG;
	bs_base->mnext = NULL;

	enable(ps);
	return pid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
