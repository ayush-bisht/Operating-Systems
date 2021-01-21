#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef NLOCK
#define NLOCK 50
#endif

#define READ 2
#define WRITE 3
#define NONE 4

#define LFREE 0
#define LUSED 1

struct lentry
{
	int lstate;    	//state
        int lqhead;	//head of queue
	int lqtail;	//tail of queue
	int ldesc;	//lock desc
	int ltype;	//type of lock, read or write
	int nr;         //no of readers holding the lock 
	int nw;		//writer in queue or holding the lock? = 1
        int pid_write;  //pid of writer
	int prio_write; //prio of writer
        int t_write;    //time that the writer tried to acquire the lock
	int lprio;	//maximum priority of a process waiting on the lock
};      

extern struct lentry locktab[];
extern int nextlock;

#define isbadlock(lock) (lock <0 || lock >= NLOCK)

void linit(void);
int lcreate(void);
int ldelete(int lockdescriptor);
int lock(int ldes1, int type, int priority);
int releaseall(int numlocks, int ldes1, ...);
void printlock(int lock);
void pinheritance(int lock);
void findmaxprio(int lock);
#endif

