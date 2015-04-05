#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "queue_list.h"

#define MAX_LINE 512
//Open file locate in path and return FILE pointer
FILE * readFile(char * path);

//Process input file and return next token
char * nextToken(FILE * inputFile);
//Process rfile and return next random number
char * nextRand(FILE * inputFile);

int myrandom (int burst);

void initEventQueue(Queue eventQueue, FILE * inputFile);

void printEvent(event_ptr event);

//Create a new event in DES loop
//@param (startTime, process, current state, next state)
event_ptr DES_createEvent (process_ptr process, int startTime, State_type from, State_type to);

//Pick next event from eventQueue in DES loop
event_ptr DES_getEvent(Queue eventQueue);

//Add event to eventQueue in DES loop
void DES_putEvent(event_ptr event, Queue eventQueue);

//Process current event and return next event
void processEvent(event_ptr event);

void addToReady_RR(process_ptr process, Queue readyQueue);
void addToReady_SJF(process_ptr process, Queue readyQueue);
void addToReady_LCFS(process_ptr process, Queue readyQueue);

typedef struct scheduler_type
{
	Queue readyQueue;
	char idle;
	unsigned int IO_until;
	unsigned int quantum;

	//Pick task from readyQueue and running, then return next event
	//@param (readyQueue)
	event_ptr (*schedule)(Queue);

	//Add process to readyQueue
	//@param (readyQueue)
	void (*addToReady)(process_ptr, Queue);

	//Communicate with DES, handle current event and put next event to eventQueue
	//@param (event*)
	void (*processEvent)(event_ptr);

}scheduler_type, *scheduler_ptr;

scheduler_ptr createRRScheduler(Queue readyQueue, unsigned int quantum);
scheduler_ptr createFCFSScheduler(Queue readyQueue);
scheduler_ptr createLCFSScheduler(Queue readyQueue);
scheduler_ptr createSJFScheduler(Queue readyQueue);

event_ptr schedule_RR (Queue readyQueue);

void DES_run(Queue eventQueue, Queue readyQueue, FILE * randFile);

typedef struct summary_type
{
	unsigned int processNum;
	unsigned int lastFinishTime;
	double CPU_utilization;
	double IO_utilization;
	double average_turnaround;
	double average_waiting;
	double throughput;

}summary_type;

