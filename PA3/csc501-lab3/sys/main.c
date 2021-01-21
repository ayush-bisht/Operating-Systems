/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
/*
int main()
{
	kprintf("\n\nHello World, Xinu@QEMU lives\n\n");
	
	int i=0;
	pd_t *entry = FRAME0*NBPG;
	for(;i< 4; i++)
	{
		kprintf("null proc entry %d, value %d \n", i+1, entry->pd_base);
		entry++;
	}

	unsigned int pd = 10;
	kprintf("print %x\n", sizeof(pd*sizeof(pd_t)));	
	virt_addr_t *vp= 1025*NBPG;
	kprintf("%d %d %d \n", vp->pd_offset, vp->pt_offset, vp->pg_offset);

	unsigned int v_addr = 1025;
	kprintf("%d %d \n", v_addr/1024, v_addr&1023);	
       
	unsigned int vaddr = 1025*NBPG;
	kprintf("%d %d", vaddr >> 22, (vaddr >> 12) & 0x000003ff);
 shutdown();
}*/

#define PROC1_VADDR	0x40000000
#define PROC1_VPNO      0x40000
#define PROC2_VADDR     0x80000000
#define PROC2_VPNO      0x80000
#define TEST1_BS	1

void proc1_test1(char *msg, int lck) {
	char *addr;
	int i;

	get_bs(TEST1_BS, 100);
//	kprintf("BS id: %d\n",TEST1_BS);
	if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
		kprintf("xmmap call failed\n");
		kprintf("BS :%d", TEST1_BS);
		sleep(3);
		return;
	}
//	printbs(TEST1_BS);
	addr = (char*) PROC1_VADDR;
	for (i = 0; i < 26; i++) {
		*(addr + i * NBPG) = 'A' + i;
	}

	sleep(6);

	for (i = 0; i < 26; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}
/*
	node* curr = front;
	do
	{
		kprintf("frame: %d\n", curr->frm_no);
		curr = curr->next;
	}while(curr != front);
*/
//	kprintf("AFTER XMMAP AND WRITING THE DATA..................\n");
//	print_frame();
//	print_pt();

//	xmunmap(PROC1_VPNO); 
//	printbs(TEST1_BS);
//	kprintf("AFTER XMUMAP.................\n");
//	print_frame();
//	print_pt();    
	return;
}

void proc1_test1b(char *msg, int lck) {
        char *addr;
        int i;

        get_bs(TEST1_BS, 100);
	kprintf("BS id: %d\n",TEST1_BS);
        if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
                kprintf("xmmap call failed\n");
                kprintf("BS :%d", TEST1_BS);
                sleep(3);
                return;
        }

        addr = (char*) PROC1_VADDR;
        for (i = 0; i < 26; i++) {
                *(addr + i * NBPG) = 'Z' - i;
        }

        sleep(6);

        for (i = 0; i < 26; i++) {
                kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
        }

/*        node* curr = front;
        do
        {
                kprintf("frame: %d\n", curr->frm_no);
                curr = curr->next;
        }while(curr != front);
*/
          kprintf("AFTER XMMAP AND WRITING THE DATA\n");
//        print_frame();
      //  print_pt();
        xmunmap(PROC1_VPNO);
        return;
}


void proc1_test2(char *msg, int lck) {
	int *x;

	kprintf("ready to allocate heap space\n");
	x = vgetmem(1024);
	kprintf("heap allocated at %x\n", x);
	*x = 100;
	*(x + 1) = 200;

	kprintf("heap variable: %d %d\n", *x, *(x + 1));
	vfreemem(x, 1024);
}

void proc1_test3(char *msg, int lck) {

	char *addr;
	int i;

	addr = (char*) 0x0;

	for (i = 0; i < 1024; i++) {
		*(addr + i * NBPG) = 'B';
	}

	for (i = 0; i < 1024; i++) {
		kprintf(" 0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	return;
}

void print_frame()
{
	int i=0;
	for(i=0;i<10;i++)
	{
		if(frm_tab[i].fr_status==FRM_MAPPED){
		kprintf("frame :%d\n",i);
		kprintf("status: %d\n", frm_tab[i].fr_status);
		kprintf("pid: %d\n", frm_tab[i].fr_pid);
		kprintf("vpno: %d\n", frm_tab[i].fr_vpno);
		kprintf("fr_type: %d\n", frm_tab[i].fr_type);
		kprintf("fr_refcnt: %d\n", frm_tab[i].fr_refcnt);
		kprintf("fr_dirty: %d\n", frm_tab[i].fr_dirty);
		kprintf("\n");
	}
	}
}


void print_pt()
{
	int i=0;
	virt_addr_t *vp;
        unsigned int pt_offset, pd_offset, vaddr;
        unsigned long pdbr;
        pd_t *pd_entry;
        pt_t *pt_entry;
        int frm_no, pid;

	for(i==0;i<NFRAMES;i++)
	{	
		if(frm_tab[i].fr_status==FRM_MAPPED){

                   frm_no = i;
                        vaddr = frm_tab[frm_no].fr_vpno*NBPG;
                        vp = (virt_addr_t*)&vaddr;
                        pid = frm_tab[frm_no].fr_pid;
                        pdbr = proctab[pid].pdbr;
                        pd_offset = vp->pd_offset;
                       pt_offset = vp->pt_offset;

	        //    pd_offset = (vaddr)>>22;
	        //    pt_offset = (vaddr)>>12 & 0x000003ff;
                        pd_entry = pdbr + pd_offset*sizeof(pd_t);
                        pt_entry = (pd_entry->pd_base*NBPG) + pt_offset*sizeof(pt_t);

 	kprintf("frm_no: %d, pid: %d, vaddr: %u, pdbr: %u, pd_offest: %d, pt_offset: %d, pd_entry: %0x, pt_entery: %0x, pt_entry->pt_acc: %d\n", frm_no, pid, vaddr, pdbr, pd_offset, pt_offset, pd_entry, pt_entry, pt_entry->pt_acc);
	}
	}
	kprintf("\n\n\n");
}
int main() {
	int pid1;
	int pid2;

//	kprintf("BEFORE PROC1 created\n");
//        print_frame();
//        print_pt();  
/*
        kprintf("\n3: Frame test\n");
        pid1 = create(proc1_test3, 2000, 20, "proc1_test3", 0, NULL);
        resume(pid1);
        sleep(3);
*/

	srpolicy(LFU);
	kprintf("debug mode: %d", debug_mode);
	kprintf("\n1: shared memory\n");
	pid1 = create(proc1_test1, 2000, 20, "proc1_test1", 0, NULL);
	resume(pid1);
	sleep(10);

//	        kprintf("AFTER PROC1 is i\n");
//       print_frame();
//        print_pt();  
/*
	
	pid1 = create(proc1_test1b, 2000, 20, "proc1_test1", 0, NULL);
	resume(pid1);
	sleep(10);
*/

/*
	        kprintf("\n3: Frame test\n");
        pid1 = create(proc1_test3, 2000, 20, "proc1_test3", 0, NULL);
        resume(pid1);
        sleep(3);
*/

	kprintf("\n2: vgetmem/vfreemem\n");
	pid1 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
	kprintf("pid %d has private heap\n", pid1);
	resume(pid1);
	sleep(3);


	kprintf("\n3: Frame test\n");
	pid1 = create(proc1_test3, 2000, 20, "proc1_test3", 0, NULL);
	resume(pid1);
	sleep(3);

}



