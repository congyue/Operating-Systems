#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "fileIO.h"
#include "queue_list.h"

typedef enum event 
{
	REQUEST = 0,
	FINISH
} eventType;

typedef struct event_type
{
	eventType event;
	int time;

	int requestTime;
	int requestCount;
	int requestTrack;
} event_type, *event_ptr;

typedef struct SUM_type
{
	int total_time;
	int tot_movement;
	double avg_turnaround;
	double avg_waittime;
	int max_waittime;
}SUM_type,*SUM_ptr;

//@return the pointer of next scheduled event
event_ptr (*ioscheduler) (Queue eventQueue, Queue requestQueue);
event_ptr IO_FIFO (Queue eventQueue, Queue requestQueue);
event_ptr IO_SSTF (Queue eventQueue, Queue requestQueue);
event_ptr IO_SCAN (Queue eventQueue, Queue requestQueue);
event_ptr IO_CSCAN (Queue eventQueue, Queue requestQueue);
event_ptr IO_FSCAN (Queue eventQueue, Queue requestQueue);



int processArgument(int argc, char * argv[]);
void EnqueueByTime(event_ptr event, Queue eventQueue);

int initEventQueue(Queue eventQueue, FILE * inputFile);
void DES_run(Queue eventQueue, Queue requestQueue);

