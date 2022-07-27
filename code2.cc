
//This exercise show how to schedule threads with Rate Monotonic

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>


#define gettid() syscall(__NR_gettid)
#define SCHED_DEADLINE 3
/*use the proper syscall numbers */
#ifdef __x86_64__
#define __NR_sched_setattr      314
#define __NR_sched_getattr      315
#endif
#ifdef __i386__
#define __NR_sched_setattr      351
#define __NR_sched_getattr      352
#endif
#ifdef __arm__
#define __NR_sched_setattr      380
#define __NR_sched_getattr      381
#endif

//code of periodic tasks
void task1_code( );
void task2_code( );
void task3_code( );

//code of aperiodic tasks

//characteristic function of the thread, only for timing and synchronization
//periodic tasks
void *task1( void *);
void *task2( void *);
void *task3( void *);

int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags);
int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags);

struct sched_attr {
    __u32 size;
    __u32 sched_policy;
    __u64 sched_flags;
 /* SCHED_NORMAL, SCHED_BATCH */
    __s32 sched_nice;
 /* SCHED_FIFO, SCHED_RR */
    __u32 sched_priority;
/* SCHED_DEADLINE (nsec) */
    __u64 sched_runtime;
    __u64 sched_deadline;
    __u64 sched_period;
};
//aperiodic tasks

// initialization of mutexes and conditions (only for aperiodic scheduling)

#define INNERLOOP 1000
#define OUTERLOOP 2000

#define NPERIODICTASKS 3
#define NAPERIODICTASKS 0
#define NTASKS NPERIODICTASKS + NAPERIODICTASKS

