#include "scheduler.h" //Header, macro, function delearation and structure definition 
#define MAX_OUTPUT 4096
#define MAX_BUF 64

unsigned int currentTime = 0, i = 0;
int linenum = 1, lineoffset = 0, randLinenum = 2, randLineoffset = 0;
short verbose = 0;

FILE * inputFile, * randFile;
scheduler_ptr scheduler = NULL;

summary_type summaryInfo;
char output[MAX_OUTPUT][MAX_BUF];

Queue readyQueue;
Queue eventQueue;

int main (int argc, char * argv[])
{
	char * inputPath, * randPath;
	
	char svalue[20];
	int index, arg;
	int offset;

	readyQueue = CreateQueue();
	eventQueue = CreateQueue();
	
  	opterr = 1;
     
	while ((arg = getopt (argc, argv, "vs:")) != -1)
         switch (arg)
	 {
           case 'v':
             verbose = 1;
             break;
           case 's':
             strcpy(svalue,optarg);
             break;
           case '?':
             if (optopt == 's')
               fprintf (stderr, "Option -%c requires an argument.\n", optopt);
             else if (isprint (optopt))
               fprintf (stderr, "Unknown option `-%c'.\n", optopt);
             else
               fprintf (stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
             return 1;
           default:
             abort ();
           
	 }

       inputPath = argv[optind++];
       randPath = argv[optind];

       inputFile = readFile(inputPath);
       randFile = readFile(randPath);
       nextRand(randFile);//ignore counter line
       
       initEventQueue(eventQueue, inputFile);

       switch (svalue[0])
	{
		case 'F':
			scheduler = createFCFSScheduler(readyQueue);
			strcat(output[0], "FCFS\n");
			break;
		case 'L':
			scheduler = createLCFSScheduler(readyQueue);
			strcat(output[0], "LCFS\n");
			break;
		case 'S':
			 scheduler = createSJFScheduler(readyQueue);
			 strcat(output[0], "SJF\n");	 
			 break;
		case 'R':
			 scheduler = createRRScheduler(readyQueue, atoi(svalue+1));
			 strcat(output[0], "RR "); 
			 strcat(output[0], svalue+1);
			 strcat(output[0], "\n");
			 break;
		default:strcat(output[0], "Operand not recognize!\n");
			printf("%s",output[0]);
			return;
			 break;
	}

       DES_run(eventQueue, readyQueue, randFile);
       while(output[i][0])
	       printf("%s",output[i++]);

       summaryInfo.lastFinishTime = currentTime;
       summaryInfo.CPU_utilization /= (double)currentTime;
       summaryInfo.CPU_utilization *= 100;
       summaryInfo.IO_utilization /= (double)currentTime;
       summaryInfo.IO_utilization *= 100;
       summaryInfo.average_turnaround /= (double)summaryInfo.processNum;
       summaryInfo.average_waiting /= (double)summaryInfo.processNum;
       summaryInfo.throughput = 100.0* (double)summaryInfo.processNum/(double)currentTime;
       
       printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
		       summaryInfo.lastFinishTime,
		       summaryInfo.CPU_utilization,
		       summaryInfo.IO_utilization,
		       summaryInfo.average_turnaround,
		       summaryInfo.average_waiting,
		       summaryInfo.throughput);
       return 0;
				
}
void DES_run(Queue eventQueue, Queue readyQueue, FILE * randFile)
{
	event_ptr event,nextEvent;
	process_ptr process;
	
	while(!(IsEmpty(eventQueue) && IsEmpty(scheduler->readyQueue)))
	{
		event = DES_getEvent(eventQueue);
		if(verbose)		
			printEvent(event);

		currentTime = event -> startTime;			
		scheduler->processEvent(event);

		//invoke scheduler if no event at current time
		nextEvent = Front(eventQueue);
		if (nextEvent == NULL || nextEvent -> startTime != currentTime)
		{
			if(scheduler->idle)
			DES_putEvent (scheduler->schedule(scheduler->readyQueue), eventQueue);
			
		}
		free(event);
	}
}

