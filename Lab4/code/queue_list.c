#include "queue_list.h"  
#include "fatal.h"  
  
  
int  
IsEmpty( Queue Q )  
{  
  return Q->Front == Q->Rear;  
}  
  
Queue  
CreateQueue( void )  
{  
  Queue Q;  
  Q = malloc( sizeof( struct Node ) );  
  Q->Front = Q->Rear = malloc( sizeof( struct QNode ) );  
  if (!Q->Front)  
    FatalError( "Out of space!!!");  
  Q->Front->Next = NULL;  
  return Q;  
}  
  
void  
MakeEmpty( Queue Q )  
{  
  if( Q == NULL )  
    Error( "Must use CreateQueue first" );  
  else  
    while( !IsEmpty( Q ) )  
      Dequeue( Q );  
}  
  
void  
DisposeQueue( Queue Q )  
{  
  while( Q->Front ) {  
    Q->Rear = Q->Front->Next;  
    free( Q->Front );  
    Q->Front = Q->Rear;  
  }  
 // printf( "\nDispose queue completed!!!" );  
}  
void  
Enqueue( ElementType X, Queue Q )  
{  
  QNodePtr p;  
  p = malloc( sizeof( QNode ) );  
  if (!p)  
    FatalError( "Out of space!!!" );  
  p->Element = X;  
  p->Next = NULL;  
  Q->Rear->Next = p;  
  Q->Rear = p;  
}  
  
ElementType  
Front( Queue Q )  
{  
  if ( !IsEmpty( Q ) )  
    return Q->Front->Next->Element;  
  return 0; /* Return value used to avoid warning */  
}  
  
void  
Dequeue( Queue Q )  
{  
  if ( !IsEmpty( Q ) )  
  {  
    QNodePtr p;  
    p = malloc( sizeof( QNode ) );  
    if (!p)  
      FatalError( "Out of space!!!" );  
    p = Q->Front->Next;  
    Q->Front->Next = p->Next;  
    if ( Q->Rear == p )  
      Q->Rear = Q->Front;  
    free( p );  
  }  
}  
  
ElementType  
FrontAndDequeue( Queue Q )  
{  
  if ( !IsEmpty( Q ) )  
  {  
    QNodePtr p;  
   /* p = malloc( sizeof( QNode ) );  
    if (!p)  
      FatalError( "Out of space!!!" );  */
    p = Q->Front->Next;  
    ElementType temp = 0;  
    temp = p->Element;  
    Q->Front->Next = p->Next;  
    if ( Q->Rear == p )  
      Q->Rear = Q->Front;  
    free( p );  
    return temp;  
  }  
  //Error( "Empty queue!!!" );  
  return 0; /* Return value used to avoid warning */  
}



void Enqueue_Reverse(ElementType X, Queue Q)
{
	QNodePtr p,q,r;
	ElementType *address, *address_cmp;
	if (IsEmpty(Q))
	{
		//printf("Add to last ...\n");
		Enqueue(X, Q);
	}
	else
	{
		q = Q->Front; //q -> head node
		r = q->Next; //r -> first node
		p = malloc( sizeof( QNode ) ); //p -> new node  
		if (!p)  
			FatalError( "Out of space!!!" );
		p->Element = X;
		//printf("Add to first ...\n");
		p->Next = r;
		q->Next = p;
	}
	return;

}
			
