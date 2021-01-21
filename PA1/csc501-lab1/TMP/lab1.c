#include "lab1.h"


int schedule_class;

void setschedclass(int sched_class)
{
	if(sched_class < 2)
		schedule_class = RANDOMSCHED;
	else
		schedule_class = LINUXSCHED;	

}

int getschedclass()
{
	return schedule_class;
}