void printEvent(event_ptr event)
{
	process_ptr process;
	process = event->process;
	printf("\n==> %d %d ts=%d %s dur=%d\n",event -> startTime, process -> PID, event -> createTime, 
			StateToString(process -> processState), event -> startTime - event -> createTime);

	printf("T(%d:%d): %s -> %s",process -> PID, event -> startTime, 
			StateToString(event -> from), StateToString(event -> to));
	if (event -> to == RUNNING || event -> process -> processState == PREEMPT)
		printf("  cb=%d rem=%d\n", event -> process -> CB_current, event -> process -> TC_remain);
	else if(event -> to == BLOCK)
		printf("  ib=%d rem=%d\n", event -> process -> IO_current, event -> process -> TC_remain);
	else
		printf("\n");
}

char * StateToString(State_type state)
{
	switch(state)
	{
		case READY: return "READY"; break;
		case RUNNING: return "RUNNING"; break;
		case BLOCK: return "BLOCK"; break;
		case DONE: return "DONE"; break;
		case PREEMPT: return "PREEMPT"; break;
		default: return NULL;
	}
}


void initEventQueue(Queue eventQueue, FILE * inputFile)
{
	char * token = NULL;
	char step = 0; //4 steps to create process: 0-AT, 1-TC, 2-CB, 3-IO
	unsigned int PID = 0;
	process_ptr newProcess;
	event_ptr newEvent;

	while(token = nextToken(inputFile))
	{
		switch (step)
		{
			case 0:
				newProcess = (process_ptr)malloc(sizeof(process_type));
				newProcess -> PID = PID++;
				newProcess -> AT = atoi(token);
				newProcess -> IT = 0;
				newProcess -> CW = 0;
				step++;			
				break;
			case 1:
				newProcess -> TC = atoi(token);
				step++;
				break;
			case 2:
				newProcess -> CB = atoi(token);
				step++;
				break;
			case 3:
				newProcess -> IO = atoi(token);
				newProcess -> TC_remain = newProcess -> TC;
				newProcess -> processState = READY;
				step = 0;
				/*
				printf("Create process%d: %d %d %d %d\n",newProcess -> PID, newProcess -> AT, 
			newProcess -> TC, newProcess -> CB, newProcess -> IO);*/
				newEvent = (event_ptr)malloc(sizeof(event_type));
				newEvent -> createTime = newProcess -> AT;
				newEvent -> startTime = newProcess -> AT;
				newEvent -> process = newProcess;
				newEvent -> from = READY;
				newEvent -> to = READY;
				Enqueue(newEvent, eventQueue);

				break;
		}	

		free(token);
	}
	
	

	


}


FILE * readFile(char * path)
{
	FILE * inputFile;
	if ((inputFile = fopen(path, "r")) == NULL)
	{
		printf("Error:Failed to open input file!\n");
		exit(1);
	}
	return inputFile;

}

char * nextToken(FILE * inputFile)
{
	char buf[MAX_LINE];
	int index = 0;
	char letter = 0;
	char * token = NULL;
	static lastTokenLen = 0;

	if (lastTokenLen)
	{
		lineoffset += lastTokenLen;
		lastTokenLen = 0;
	}

	while (!feof(inputFile))
	{
		letter = fgetc(inputFile);
		lineoffset++;

		if (letter == ' ' || letter == '\t')
		{
			continue;
		}
		else if (letter == '\n')
		{
			if (fgetc(inputFile) == EOF)
			{
				//printf("enter with EOF\n");
				return NULL;
			}
			else
				fseek(inputFile, -1, SEEK_CUR);

			linenum++;
			lineoffset = 0;
		}
		else
		{		
			buf[index++] = letter;
			while (!feof(inputFile))
			{
				letter = fgetc(inputFile);
				
				if (letter == ' '||letter == '\t'||letter == '\n' ||letter == EOF)
				{
					fseek(inputFile, -1, SEEK_CUR);
					buf[index] = '\0';
					token = (char *) malloc(strlen(buf));
					strcpy(token, buf);
					return token;
				}

				buf[index++] = letter;
				lastTokenLen++;
			}
		}
	
	}
	return NULL;
}


