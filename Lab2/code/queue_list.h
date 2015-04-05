#ifndef _QUEUE_LIST_H  
#define _QUEUE_LIST_H  

#define ElementType void*

typedef enum State_type {READY=1, RUNNING, BLOCK, DONE, PREEMPT} State_type;

char * StateToString(State_type state);


typedef struct process_type
{
	unsigned int PID;

	unsigned int AT;
	unsigned int TC;
	unsigned int CB;
	unsigned int IO;
	State_type processState;

	unsigned int CB_current;
	unsigned int IO_current;
	unsigned int TC_remain;
	unsigned int timeStamp;
	
	unsigned int IT; //Total IO time
	unsigned int CW; //Total waiting time
}process_type, *process_ptr;

typedef struct event_type
{
	unsigned int createTime;
	unsigned int startTime;
	process_ptr process;
	State_type from;
	State_type to;

		
} event_type, *event_ptr;


struct Node;  
struct QNode;  
typedef struct Node *PtrToNode;  
typedef PtrToNode Queue;  
  
  
int IsEmpty( Queue Q );  
Queue CreateQueue( void );  
void DisposeQueue( Queue Q );  
void MakeEmpty( Queue Q );  
void Enqueue( ElementType X, Queue Q ); 

//Used for LCFS readyQueue
void Enqueue_Reverse(ElementType X, Queue Q);

//Forward-order insert node to a Queue, by the member provided
//Used for FCFS,SJF,RR readyQueue and eventQueue
//To get offset of the member for the argument, use:
//(int)(&((TYPE *)0)->MEMBER)
void Insert(ElementType X, Queue Q, int offset);

//Reverse-order insert node to a Queue, by the member provided
void Insert_Reverse(ElementType X, Queue Q, int offset);


ElementType Front( Queue Q );  
void Dequeue( Queue Q );  
ElementType FrontAndDequeue( Queue Q );  


#endif /* _QUEUE_LIST_H */ 


