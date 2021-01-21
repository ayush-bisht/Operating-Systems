/* user.c - main */

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>

#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }
int mystrncmp(char* des,char* target,int n){
    int i;
    for (i=0;i<n;i++){
        if (target[i] == '.') continue;
        if (des[i] != target[i]) return 1;
    }
    return 0;
}
extern int ctr1000;
/*--------------------------------Test 1--------------------------------*/
 
void reader1 (char *msg, int lck)
{
	lock (lck, READ, DEFAULT_LOCK_PRIO);
	kprintf ("  %s: acquired lock, sleep 2s\n", msg);
	sleep (2);
	kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void test1 ()
{
	int	lck;
	int	pid1;
	int	pid2;

	kprintf("\nTest 1: readers can share the rwlock\n");
	lck  = lcreate ();
	assert (lck != SYSERR, "Test 1 failed");

	pid1 = create(reader1, 2000, 20, "reader a", 2, "reader a", lck);
	pid2 = create(reader1, 2000, 20, "reader b", 2, "reader b", lck);

	resume(pid1);
	resume(pid2);
	
	sleep (5);
	ldelete (lck);
	kprintf ("Test 1 ok\n");
}

/*----------------------------------Test 2---------------------------*/
char output2[10];
int count2;
void reader2 (char msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        output2[count2++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output2[count2++]=msg;
        kprintf ("  %c: to release lock\n", msg);
	releaseall (1, lck);
}

void writer2 (char msg, int lck, int lprio)
{
	kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, WRITE, lprio);
        output2[count2++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output2[count2++]=msg;
        kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}

void test2 ()
{
        count2 = 0;
        int     lck;
        int     rd1, rd2, rd3, rd4;
        int     wr1;

        kprintf("\nTest 2: wait on locks with priority. Expected order of"
		" lock acquisition is: reader A, reader B, reader D, writer C & reader E\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 2 failed");

	rd1 = create(reader2, 2000, 20, "reader2", 3, 'A', lck, 20);
	rd2 = create(reader2, 2000, 20, "reader2", 3, 'B', lck, 30);
	rd3 = create(reader2, 2000, 20, "reader2", 3, 'D', lck, 25);
	rd4 = create(reader2, 2000, 20, "reader2", 3, 'E', lck, 20);
        wr1 = create(writer2, 2000, 20, "writer2", 3, 'C', lck, 28);
	
        kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(rd1);
        sleep (1);

        kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume(wr1);
        sleep10 (1);


        kprintf("-start reader B, D, E. reader B is granted lock.\n");
        resume (rd2);
	resume (rd3);
	resume (rd4);


        sleep (15);
        kprintf("output=%s\n", output2);
        assert(mystrncmp(output2,"ABABCCDEED",10)==0,"Test 2 FAILED\n");
        kprintf ("Test 2 OK\n");
}

/*----------------------------------Test 3---------------------------*/
void reader3 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer3 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test3 ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 3: test the basic priority inheritence\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 3 failed");

        rd1 = create(reader3, 2000, 25, "reader3", 2, "reader A", lck);
        rd2 = create(reader3, 2000, 30, "reader3", 2, "reader B", lck);
        wr1 = create(writer3, 2000, 20, "writer3", 2, "writer", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
	assert (getprio(wr1) == 25, "Test 3 failed");

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
	sleep (1);
	assert (getprio(wr1) == 30, "Test 3 failed");

	kprintf("-change reader B, then sleep 1s\n");
	chprio(rd2, 20);
	sleep(1);
	assert(getprio(wr1)==25, "Test 3 failed");
	
	kprintf("-change reader B, then sleep 1s\n");
        chprio(rd2, 40);
        sleep(1);
        assert(getprio(wr1)==40, "Test 3 failed");

	
	kprintf("-kill reader B, then sleep 1s\n");
	kill (rd2);
	sleep (1);
	assert (getprio(wr1) == 25, "Test 3 failed");

	kprintf("-kill reader A, then sleep 1s\n");
	kill (rd1);
	sleep(1);
	assert(getprio(wr1) == 20, "Test 3 failed");

        sleep (8);
        kprintf ("Test 3 OK\n");
}
#if 0
/*----------------------TEST 4-------------------------------------------*/

/* testing priority inversion with transitivity */


void reader4 (char *msg, int lck, int lck2)
{
        int     ret;

        kprintf ("  %s: to acquire lock %d\n", msg, lck);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock %d\n", msg, lck);	
	kprintf ("  %s: to acquire lock %d\n", msg, lck2);
	lock (lck2, READ, DEFAULT_LOCK_PRIO);
	kprintf (" %s: acquired lock2 %d\n", msg, lck2);
	sleep(2);
        kprintf (" %s: to release lock %d\n", msg, lck);
        kprintf (" %s: to release lock %d\n", msg, lck2);
	releaseall (2, lck, lck2);
	kprintf("prioirty of reader 1 %d\n", getprio(48));
        kprintf("priority of writer 1 %d\n", getprio(47));
        kprintf("priority of writer 2 %d\n\n\n", getprio(46));
}

void writer4 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock %d\n", msg, lck);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s %d\n", msg,lck);
	sleep (4);
        kprintf ("  %s: to release lock %d\n", msg, lck);
        releaseall (1, lck);
	kprintf("prioirty of reader 1 %d\n", getprio(48));
        kprintf("priority of writer 1 %d\n", getprio(47));
        kprintf("priority of writer 2 %d\n\n\n", getprio(46));
}