char * nextRand(FILE * inputFile)
{
	char buf[MAX_LINE];
	int index = 0;
	char letter = 0;
	char * token = NULL;
	static lastTokenLen = 0;

	if (lastTokenLen)
	{
		randLineoffset += lastTokenLen;
		lastTokenLen = 0;
	}

	while (!feof(inputFile))
	{
		letter = fgetc(inputFile);
		randLineoffset++;

		if (letter == ' ' || letter == '\t')
		{
			continue;
		}
		else if (letter == '\n')
		{
			if (fgetc(inputFile) == EOF)
			{
				rewind(inputFile);
				while(fgetc(inputFile)!='\n');//ignore counter line
				randLineoffset = 0;
				randLinenum = 2;
				continue;		
			}
			else
				fseek(inputFile, -1, SEEK_CUR);

			randLinenum++;
			randLineoffset = 0;
		}
		else
		{		
			buf[index++] = letter;
			while (!feof(inputFile))
			{
				letter = fgetc(inputFile);
				
				if (letter == ' '||letter == '\t'||letter == '\n' ||letter == EOF)
				{
					fseek(inputFile, -1, SEEK_CUR);
					buf[index] = '\0';
					token = (char *) malloc(strlen(buf));
					strcpy(token, buf);
					return token;
				}

				buf[index++] = letter;
				lastTokenLen++;
			}
		}
	
	}
	return NULL;
}

scheduler_ptr createRRScheduler(Queue readyQueue, unsigned int quantum)
{
	scheduler_ptr scheduler = (scheduler_ptr) malloc(sizeof(scheduler_type));
	scheduler -> readyQueue = readyQueue;
	scheduler -> idle = 1;
	scheduler -> processEvent = processEvent;
	scheduler -> quantum = quantum;

	scheduler -> schedule = schedule_RR;
	scheduler -> addToReady = addToReady_RR;
	return scheduler;

}

scheduler_ptr createFCFSScheduler(Queue readyQueue)
{
	return createRRScheduler(readyQueue, -1);
}

scheduler_ptr createLCFSScheduler(Queue readyQueue)
{
	scheduler_ptr scheduler = (scheduler_ptr) malloc(sizeof(scheduler_type));
	scheduler -> readyQueue = readyQueue;
	scheduler -> idle = 1;
	scheduler -> processEvent = processEvent;
	scheduler -> quantum = -1;

	scheduler -> schedule = schedule_RR;
	scheduler -> addToReady = addToReady_LCFS;
	return scheduler;

}

scheduler_ptr createSJFScheduler(Queue readyQueue)
{
	scheduler_ptr scheduler = (scheduler_ptr) malloc(sizeof(scheduler_type));
	scheduler -> readyQueue = readyQueue;
	scheduler -> idle = 1;
	scheduler -> processEvent = processEvent;
	scheduler -> quantum = -1;
		
	scheduler -> schedule = schedule_RR;
	scheduler -> addToReady = addToReady_SJF;

	return scheduler;

}

int myrandom (int burst)
{
	return 1 + (atoi(nextRand(randFile))%burst);
}

event_ptr DES_createEvent (process_ptr process, int startTime, State_type from, State_type to)
{
	event_ptr newEvent = (event_ptr) malloc(sizeof(event_type));
	newEvent -> createTime = process -> timeStamp;
	newEvent -> process = process;
	newEvent -> startTime = startTime;
	newEvent -> from = from;
	newEvent -> to = to;

	return newEvent;
}

event_ptr DES_getEvent(Queue eventQueue)
{
	event_ptr newEvent = (event_ptr) malloc(sizeof(event_type));
	newEvent = (event_ptr)FrontAndDequeue(eventQueue);
	return newEvent;
}

void DES_putEvent(event_ptr event, Queue eventQueue)
{
	if (event != NULL)
		Insert(event, eventQueue, (unsigned long)(&((event_ptr)0)->startTime));
}

