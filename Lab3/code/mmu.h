#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fileIO.h"
#include "queue_list.h"

//macro definition for page table element
#define VIRTUAL_PAGE_NUM 64
#define PRESENT		(1<<31)
#define MODIFIED	(1<<30)
#define REFERENCED	(1<<29)
#define PAGEDOUT	(1<<28)
#define FLAGBITS	(PRESENT|MODIFIED|REFERENCED|PAGEDOUT)


//bit definition for option
#define OPTION_O 1
#define OPTION_P (1<<1)
#define OPTION_F (1<<2)
#define OPTION_S (1<<3)
#define OPTION_p (1<<4)
#define OPTION_f (1<<5)
#define OPTION_a (1<<6)

//Variable for each parameters
char option = 0;		//Option flag char
char aValue = 'l';		//Algorithm type
unsigned int fValue = 32;	//Length of physical frame

///////////* pageTable */////////////////
unsigned int pageTable [VIRTUAL_PAGE_NUM] = {0};


///////////* frameTable *///////////////
typedef struct physicalFrame_type
{
	int pageIndex; //Index: 0 - 63; Invalid: -1;

} frame_type, *frame_pointer;
frame_type * frameTable;

//////////* Global counters for SUM */////
unsigned long instCounter = 0;
unsigned long long totalcost = 0;
typedef struct stats_type
{
	unsigned int unmaps;
	unsigned int maps;
	unsigned int ins;
	unsigned int outs;
	unsigned int zeros;
}stats_type;


//////////* Function Declaration */////

//Get command line argument and create instance of pageReplacement();
int processArgument(int argc, char * argv[]);

//Initialize pageIndex of physical frames to -1
void initFrameTable(frame_type frameTable[]);

//Process instruction and call pageReplacement when needed.
void mmu (int mode, int pageIndex);

void printPageTable ();
void printFrameTable ();
void printSUM();

unsigned int (* pageReplacement) (int pageIndex);


//Return swapFrameOffset to be replaced
unsigned int NRU_virtual (int pageIndex);
unsigned int LRU_physical (int pageIndex);
unsigned int Random_physical (int pageIndex);
unsigned int FIFO_physical (int pageIndex);
unsigned int SecondChance_physical (int pageIndex);
unsigned int Clock_physical (int pageIndex);
unsigned int Clock_virtual (int pageIndex);
unsigned int Aging_physical (int pageIndex);
unsigned int Aging_virtual (int pageIndex);
