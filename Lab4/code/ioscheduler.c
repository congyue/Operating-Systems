#include"ioscheduler.h"

#define DEBUG 0

SUM_ptr summary;

Queue eventQueue;
Queue requestQueue;

int track = 0, time = 0, requestCount = 0, finishedRequest = 0;

int main(int argc, char * argv[])
{
	
	processArgument(argc, argv);
	summary = (SUM_ptr) malloc (sizeof(SUM_type));	
	eventQueue = CreateQueue();
	requestQueue = CreateQueue();

	initEventQueue(eventQueue, inputFile);
	if (DEBUG)
		printf("TRACE\n");
	DES_run(eventQueue, requestQueue);
	

	return 0;
}

void DES_run(Queue eventQueue, Queue requestQueue)
{
	event_ptr event,nextEvent;
	int IOReady = 1;
	
	while(!(IsEmpty(eventQueue) && IsEmpty(requestQueue)))
	{
		event = (event_ptr)FrontAndDequeue(eventQueue);
		time = event -> time;
		if(event -> event == REQUEST)
		{
			event -> requestCount = requestCount;
			event -> requestTime = time;
			Enqueue(event,requestQueue);
			if(DEBUG)
				printf("%d:     %d add %d\n",time,
						event -> requestCount,event->requestTrack);
			requestCount++;
		}
		else if(event -> event == FINISH)
		{
			track = event -> requestTrack;
			if(DEBUG)
				printf("%d:     %d finish %d\n",time,
						event -> requestCount,time - event -> requestTime);
			summary -> avg_turnaround += time - event -> requestTime;
			free(event);
			IOReady = 1;			
		}
	
		if(IOReady)
		{
			nextEvent = ioscheduler(eventQueue, requestQueue);
			
			if(nextEvent != NULL)
			{
				IOReady = 0;
				summary -> tot_movement += abs(track - nextEvent -> requestTrack);
				summary -> avg_waittime += (time - nextEvent -> requestTime);
				if(time - nextEvent -> requestTime > summary -> max_waittime)
					summary -> max_waittime = time - nextEvent -> requestTime;

				if(DEBUG)
					printf("%d:     %d issue %d %d\n",time,nextEvent->requestCount,
							nextEvent -> requestTrack, track);

				nextEvent -> time = time + abs(nextEvent->requestTrack - track);
				nextEvent -> event = FINISH;
				EnqueueByTime(nextEvent, eventQueue);
			}
		}
	}
	summary -> total_time = time;
	summary -> avg_turnaround /= (double)requestCount;
	summary -> avg_waittime /= (double)requestCount;
	printf("SUM: %d %d %.2lf %.2lf %d\n", summary -> total_time,
			summary->tot_movement, summary->avg_turnaround, 
			summary->avg_waittime, summary->max_waittime);
	
}

int initEventQueue(Queue eventQueue, FILE * inputFile)
{
	char * token;
	event_ptr newEvent;
	while ((token = nextToken(inputFile)) != NULL)
	{
		newEvent = (event_ptr) malloc (sizeof(event_type));
		newEvent -> event = REQUEST;
		newEvent -> time = atoi(token);
		if ((token = nextToken(inputFile)) != NULL)
			newEvent -> requestTrack = atoi(token);
		else
		{
			printf("Input format wrong!");
			exit(1);
		}
		Enqueue(newEvent, eventQueue);
	}

	return 0;
}

