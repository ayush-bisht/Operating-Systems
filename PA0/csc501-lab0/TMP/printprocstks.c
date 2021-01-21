
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>



unsigned long *esp;

void printprocstks(int priority){
        int i =0;
        kprintf("void printprocstks()\n");
	for(i; i<NPROC; i++)
        {
                if(proctab[i].pstate != PRFREE && proctab[i].pprio > priority)
                {
                        kprintf("Process [%s]\n",proctab[i].pname);
                        kprintf("\t pid: %d\n",i);
                        kprintf("\t priority: %d\n",proctab[i].pprio);
                        kprintf("\t base: 0x%08X\n",proctab[i].pbase);
                        kprintf("\t limit: 0x%08X\n",proctab[i].plimit);
                        kprintf("\t len: %d\n",proctab[i].pstklen);
                	if(i == currpid) {
				asm("movl %esp, esp"); 
				kprintf("\t pointer: 0x %08X\n", esp);	
			 }       
		 	else	kprintf("\t pointer: 0x %08X\n",proctab[i].pesp);
                }
        }
	kprintf("\n");
}