void writer5 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock %d\n", msg, lck);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
	kprintf ("  %s: acquired lock %d\n", msg, lck);
       // kprintf ("  %s: acquired lock, sleep 5 sec %d\n", msg, lck);
       // sleep(5);
	kprintf ("  %s: to release lock %d\n", msg, lck);
        releaseall (1, lck);
}


void test4()
{
	int     lck, lck2;
        int     rd1, rd2;
        int     wr1, wr2;

        kprintf("\nTest 4: test the basic priority inheritence with transitivity\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 4 failed");

	lck2 = lcreate ();
	assert (lck2 != SYSERR, "Test 4 failed");
        rd1 = create(reader4, 2000, 20, "reader4", 3, "reader C", lck, lck2);
       // rd2 = create(reader3, 2000, 30, "reader3", 2, "reader B", lck);
        wr1 = create(writer5, 2000, 40, "writer4", 2, "writer A", lck);
	wr2 = create(writer4, 2000, 30, "writer4", 2, "writer B", lck2);

        kprintf("-start writer 2, then sleep 1s. writer2(prio 30) acquires lock lck2, priority of r1 should not change\n");
        resume(wr2);
        sleep (1);

	kprintf("-start reader, then sleep 1s. lock granted to rd1 (prio 20)\n");
        resume(rd1);
        sleep (1);
        
	kprintf("prioirty of reader 1 %d\n", getprio(rd1));
	kprintf("priority of writer 1 %d\n", getprio(wr1));
	kprintf("priority of writer 2 %d\n\n\n", getprio(wr2));
	

        kprintf("-start writer 1, then sleep 1s. writer 1(prio 40) blocked on the lock1\n");
        resume (wr1);
	sleep(1);
        kprintf("prioirty of reader 1 %d\n", getprio(rd1));
        kprintf("priority of writer 1 %d\n", getprio(wr1));
        kprintf("priority of writer 2 %d\n\n\n", getprio(wr2));
	
	sleep(6);
	kprintf("prioirty of reader 1 %d\n", getprio(rd1));
        kprintf("priority of writer 1 %d\n", getprio(wr1));
        kprintf("priority of writer 2 %d\n\n\n", getprio(wr2));        
}
#endif

#if 0

/*------------------------------ test 6-------------------------------------*/
/* checking 0.4 sec of time leverage given to readers */

char output6[20];
int count6;
void reader6 (char *msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        output6[count6++]=msg;
        kprintf ("  %s: acquired lock, sleep 3s\n", msg);
        sleep (3);										
        output6[count6++]=msg;          
 	kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}                                                                                        
void writer6 (char *msg, int lck, int lprio)
{
        kprintf ("  %s: to acquire lock\n", msg);                                           
     	lock (lck, WRITE, lprio);
        output6[count6++]=msg;								
        kprintf ("  %s: acquired lock, sleep 3s\n", msg);
        sleep (3);      
        output6[count6++]=msg;
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test6(){
	int lck;
	int rd1, rd2, rd3, rd4, rd5;
	int wr;
	lck = lcreate();

        rd1 = create(reader6, 2000, 25, "reader6", 3, "A", lck, 20);
        rd2 = create(reader6, 2000, 25, "reader6", 3, "B", lck, 20);
        rd3 = create(reader6, 2000, 25, "reader6", 3, "C", lck, 20);
        rd4 = create(reader6, 2000, 25, "reader6", 3, "D", lck, 20);
        rd5 = create(reader6, 2000, 25, "reader6", 3, "E", lck, 20);
        wr = create(writer6, 2000, 25, "writer6", 3, "F", lck, 20);

	sleep(1);
	resume(rd1);
	resume(wr);
	kprintf("sleep for 0.4 sec..................\n");
	sleep10(4);
	resume(rd2);
	sleep10(1);
	resume(rd3);
	resume(rd4);
	resume(rd5);
	kprintf("output %s\n", output6);
	sleep(20);
}
#endif

#if 0
/************************************* Test 7 **********************************/
/*	checking lock delete */

void reader7 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
	sleep(5);
        kprintf ("  %s: to release lock\n", msg);
	ldelete(lck);
}

