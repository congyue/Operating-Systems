#ifndef _QUEUE_LIST_H  
#define _QUEUE_LIST_H  

#define ElementType void*

//struct Node;  
//struct QNode;

typedef struct QNode  
{  
  ElementType Element;  
  struct QNode *Next;  
}QNode, *QNodePtr;  
  
struct Node {  
  QNodePtr Front;  
  QNodePtr Rear;  
};  

typedef struct QNode *PtrToQNode; 
typedef struct Node *PtrToNode;  
typedef PtrToNode Queue;  
  
  
int IsEmpty( Queue Q );  
Queue CreateQueue( void );  
void DisposeQueue( Queue Q );  
void MakeEmpty( Queue Q );  
void Enqueue( ElementType X, Queue Q ); 

//Add element to first place
void Enqueue_Reverse(ElementType X, Queue Q);

ElementType Front( Queue Q );  
void Dequeue( Queue Q );  
ElementType FrontAndDequeue( Queue Q );  


#endif /* _QUEUE_LIST_H */ 