void processEvent(event_ptr event)
{	
	static unsigned int IOfrom = 0, IOto = 0;
	event_ptr result = NULL;
	//Mark the process is processed at this time
	event -> process -> timeStamp = currentTime;
	switch(event -> process -> processState)
	{
		//READY -> RUNNING
		case READY:
			scheduler -> addToReady(event->process, scheduler->readyQueue);	
			break;
		//BLOCK -> READY
		case BLOCK:
			//add IO time to final summary
			if (currentTime > IOto)
			{
				
				IOfrom = currentTime;
				IOto = currentTime + event->process->IO_current;
				summaryInfo.IO_utilization += IOto - IOfrom;
				IOfrom = IOto;
			}
			else if (currentTime + event->process->IO_current > IOto)
			{
		 		IOto = currentTime + event->process->IO_current;
				summaryInfo.IO_utilization += IOto - IOfrom;
				IOfrom = IOto;
			}
			
			//add CPU time to final summary
			summaryInfo.CPU_utilization += currentTime - event -> createTime;

			scheduler->idle = 1;
			event -> process -> processState = READY;
			event -> process -> CB_current= 0;
			event -> process -> IT += event->process->IO_current;
			result = DES_createEvent (event -> process,
					event->startTime + event->process->IO_current, 
									BLOCK, READY);
			DES_putEvent(result,eventQueue);
			break;
		case RUNNING:
			//RUNNING -> READY
			if (event->process->CB_current > scheduler->quantum)
			{
				event -> process -> processState = PREEMPT;
				event -> process -> TC_remain -= scheduler->quantum;
				event->process->CB_current -= scheduler->quantum;
				result = DES_createEvent (event -> process,
					event->startTime + scheduler->quantum, 
									RUNNING, READY);
				DES_putEvent(result,eventQueue);
			}
			//RUNNING -> BLOCK
			else if (event->process->CB_current < event->process->TC_remain)
			{
				event -> process -> processState = BLOCK;
				event -> process -> TC_remain -= event->process->CB_current;
				event -> process -> IO_current = 
					myrandom(event -> process -> IO);
				result = DES_createEvent (event -> process, 
					event->startTime + event->process->CB_current, 
									RUNNING, BLOCK);
				DES_putEvent(result,eventQueue);
			}
			//RUNNING -> DONE
			else
			{
				event -> process -> processState = DONE;
				result = DES_createEvent (event -> process, 
					event->startTime + event->process->TC_remain, 
									RUNNING, DONE); 
				DES_putEvent(result,eventQueue);
			}
			break;

		//READY -> RUNNING
		case PREEMPT:
			//add CPU time to final summary
			summaryInfo.CPU_utilization += currentTime - event -> createTime;

			scheduler->idle = 1;
			event -> process -> processState = READY;
			scheduler -> addToReady(event -> process, scheduler->readyQueue);
			break;
		//RUNNING -> DONE, print info to output buffer
		case DONE:
			//add CPU time to final summary
			summaryInfo.CPU_utilization += currentTime - event -> createTime;

			scheduler->idle = 1;
			snprintf(output[event->process->PID+1], sizeof(output[0]), 
					"%04d: %4d %4d %4d %4d | %4d %4d %4d %4d\n", 
					event->process->PID,
					event->process->AT,
					event->process->TC,
					event->process->CB,
					event->process->IO,
					currentTime,
					currentTime - event->process->AT,
					event->process->IT,
					event->process->CW
					);
			summaryInfo.processNum++;
			summaryInfo.average_turnaround += currentTime - event->process->AT;
			summaryInfo.average_waiting += event->process->CW;
			break;
		default:
			break;
		
	}
}

void addToReady_RR(process_ptr process, Queue readyQueue)
{
	if (process != NULL)
		Enqueue(process, readyQueue);
		//Insert(process, readyQueue, (unsigned long)(&((process_ptr)0)->AT));
}

void addToReady_SJF(process_ptr process, Queue readyQueue)
{
	if (process != NULL)
		Insert(process, readyQueue, (unsigned long)(&((process_ptr)0)->TC_remain));
}

void addToReady_LCFS(process_ptr process, Queue readyQueue)
{
	if (process != NULL)
		Enqueue_Reverse(process, readyQueue);
}

event_ptr schedule_RR (Queue readyQueue)
{
	event_ptr nextEvent = NULL;
	//pick out first ready process from readyQueue
	process_ptr process = FrontAndDequeue (readyQueue);
	
	if(process == NULL)
		return NULL;

	scheduler -> idle = 0;
	unsigned int randomCB = 0;
	
	if (process -> CB_current!=0) //No need to get random number
		randomCB = process -> CB_current;
	else //calculate CPU burst by using random number
		randomCB = myrandom (process -> CB);

	if (randomCB < process -> TC_remain)
		process -> CB_current = randomCB;
	else
		process -> CB_current = process -> TC_remain;
	
	nextEvent = (event_ptr) malloc(sizeof(event_type));
	nextEvent -> createTime = process -> timeStamp;
	nextEvent -> startTime = currentTime;
	nextEvent -> process = process;
	nextEvent -> process -> CW += currentTime - process -> timeStamp;
	nextEvent -> process -> processState = RUNNING;
	nextEvent -> from = READY;
	nextEvent -> to = RUNNING;
	return nextEvent;
}