void writer7 (char *msg, int lck)
{
	int l;
        kprintf ("  %s: to acquire lock\n", msg);
	l = lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf("the processes lock wait return %d\n", proctab[currpid].plockret);
	if(l == OK){
	kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (5);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);}
	else if(l == DELETED) {
	kprintf("The lock was deleted\n");
	kprintf("the processes lock wait return %d\n", proctab[currpid].plockret); 
	}
	
}

void test7(){
	kprintf("in test 7*********\n");
	int lck;
	int rd, wr;
	lck = lcreate();
	rd = create(reader7, 2000, 25, "reader7", 2, "reader", lck);
        wr = create(writer7, 2000, 25, "writer7", 2, "writer", lck);
	resume(rd);
	sleep(1);
	resume(wr);
	sleep(10);
}

#endif

#if 0
/* Test 5: Delete a lock and processes waiting on that lock should return deleted*/
int reader5(char* msg, int lck){
        int     ret;

        kprintf ("  %s: to acquire lock1\n", msg);
        ret = lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock1\n", msg);
	sleep(1);
        kprintf ("  %s: to delete lock1\n", msg);
	ldelete(lck);	
}

int reader6(char* msg, int lck, int lck1){
        int     ret, ret1, ret2;

        kprintf ("  %s: to acquire lock 1\n", msg);
        ret = lock (lck, READ, DEFAULT_LOCK_PRIO);
        if(ret == OK){
                kprintf ("  %s: acquired lock 1\n", msg);
                kprintf ("  %s: to release lock 1\n", msg);
                releaseall (1, lck);
        }
        else if(ret == DELETED){
                kprintf ("Lock 1 %d deleted, not acquired\n");
        }
	else if(ret == SYSERR)
	{
		kprintf ("Lock 1 %d deleted, syserror, not acquired\n");
        }
	sleep(5);
	
	ret1 = lock(lck1, READ, DEFAULT_LOCK_PRIO);
	if(ret1 == OK){
                kprintf ("  %s: acquired lock 2\n", msg);
                kprintf ("  %s: to release lock 2\n", msg);
                releaseall (1, lck);
        }
        else if(ret1 == DELETED){
                kprintf ("Lock 2 %d deleted, not acquired\n");
        }
	else if(ret1 == SYSERR)
	{
		kprintf ("Lock 2 %d deleted, syserror, not acquired\n");
        }
	
	sleep(5);

        kprintf ("  %s: to acquire lock 1\n", msg);
        ret2 = lock (lck, READ, DEFAULT_LOCK_PRIO);
        if(ret2 == OK){
                kprintf ("  %s: acquired lock 1\n", msg);
                kprintf ("  %s: to release lock 1\n", msg);
                releaseall (1, lck);
        }
        else if(ret2 == DELETED){
                kprintf ("Lock 1 deleted, not acquired\n");
        }
	else if(ret2 == SYSERR)
	{
		kprintf ("Lock 1 deleted, syserror, not acquired\n");
        }

}

int writer5(char* msg, int lck){
        int ret;
        kprintf ("  %s: to acquire lock 1\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock 1, sleep 10s\n", msg);
        sleep (15);
	kprintf ("  %s: to release lock 1\n", msg);
        releaseall (1, lck);
	
}

void test5(){
        int lck = lcreate();
        
	kprintf("lock created: %d \n", lck);
	int rd1, rd2, rd3;
        int wr1;
	int lck1 = lcreate();
	
	kprintf("lock created: %d \n", lck1);
	rd1 = resume(create(reader5, 2000, 30, "reader5", 2, "reader A", lck));
        rd2 = resume(create(reader6, 2000, 30, "reader5", 3, "reader B", lck, lck1));
        
	resume(rd1);
	sleep(1);
	resume(rd2);
	sleep(1);
	lck = lcreate();
	kprintf("lock created: %d \n", lck);
	wr1 = resume(create(writer5, 2000, 30, "writer5", 2, "writer", lck));
        sleep(20);
}
#endif
int main()
{
        /* These test cases are only used for test purposes.
 *          * The provided results do not guarantee your correctness.
 *                   * You need to read the PA2 instruction carefully.
 *                            */

	kprintf("\t\t\t\t&&&&&&&&&&& TEST 1 &&&&&&&&&&&&&&&\n");
	test1();
	kprintf("\t\t\t\t&&&&&&&&&&& TEST 2 &&&&&&&&&&&&&&&\n");
	test2();
	kprintf("\t\t\t\t&&&&&&&&&&& TEST 3 &&&&&&&&&&&&&&&\n");
	test3();
//	test4();	
//	test6();
//	test7();
//	test5();
	// Aditional task 1
	task1(); 
/* The hook to shutdown QEMU for process-like execution of XINU.
 *          * This API call exists the QEMU process.
 *                   */
        shutdown();
}





