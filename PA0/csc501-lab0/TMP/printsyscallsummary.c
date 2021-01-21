
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include "lab0.h"

void syscallsummary_start()
{	
	struct pentry *proc;
	summary_flag = 1;
	int i,j;
	for(i = 0; i < NPROC; i++)
	{
		proc = &proctab[i];
		for(j=0; j< 43; j++)
		{
			proc->s[j].count = 0;
		}	
	}
	return;
}

void syscallsummary_stop()
{
	summary_flag = 0;
	return;
}

void printsyscallsummary()
{
	int i,j;
	struct pentry *proc;
	kprintf("void printsyscallsummary()\n");
	
	for(i=0; i < NPROC; i++)
	{
		proc = &proctab[i];
		if(proc->totalcount > 0)
		{	
			kprintf("Process [pid: %d]\n",i);
			for( j =0 ; j < 43; j++)
			{
				if(proc->s[j].count > 0)
				{
					kprintf("\tSyscall: %s, count: %d, average execution time: %lu (ms)\n", proc->s[j].name, proc->s[j].count, proc->s[j].time);
				}	  
			}
		}
	}

	return;
}
