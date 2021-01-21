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

void lprocA(char *msg, int lck){
        int i;
        kprintf ("process %s: to acquire lock with lock id: %d\n", msg, lck);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("process %s: acquired lock with lock id: %d\n", msg, lck);
        kprintf ("process %s: Going to sleep while holding the lock for 2s\n", msg);
        sleep(2);
        kprintf("process %s: resuming\n", msg);
        for(i =0; i< 50; i++)   kprintf("%s", msg);
        kprintf("\n");
        kprintf ("process %s: to release lock with lock id: \n", msg, lck);
        releaseall (1, lck);
        kprintf ("returning to procces %s after the lock was released to do the remaining work\n", msg);
        for(i =0; i< 50; i++)   kprintf("%s", msg);
        kprintf("\n");
}


void lprocB(char *msg){
        kprintf("process %s: starting\n", msg);
        int i = 0, count = 0;
        for(i = 0; i< 50; i++)  kprintf("%s", msg);
        kprintf("\n");
        kprintf("process %s: going to sleep for 2s\n", msg);
        sleep(2);
        kprintf("process %s: resuming\n", msg);
        for(i =0; i< 50; i++)   kprintf("%s", msg);
        kprintf("\n");
        kprintf("process %s: ending\n", msg);
}

void lprocC(char *msg, int lck){
        int i = 0;
        kprintf ("process %s: to acquire lock with lock id: %d\n", msg, lck);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("process %s: acquired lock with lock id: %d\n", msg, lck);
        for(i = 0; i< 50; i++)  kprintf("%s", msg);
        kprintf("\n");
        kprintf ("process %s: to release lock with lock id: \n", msg, lck);
        releaseall (1, lck);
}

void sprocA(char *msg, int sem){
        int i;
        kprintf ("process %s: to acquire sem with sem id: %d\n", msg, sem);
        wait(sem);
        kprintf ("process %s: acquired sem with sem id: %d\n", msg, sem);
        kprintf ("process %s: Going to sleep while holding the sem for 2s\n", msg);
        sleep(2);
        kprintf("process %s: resuming\n", msg);
        for(i =0; i< 50; i++)   kprintf("%s", msg);
        kprintf("\n");
        kprintf ("process %s: to release sem with sem id: \n", msg, sem);
        signal(sem);
        kprintf ("returning to procces %s after the sem was released to do the remaining work\n", msg);
        for(i =0; i< 50; i++)   kprintf("%s", msg);
        kprintf("\n");
}

void sprocC(char *msg, int sem){
        int i = 0;
        kprintf ("process %s: to acquire sem with sem id: %d\n", msg, sem);
        wait(sem);
        kprintf ("process %s: acquired lock with lock id: %d\n", msg, sem);
        for(i = 0; i< 50; i++)  kprintf("%s", msg);
        kprintf("\n");
        kprintf ("process %s: to release lock with lock id: \n", msg, sem);
        signal(sem);
}


void locktest(){

        int     lck;
        int     procA, procB, procC;

        kprintf("Testing priority inversion problem with reader/writer lock system\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test with lock failed");

        procA = create(lprocA, 2000, 25, "processA", 2, "A", lck);
        procB = create(lprocB, 2000, 30, "processB", 1, "B");
        procC = create(lprocC, 2000, 40, "processC", 2, "C", lck);

        resume(procA);
        resume(procB);
        resume(procC);

        sleep(6);
        ldelete(lck);
}

void semtest(){
        int sem;
        int procA, procB, procC;

        kprintf("Testing priority inversion problem with existing XINU semaphore\n");
        sem = screate(1);
        assert (sem != SYSERR, "Test with semaphore failed");

        procA = create(sprocA, 2000, 25, "processA", 2, "A", sem);
        procB = create(lprocB, 2000, 30, "processB", 1, "B");
        procC = create(sprocC, 2000, 40, "processC", 2, "C", sem);

        resume(procA);
        resume(procB);
        resume(procC);

        sleep(6);
        sdelete(sem);
}

int task1(){

        locktest();
        kprintf("\n\n");
        semtest();
	
	shutdown();
}
