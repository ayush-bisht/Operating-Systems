/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */

int prX,prX2;
prch(c)
char c;
{
	int i;
	sleep(1);
}

prch1(c)
char c;
{
	int j;
	sleep(5);
}
int main()
{
	kprintf("\n\nHello World, Xinu lives\n\n");
	long op;
	long ip = 0xaabbccdd;
	op = zfunction(ip);
	printf("The output of zfunction is: 0x%X\n\n",op);	
	printsegaddress();
	printtos();
//	create(prch1,2000,39,"prox Y",1,'B');
	printprocstks(0);	
	syscallsummary_start();
	int i = getpid();
	resume(prX = create(prch,2000,20,"proc X",1,'A'));
	sleep(2);
	sleep(2);
	syscallsummary_stop();
	printsyscallsummary();
	return 0;
}