long int periods[NTASKS];
struct timespec next_arrival_time[NTASKS];
double WCET[NTASKS];
pthread_attr_t attributes[NTASKS];
pthread_t thread_id[NTASKS];
struct sched_param parameters[NTASKS];
int missed_deadlines[NTASKS];
sched_attr attr[NTASKS];
main()
{
  	// set task periods in nanoseconds
	//the first task has period 100 millisecond
	//the second task has period 200 millisecond
	//the third task has period 400 millisecond
	//you can already order them according to their priority; 
	//if not, you will need to sort them
  	periods[0]= 100000000; //in nanoseconds
  	periods[1]= 200000000; //in nanoseconds
  	periods[2]= 400000000; //in nanoseconds

  	//for aperiodic tasks we set the period equals to 0

	//this is not strictly necessary, but it is convenient to
	//assign a name to the maximum and the minimum priotity in the
	//system. We call them priomin and priomax.

  	struct sched_param priomax;
  	priomax.sched_priority=sched_get_priority_max(SCHED_FIFO);
  	struct sched_param priomin;
  	priomin.sched_priority=sched_get_priority_min(SCHED_FIFO);

	// set the maximum priority to the current thread (you are required to be
  	// superuser). Check that the main thread is executed with superuser privileges
	// before doing anything else.

  	if (getuid() == 0)
    		pthread_setschedparam(pthread_self(),SCHED_FIFO,&priomax);

  	// execute all tasks in standalone modality in order to measure execution times
  	// (use gettimeofday). Use the computed values to update the worst case execution
  	// time of each task.

 	int i;
  	for (i =0; i < NTASKS; i++)
    	{

		// initializa time_1 and time_2 required to read the clock
		struct timespec time_1, time_2;
		clock_gettime(CLOCK_REALTIME, &time_1);

		//we should execute each task more than one for computing the WCET
		//periodic tasks
 	     	if (i==0)
			task1_code();
      		if (i==1)
			task2_code();
      		if (i==2)
			task3_code();
      		
      		//aperiodic tasks

		clock_gettime(CLOCK_REALTIME, &time_2);


		// compute the Worst Case Execution Time (in a real case, we should repeat this many times under
		//different conditions, in order to have reliable values

      		WCET[i]= 1000000000*(time_2.tv_sec - time_1.tv_sec)
			       +(time_2.tv_nsec-time_1.tv_nsec);
      		printf("\nWorst Case Execution Time %d=%f \n", i, WCET[i]);
    	}

    	// compute U
	double U = WCET[0]/periods[0]+WCET[1]/periods[1]+WCET[2]/periods[2];

    	// compute Ulub by considering the fact that we have harmonic relationships between periods
	double Ulub = 1;
    	
	//if there are no harmonic relationships, use the following formula instead
	//double Ulub = NPERIODICTASKS*(pow(2.0,(1.0/NPERIODICTASKS)) -1);
	
	//check the sufficient conditions: if they are not satisfied, exit  
  	if (U > Ulub)
    	{
      		printf("\n U=%lf Ulub=%lf Non schedulable Task Set", U, Ulub);
      		return(-1);
    	}
  	printf("\n U=%lf Ulub=%lf Scheduable Task Set", U, Ulub);
  	fflush(stdout);
  	sleep(5);

  	// set the minimum priority to the current thread: this is now required because 
	//we will assign higher priorities to periodic threads to be soon created
	//pthread_setschedparam

  	if (getuid() == 0)
    		pthread_setschedparam(pthread_self(),SCHED_FIFO,&priomin);

  
  	// set the attributes of each task, including scheduling policy and priority
  	

 	// aperiodic tasks

	//delare the variable to contain the return values of pthread_create	
  	int iret[NTASKS];

	//declare variables to read the current time
	struct timespec time_1;
	clock_gettime(CLOCK_REALTIME, &time_1);

  	// set the next arrival time for each task. This is not the beginning of the first
	// period, but the end of the first period and beginning of the next one. 
  	for (i = 0; i < NPERIODICTASKS; i++)
    	{
		// first we encode the current time in nanoseconds and add the period 
		long int next_arrival_nanoseconds = time_1.tv_sec*1000000000 + time_1.tv_nsec + periods[i];
		//then we compute the end of the first period and beginning of the next one
		next_arrival_time[i].tv_sec= next_arrival_nanoseconds/1000000000;
		next_arrival_time[i].tv_nsec= next_arrival_nanoseconds%1000000000;
       		missed_deadlines[i] = 0;
    	}

	

	// create all threads(pthread_create)
  	iret[0] = pthread_create( &(thread_id[0]),NULL, task1, NULL);
  	iret[1] = pthread_create( &(thread_id[1]),NULL, task2, NULL);
  	iret[2] = pthread_create( &(thread_id[2]),NULL, task3, NULL);

  	// join all threads (pthread_join)
  	pthread_join( thread_id[0], NULL);
  	pthread_join( thread_id[1], NULL);
  	pthread_join( thread_id[2], NULL);

  	// set the next arrival time for each task. This is not the beginning of the first
	// period, but the end of the first period and beginning of the next one. 
  	for (i = 0; i < NTASKS; i++)
    	{
      		printf ("\nMissed Deadlines Task %d=%d", i, missed_deadlines[i]);
		fflush(stdout);
    	}
  	exit(0);
}

int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags)
{
  return syscall(__NR_sched_setattr, pid, attr, flags);
}
int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags)
{
   return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

// application specific task_1 code

void task1_code()
{
	//print the id of the current task
  	printf(" 1[ "); fflush(stdout);

	//this double loop with random computation is only required to waste time
	int i,j;
	double uno;
  	for (i = 0; i < OUTERLOOP; i++)
    	{
      		for (j = 0; j < INNERLOOP; j++)
		{
			uno = rand()*rand()%10;
    		}
  	}

  	// when the random variable uno=0, then aperiodic task 5 must
  	// be executed
 
  	// when the random variable uno=1, then aperiodic task 5 must
  	// be executed
  
  	//print the id of the current task
  	printf(" ]1 "); fflush(stdout);
}

//thread code for task_1 (used only for temporization)
void *task1( void *ptr)
{
	// set thread affinity, that is the processor on which threads shall run
	cpu_set_t cset;
	CPU_ZERO (&cset);
	CPU_SET(0, &cset);
	unsigned int flags = 0;
	//pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);
        
	int i=0;
	
	attr[i].size = sizeof(attr[i]);
	attr[i].sched_flags = 0;
	attr[i].sched_priority = 0;
	attr[i].sched_nice = 0;
	attr[i].sched_policy = SCHED_DEADLINE;
	attr[i].sched_runtime = WCET[i]+100000000;
	attr[i].sched_deadline = periods[i];
	attr[i].sched_period = periods[i];

	int res = sched_setattr(0, &attr[i], flags);
	
	if(res != 0)
	{
	 perror("set SCHED_DEADLINE parameters");
	pthread_exit(NULL);
	}

   	//execute the task one hundred times... it should be an infinite loop (too dangerous)
  	int k=0;
  	for (k=0; k < 100; k++)
    	{
      		// execute application specific code
		task1_code();

		// it would be nice to check if we missed a deadline here... why don't
		// you try by yourself?

		// sleep until the end of the current period (which is also the start of the
		// new one
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[0], NULL);

		// the thread is ready and can compute the end of the current period for
		// the next iteration
 		
		long int next_arrival_nanoseconds = next_arrival_time[0].tv_sec*1000000000 + next_arrival_time[0].tv_nsec + periods[0];
		next_arrival_time[0].tv_sec= next_arrival_nanoseconds/1000000000;
		next_arrival_time[0].tv_nsec= next_arrival_nanoseconds%1000000000;
    	}
}