int processArgument(int argc, char * argv[])
{
	int arg;
	char * inputPath, *sValue;

	while ((arg = getopt (argc, argv, "s:")) != -1)
		switch (arg)
		{
			case 's':
				sValue = optarg;
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

	if(inputPath == NULL)
	{
		printf("[Error] Input file not specified!\n");
		exit(1);
	}
	inputFile = readFile(inputPath);

	//Create instance for ioscheduler()
	switch(sValue[0])
	{
		case 'f': ioscheduler = IO_FIFO; break;
		case 's': ioscheduler = IO_SSTF; break;
		case 'S': ioscheduler = IO_SCAN; break;
		case 'C': ioscheduler = IO_CSCAN; break;
		case 'F': ioscheduler = IO_FSCAN; break;
		default: break;
	}

	return 0;
}

event_ptr IO_FIFO (Queue eventQueue, Queue requestQueue)
{
	event_ptr event = (event_ptr) FrontAndDequeue(requestQueue);
	return event;	
}

event_ptr IO_SSTF (Queue eventQueue, Queue requestQueue)
{
	event_ptr event = NULL;
	QNodePtr currentNode, nextNode, selectNode;
	unsigned int distance, minimum = -1;

	currentNode = requestQueue -> Front -> Next;
	if (currentNode == NULL)		
		return NULL;

	selectNode = currentNode;
	nextNode = currentNode -> Next;


	//Find shortest distance node
	while(currentNode != NULL)
	{
		distance = abs(((event_ptr)(currentNode -> Element)) -> requestTrack -track);
	
		if (distance < minimum)
		{
			minimum = distance;
			selectNode = currentNode;
			event = (event_ptr)(selectNode -> Element);
		}
		currentNode = currentNode -> Next;
	}

	//Remove the select node and return
	currentNode = requestQueue -> Front;
	nextNode = currentNode -> Next;
	while(nextNode != NULL)
	{
		if (nextNode == selectNode)
		{
			currentNode -> Next = nextNode -> Next;
			if (nextNode -> Next == NULL)
				requestQueue -> Rear = currentNode;
			break;
		}
		currentNode = currentNode -> Next;
		nextNode = currentNode -> Next;
	}
	return event;
}

event_ptr IO_SCAN (Queue eventQueue, Queue requestQueue)
{
	event_ptr event = NULL;
	QNodePtr currentNode, nextNode, selectNode;
	//direction 1: head to higher track; 0: head to lower track
	static int direction = 1;
	int distance;
	unsigned int minimum;

	//printf("Current direction: %d\n", direction);

	//Return NULL if nothing can be scheduled
	if (IsEmpty(requestQueue))
		return NULL;
	//Return head element if only one element exist
	else if (requestQueue -> Front -> Next -> Next == NULL)
		return (event_ptr)FrontAndDequeue(requestQueue); 

	//Find nearest element in current direction
	while(event == NULL)
	{	
		distance = 0;
		minimum = -1;
		currentNode = requestQueue -> Front -> Next;
		selectNode = NULL;
		if (direction)
		{
			while(currentNode != NULL)
			{
				distance = ((event_ptr)(currentNode->Element))->requestTrack - track;
				if (distance >= 0 && distance < minimum)
				{
					minimum = distance;
					selectNode = currentNode;
					/*printf("Consider track: %d  ",
						((event_ptr)(selectNode -> Element))->requestTrack);
					printf("Distance is %d\n", distance);	*/	
				}
				currentNode = currentNode -> Next;
			}

			//Found next element
			if (selectNode != NULL)
				event = (event_ptr)(selectNode -> Element);
			//No element in current direction, change to another direction
			else
				direction = 0;	

		}

		else
		{
			while(currentNode != NULL)
			{
				distance = ((event_ptr)(currentNode->Element))->requestTrack - track;
				if (distance <= 0 && abs(distance) < minimum)
				{
					minimum = abs(distance);
					selectNode = currentNode;
					/*printf("Consider track: %d\n",
						((event_ptr)(selectNode->Element))->requestTrack);*/
				}
				currentNode = currentNode -> Next;
			}

			//Found next element
			if (selectNode != NULL)
				event = (event_ptr)(selectNode -> Element);
			//No element in current direction, change to another direction
			else
				direction = 1;
		}

		//Remove the select node and return
		currentNode = requestQueue -> Front;
		nextNode = currentNode -> Next;
		while(nextNode != NULL)
		{
			if (nextNode == selectNode)
			{
				currentNode -> Next = nextNode -> Next;
				if (nextNode -> Next == NULL)
					requestQueue -> Rear = currentNode;
				break;
			}
			currentNode = currentNode -> Next;
			nextNode = currentNode -> Next;
		}
	}
	//printf("Current track: %d, next track: %d\n", track, event -> requestTrack);
	return event;
}

event_ptr IO_CSCAN (Queue eventQueue, Queue requestQueue)
{
	event_ptr event = NULL;
	QNodePtr currentNode, nextNode, selectNode;
	int distance, comparedTrack = track;
	unsigned int minimum;

	//printf("Current direction: %d\n", direction);

	//Return NULL if nothing can be scheduled
	if (IsEmpty(requestQueue))
		return NULL;
	//Return head element if only one element exist
	else if (requestQueue -> Front -> Next -> Next == NULL)
		event = (event_ptr)FrontAndDequeue(requestQueue); 

	//Find nearest element in current direction
	while(event == NULL)
	{	
		distance = 0;
		minimum = -1;
		currentNode = requestQueue -> Front -> Next;
		selectNode = NULL;
		while(currentNode != NULL)
		{
			distance = ((event_ptr)(currentNode->Element))->requestTrack - comparedTrack;
			if (distance >= 0 && distance < minimum)
			{
				minimum = distance;
				selectNode = currentNode;
				/*printf("Consider track: %d  ",
				  ((event_ptr)(selectNode -> Element))->requestTrack);
				  printf("Distance is %d\n", distance);	*/	
			}
			currentNode = currentNode -> Next;
		}

		//Found next element
		if (selectNode != NULL)
			event = (event_ptr)(selectNode -> Element);
		//No element in current direction, reset current track to beginning
		else
			comparedTrack = 0;
	}


		//Remove the select node and return
		currentNode = requestQueue -> Front;
		nextNode = currentNode -> Next;
		while(nextNode != NULL)
		{
			if (nextNode == selectNode)
			{
				currentNode -> Next = nextNode -> Next;
				if (nextNode -> Next == NULL)
					requestQueue -> Rear = currentNode;
				break;
			}
			currentNode = currentNode -> Next;
			nextNode = currentNode -> Next;
		}
	//printf("Current track: %d, next track: %d\n", track, event -> requestTrack);
	return event;
}

event_ptr IO_FSCAN (Queue eventQueue, Queue requestQueue)
{
	//Additional queue for FSCAN
	static Queue secondQueue;
	event_ptr event = NULL;
	QNodePtr currentNode, nextNode, selectNode;
	//direction 1: head to higher track; 0: head to lower track
	static int direction = 1;
	int distance;
	unsigned int minimum;

	if (secondQueue == NULL)
		secondQueue = CreateQueue();

	if (IsEmpty(secondQueue))
	{
		//Return NULL if nothing can be scheduled
		if (IsEmpty(requestQueue))
			return NULL;
		//Return head element if only one element exist
		else if (requestQueue -> Front -> Next -> Next == NULL)
			return (event_ptr)FrontAndDequeue(requestQueue); 
		
		//Move all elements from requestQueue to secondQueue
		while (!IsEmpty(requestQueue))
			Enqueue(FrontAndDequeue(requestQueue), secondQueue);
		//Reset direction when secondQueue is refreshed
		direction = 1;
	}

	//Find nearest element in current direction
	while(event == NULL)
	{	
		distance = 0;
		minimum = -1;
		currentNode = secondQueue -> Front -> Next;
		selectNode = NULL;
		if (direction)
		{
			while(currentNode != NULL)
			{
				distance = ((event_ptr)(currentNode->Element))->requestTrack - track;
				if (distance >= 0 && distance < minimum)
				{
					minimum = distance;
					selectNode = currentNode;
					/*printf("Consider track: %d  ",
						((event_ptr)(selectNode -> Element))->requestTrack);
					printf("Distance is %d\n", distance);*/		
				}
				currentNode = currentNode -> Next;
			}

			//Found next element
			if (selectNode != NULL)
				event = (event_ptr)(selectNode -> Element);
			//No element in current direction, change to another direction
			else
				direction = 0;	

		}

		else
		{
			while(currentNode != NULL)
			{
				distance = ((event_ptr)(currentNode->Element))->requestTrack - track;
				if (distance <= 0 && abs(distance) < minimum)
				{
					minimum = abs(distance);
					selectNode = currentNode;
					/*printf("Consider track: %d\n",
						((event_ptr)(selectNode->Element))->requestTrack);*/
				}
				currentNode = currentNode -> Next;
			}

			//Found next element
			if (selectNode != NULL)
				event = (event_ptr)(selectNode -> Element);
			//No element in current direction, change to another direction
			else
				direction = 1;
		}

		//Remove the select node and return
		currentNode = secondQueue -> Front;
		nextNode = currentNode -> Next;
		while(nextNode != NULL)
		{
			if (nextNode == selectNode)
			{
				currentNode -> Next = nextNode -> Next;
				if (nextNode -> Next == NULL)
					secondQueue -> Rear = currentNode;
				break;
			}
			currentNode = currentNode -> Next;
			nextNode = currentNode -> Next;
		}
	}
	//printf("Current track: %d, next track: %d\n", track, event -> requestTrack);
	return event;
}

void EnqueueByTime(event_ptr event, Queue eventQueue)
{
	QNodePtr currentNode, nextNode, insertNode;
	event_ptr currentEvent, nextEvent;

	currentNode = eventQueue -> Front;
	nextNode = currentNode -> Next;

	if(event->time < time)
	{
		printf("[Error] Insert event time less than current time!\n");
		exit(1);
	}

	//Directly append if queue is empty
	if (IsEmpty(eventQueue))
	{
		//printf("Append %d!\n", event->time);
		Enqueue(event, eventQueue);
		return;
	}

	insertNode = (QNodePtr) malloc(sizeof(QNode));
	insertNode -> Element = event;
	
	while(nextNode != NULL)
	{
		currentEvent = (event_ptr) currentNode -> Element;
		nextEvent = (event_ptr) nextNode -> Element;
		//Insert to head if insert node time is less than all nodes
		if (currentEvent!= NULL && event -> time < currentEvent -> time )
		{
			eventQueue -> Front -> Next = insertNode;
			insertNode -> Next = currentNode;
			/*printf("Insert %d before %d\n", 
						((event_ptr)(insertNode -> Element)) ->time, 
						((event_ptr)(insertNode -> Next -> Element)) ->time);*/
			return;
		}
		//Insert to queue following ascending order
		else
		{
			if (event -> time < nextEvent -> time)
			{
				currentNode -> Next = insertNode;
				insertNode -> Next = nextNode;
				/*printf("Insert %d before %d\n", 
						((event_ptr)(insertNode -> Element)) ->time, 
						((event_ptr)(insertNode -> Next -> Element)) ->time);*/
				return;
			}
		}
		currentNode = currentNode -> Next;
		nextNode = currentNode -> Next;
	}

	//Append to tail if time is larger than all nodes
	if(nextNode == NULL)
	{
		Enqueue(event, eventQueue);
		/*printf("Append %d!\n", 
				((event_ptr)(insertNode -> Element)) ->time);*/
	}

	
}
