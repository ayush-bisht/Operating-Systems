
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

unsigned long *esp, *ebp;

void printtos(){

        kprintf("void printtos()\n");
        unsigned long *temp;
        int count=4;
        asm("movl %ebp, ebp");
        asm("movl %esp, esp");
        kprintf("Before[0x%08X]: 0x%08X\n",ebp+2, *(ebp+2));
        kprintf("After [0x%08X]: 0x%08X\n",ebp, *ebp);
        temp = esp;
        while(count > 0)
        {
                kprintf("\telement[0x%08X]: 0x%08X\n",temp, *temp);
                temp++;
                count--;
        }
        kprintf("\n");
}

