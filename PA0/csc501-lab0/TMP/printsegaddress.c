
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <mem.h>


extern long etext,edata;
extern WORD _end;
void printsegaddress(){
	kprintf("void printsegaddress()\n");
        kprintf("Current: etext[0x%08X]=0x%08X, edata[0x%08X]=0x%08X, ebss[0x%08X]=0x%08X\n", &etext, etext, &edata, edata, &_end, _end);
        kprintf("Preceding: etext[0x%08X]=0x%08X, edata[0x%08X]=0x%08X, ebss[0x%08X]=0x%08X\n", &etext-1, *(&etext-1), &edata-1, *(&edata-1), &_end-1,*(&_end-1));
        kprintf("After: etext[0x%08X]=0x%08X, edata[0x%08X]=0x%08X, ebss[0x%08X]=0x%08X\n\n", &etext+1, *(&etext+1), &edata+1, *(&edata+1), &_end+1, *(&_end+1));
}