void task2_code()
{
	//print the id of the current task
  	printf(" 2[ "); fflush(stdout);
	int i,j;
	double uno;
  	for (i = 0; i < OUTERLOOP; i++)
    	{
      		for (j = 0; j < INNERLOOP; j++)
		{
			uno = rand()*rand()%10;
		}
    	}
	//print the id of the current task
  	printf(" ]2 "); fflush(stdout);
}


void *task2( void *ptr )
{
	// set thread affinity, that is the processor on which threads shall run
	cpu_set_t cset;
	CPU_ZERO (&cset);
	CPU_SET(0, &cset);
	unsigned int flags = 0;
	//pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);
	int i=1;
	
	attr[i].size = sizeof(attr[i]);
	attr[i].sched_flags = 0;
	attr[i].sched_priority = 0;
	attr[i].sched_nice = 0;
	attr[i].sched_policy = SCHED_DEADLINE;
	attr[i].sched_runtime = WCET[i]+100000000;
	attr[i].sched_deadline = periods[i];
	attr[i].sched_period = periods[i];

	int res = sched_setattr(0, &attr[i], flags);
	
	if(res != 0)
	{
	 perror("set SCHED_DEADLINE parameters");
	pthread_exit(NULL);
}

	int k=0;
  	for (k=0; k < 100; k++)
    	{
      		task2_code();

		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[1], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[1].tv_sec*1000000000 + next_arrival_time[1].tv_nsec + periods[1];
		next_arrival_time[1].tv_sec= next_arrival_nanoseconds/1000000000;
		next_arrival_time[1].tv_nsec= next_arrival_nanoseconds%1000000000;
    	}
}

void task3_code()
{
	//print the id of the current task
  	printf(" 3[ "); fflush(stdout);
	int i,j;
	double uno;
  	for (i = 0; i < OUTERLOOP; i++)
    	{
      		for (j = 0; j < INNERLOOP; j++);		
			double uno = rand()*rand()%10;
    	}
	//print the id of the current task
  	printf(" ]3 "); fflush(stdout);
}

void *task3( void *ptr)
{
	// set thread affinity, that is the processor on which threads shall run
	cpu_set_t cset;
	CPU_ZERO (&cset);
	CPU_SET(0, &cset);
	unsigned int flags = 0;
	//pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);
	int i=2;
	
	attr[i].size = sizeof(attr[i]);
	attr[i].sched_flags = 0;
	attr[i].sched_priority = 0;
	attr[i].sched_nice = 0;
	attr[i].sched_policy = SCHED_DEADLINE;
	attr[i].sched_runtime = WCET[i]+100000000;
	attr[i].sched_deadline = periods[i];
	attr[i].sched_period = periods[i];

	int res = sched_setattr(0, &attr[i], flags);
	
	if(res != 0)
	{
	 perror("set SCHED_DEADLINE parameters");
	pthread_exit(NULL);
}
	
	int k=0;
  	for (k=0; k < 100; k++)
    	{
      		task3_code();

		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[2], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[2].tv_sec*1000000000 + next_arrival_time[2].tv_nsec + periods[2];
		next_arrival_time[2].tv_sec= next_arrival_nanoseconds/1000000000;
		next_arrival_time[2].tv_nsec= next_arrival_nanoseconds%1000000000;
    }
}



