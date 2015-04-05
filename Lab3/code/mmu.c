#include "mmu.h"

//Create instance for stats
stats_type stats =
{
	.unmaps = 0,
	.maps = 0,
	.ins = 0,
	.outs = 0,
	.zeros = 0,
};

int main (int argc, char * argv[])
{
	char * token;
	int instMode, instPage;
	processArgument(argc, argv);
	
	nextRand(randFile); //ignore counter line
	frameTable = (frame_type *) malloc(fValue*sizeof(frame_type));
	initFrameTable(frameTable);
	
	while ((token = nextToken(inputFile)) != NULL)
	{
		instMode = atoi(token);

		if ((token = nextToken(inputFile)) != NULL)
			instPage = atoi(token);
		else
		{
			printf("Input format wrong!");
			exit(1);
		}

			
		if (option & OPTION_O)
			printf("==> inst: %d %d\n", instMode, instPage);

		mmu(instMode,instPage);

		if(option & OPTION_p)
			printPageTable();
		if(option & OPTION_f)
			printFrameTable();
		instCounter++;	
	}

	if (option & OPTION_P)
		printPageTable();
	if (option & OPTION_F)
		printFrameTable();
	if (option & OPTION_S)
		printSUM();
}

void mmu (int mode, int pageIndex)
{
	static unsigned int frameTableOffset = 0;
       	unsigned int swapFrameOffset = 0;
	if (pageIndex > VIRTUAL_PAGE_NUM)
	{
		printf("Page index %d exceed maximum page number\n", pageIndex);
		exit(1);
	}

	//Access elements
	totalcost +=1;

	if (!(pageTable[pageIndex] & PRESENT))		//page not exist, need zero/swap in
	{
		if (frameTableOffset < fValue)		//still have free frames to use
		{
			pageTable[pageIndex] |= (frameTableOffset & (~FLAGBITS)); //set virtual->physical map to lower 28 bits
	
			//Some pageReplacement algorithm need maintenance reference order
			pageReplacement(pageIndex);

			frameTable[frameTableOffset].pageIndex = pageIndex; //set inverse map
			if (option & OPTION_O)
			{
				printf("%lu: ZERO       %2d\n", instCounter, frameTableOffset);
				printf("%lu: MAP    %2d  %2d\n", instCounter, pageIndex, pageTable[pageIndex]);
			}
			stats.zeros++;
			totalcost += 150;

			stats.maps++;
			totalcost += 400;
			frameTableOffset++;

		}
		else					//no free frames, call pageReplacement() to get swap frame
		{
			//Get swapFrameOffset from pageReplacement();
			swapFrameOffset = pageReplacement(pageIndex);

			if (option & OPTION_O)
				printf("%lu: UNMAP  %2d  %2d\n", instCounter, 
						frameTable[swapFrameOffset].pageIndex, swapFrameOffset);
			stats.unmaps++;
			totalcost += 400;

			//Swap out virtual page if modified
			if (pageTable[frameTable[swapFrameOffset].pageIndex] & MODIFIED)
			{
				if (option & OPTION_O)
					printf("%lu: OUT    %2d  %2d\n", instCounter, 
							frameTable[swapFrameOffset].pageIndex, swapFrameOffset);

				stats.outs++;
				totalcost += 3000;
				pageTable[frameTable[swapFrameOffset].pageIndex] |= PAGEDOUT;
			}
			//Unmapping virtual page to frame, clear all bits except PAGEDOUT
			pageTable[frameTable[swapFrameOffset].pageIndex] &= PAGEDOUT;
		
			//Set inverse pageIndex invalid
			frameTable[swapFrameOffset].pageIndex = -1;

			//Need swap in the new frame
			if (pageTable[pageIndex] & PAGEDOUT) 		
			{
				if (option & OPTION_O)
					printf("%lu: IN     %2d  %2d\n", instCounter, pageIndex, swapFrameOffset);

				stats.ins++;
				totalcost += 3000;
			}
			
			//Need ZERO the new frame
			else	
			{
				if (option & OPTION_O)
					printf("%lu: ZERO       %2d\n", instCounter, swapFrameOffset);

				stats.zeros++;
				totalcost += 150;
				frameTable[swapFrameOffset].pageIndex = 0;
			}
		
			
			if (option & OPTION_O)
				printf("%lu: MAP    %2d  %2d\n", instCounter, pageIndex, swapFrameOffset);
			stats.maps++;
			totalcost += 400;

			frameTable[swapFrameOffset].pageIndex = pageIndex;    //Map pageIndex to frame 
			pageTable[pageIndex] |= (swapFrameOffset & ~FLAGBITS); //Map frameOffset to page
		}

		//page PRESENT after mapping
		pageTable[pageIndex] |= PRESENT;
	}
	else
		pageReplacement(pageIndex);

	pageTable[pageIndex] |= REFERENCED;
	if (mode == 1)
		pageTable[pageIndex] |= MODIFIED;
	

}

void printFrameTable ()
{
	int offset;
	for (offset=0; offset<fValue; offset++)
	{
		if (frameTable[offset].pageIndex < 0)
			printf("* ");
		else
			printf("%d ", frameTable[offset].pageIndex);
	}
	printf("\n");
}

void printPageTable ()
{
	int i;
	char bitR, bitM, bitS;
	for (i=0; i<VIRTUAL_PAGE_NUM; i++)
	{
		if (pageTable[i] & PRESENT)
		{
			bitR = (pageTable[i] & REFERENCED ? 'R' : '-');
			bitM = (pageTable[i] & MODIFIED ? 'M' : '-');
			bitS = (pageTable[i] & PAGEDOUT ? 'S' : '-');
			printf("%d:%c%c%c ", i, bitR, bitM, bitS);
		}
		else if (pageTable[i] & PAGEDOUT)
			printf("# ");
		else
			printf("* ");

	}
	printf("\n");
}

void printSUM()
{
	printf("SUM %lu U=%d M=%d I=%d O=%d Z=%d ===> %llu\n",
		instCounter, stats.unmaps, stats.maps, stats.ins, stats.outs, stats.zeros, totalcost);	
}

void initFrameTable(frame_type frameTable[])
{
	int offset;
	for (offset=0; offset<fValue; offset++)
		frameTable[offset].pageIndex = -1; //negative value means not mapped to virtual page

}

int processArgument(int argc, char * argv[])
{
	int arg;
	char * inputPath, * randPath;

	while ((arg = getopt (argc, argv, "a:o:f:")) != -1)
		switch (arg)
		{
			case 'a':
				if (strlen(optarg)>1)
				{
					printf("[Error] Invalid algorithm!\n");
					exit(1);
				}
				aValue = *optarg;
				break;
			case 'o':
				while(*optarg != '\0')
				{
					if (*optarg == 'O')
						option |= OPTION_O;
					else if (*optarg == 'P')
						option |= OPTION_P;
					else if (*optarg == 'F')
						option |= OPTION_F;
					else if (*optarg == 'S')
						option |= OPTION_S;
					else if (*optarg == 'p')
						option |= OPTION_p;
					else if (*optarg == 'f')
						option |= OPTION_f;
					else if (*optarg == 'a')
						option |= OPTION_a;
					else
					{
						printf("[Error] Invalid option!\n");
						exit(1);
					}
					optarg++;	
				}
				break;
			case 'f':
				fValue = atoi(optarg);
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

	if(inputPath == NULL || randPath == NULL)
	{
		printf("[Error] Input/Random file not specified!\n");
		exit(1);
	}
	inputFile = readFile(inputPath);
	randFile = readFile(randPath);

	//Create instance for pageReplacement()
	switch(aValue)
	{
		case 'N': pageReplacement = NRU_virtual; break;
		case 'l': pageReplacement = LRU_physical; break;
		case 'r': pageReplacement = Random_physical; break;
		case 'f': pageReplacement = FIFO_physical; break;
		case 's': pageReplacement = SecondChance_physical; break;
		case 'c': pageReplacement = Clock_physical; break;
		case 'C': pageReplacement = Clock_virtual; break;
		case 'a': pageReplacement = Aging_physical; break;
		case 'A': pageReplacement = Aging_virtual; break;
		default: pageReplacement = LRU_physical; break;
	}

	return 0;
}

unsigned int NRU_virtual (int pageIndex)
{
	static int callingTimes = 1;
	unsigned int i=0, selidx = 0, idx0 =0, idx1 = 0, idx2 = 0, idx3 = 0;
	unsigned int selectPage, pageClassTable [4][VIRTUAL_PAGE_NUM];

	//No need to process if page exist or frame not full
	if (pageTable[pageIndex] & PRESENT || frameTable[fValue-1].pageIndex<0)
		return 0;

	if (callingTimes == 10 && option & OPTION_a)
		printf(" @@ reset NRU refbits after walking PTE\n");

	for (i=0; i<VIRTUAL_PAGE_NUM; i++)
	{
		//ignore this turn if page not PRESENT
		if (!(pageTable[i] & PRESENT))
			continue;

		if (pageTable[i] & REFERENCED)
		{
			if (pageTable[i] & MODIFIED)
				pageClassTable[3][idx3++] = i;
			else
				pageClassTable[2][idx2++] = i;
		}
		else
		{
			if (pageTable[i] & MODIFIED)
				pageClassTable[1][idx1++] = i;
			else
				pageClassTable[0][idx0++] = i;
		}
	}

	if (idx0 != 0)
	{
		selidx = atoi(nextRand(randFile))%idx0;
		selectPage = pageClassTable[0][selidx];
		if (option & OPTION_a)
		{
			printf(" @@ lowest_class=0: ");
			printf("selidx=%d from ", selidx);
			for (i=0; i< idx0; i++)
				printf("%u ", pageClassTable[0][i]);
			printf("\n");
		}		
	}

	else if (idx1 != 0)
	{
		selidx = atoi(nextRand(randFile))%idx1;
		selectPage = pageClassTable[1][selidx] & ~FLAGBITS;
		if (option & OPTION_a)
		{
			printf(" @@ lowest_class=1: ");
			printf("selidx=%d from ", selidx);
			for (i=0; i< idx1; i++)
				printf("%u ", pageClassTable[1][i]);
			printf("\n");
		}	
	}

	else if (idx2 != 0)
	{
		selidx = atoi(nextRand(randFile))%idx2;
		selectPage = pageClassTable[2][selidx] & ~FLAGBITS;
		if (option & OPTION_a)
		{
			printf(" @@ lowest_class=2: ");
			printf("selidx=%d from ", selidx);
			for (i=0; i< idx2; i++)
				printf("%u ", pageClassTable[2][i]);
			printf("\n");
		}
	}

	else if (idx3 != 0)
	{
		selidx = atoi(nextRand(randFile))%idx3;
		selectPage = pageClassTable[3][selidx] & ~FLAGBITS;
		if (option & OPTION_a)
		{
			printf(" @@ lowest_class=3: ");
			printf("selidx=%d from ", selidx);
			for (i=0; i< idx3; i++)
				printf("%u ", pageClassTable[3][i]);
			printf("\n");
		}
	}
	else
	{
		if (option & OPTION_a)
			printf("[Error] Counting NRU class table error!\n");
		exit(1);
	}

	//Clear all REFERENCED bits
	if(callingTimes == 10)
	{
		for (i=0; i<VIRTUAL_PAGE_NUM; i++)
			if (pageTable[i] & PRESENT)
				pageTable[i] &= ~REFERENCED;
		callingTimes = 0;
	}	

	callingTimes++;

	return pageTable[selectPage] & ~FLAGBITS;
}

unsigned int LRU_physical (int pageIndex)
{
	//Linked list store frameIndex: 0 - (fValue-1)
	static Queue orderList;
	PtrToQNode orderListElement, orderListElementNext;
	unsigned int i, selectFrame = 0, frameIndex;

	if (orderList == NULL)
		orderList = CreateQueue();

	//If page exist, change the page to the head of orderList
	if (pageTable[pageIndex] & PRESENT)
	{
		orderListElement = orderList -> Front;
		orderListElementNext = orderListElement -> Next;
		//find currently referenced element
		while (orderListElement -> Next != NULL)
		{
			frameIndex = pageTable[pageIndex] & ~FLAGBITS;
			//printf("Comparing %u and %u ...\n", orderListElementNext -> Element, frameIndex);
			if ( orderListElementNext -> Element == frameIndex)
				break;
			orderListElement = orderListElementNext;
			orderListElementNext = orderListElement -> Next;
		}
		//Not found element
		if (orderListElement -> Next == NULL)
		{
			printf("[Error] Present page cannot be found in orderList!\n");
			exit(1);
		}
		//Changing order
		orderListElement -> Next = orderListElementNext -> Next;
		Enqueue_Reverse (orderListElementNext -> Element, orderList);
		free(orderListElementNext);
	}

	//if page not exist and frame available, add to the head of orderList 
	else if (frameTable[fValue-1].pageIndex<0)
	{
		frameIndex = pageTable[pageIndex] & ~FLAGBITS;
		Enqueue_Reverse (frameIndex , orderList);
	}
	//if page not exist and frame is full, move tail element to the first place, and return that element
	else
	{
		orderListElement = orderList -> Front -> Next;
		orderListElementNext = orderListElement -> Next;
		while (orderListElementNext -> Next != NULL)
		{
			orderListElement = orderListElementNext;
			orderListElementNext = orderListElement -> Next;
		}
		selectFrame = (orderListElementNext -> Element);
		Enqueue_Reverse (selectFrame, orderList);
		orderListElement -> Next = NULL;
		free (orderListElementNext);
	}
	/*
	printf("Current physical orderList: ");
	orderListElement = orderList -> Front -> Next;
	while (orderListElement != NULL)
	{
		printf("%d ", orderListElement -> Element);
		orderListElement = orderListElement -> Next;
	}
	printf("\n");*/
	
	return selectFrame;
}

unsigned int Random_physical (int pageIndex)
{
	//No need to process if page exist or frame not full
	if (pageTable[pageIndex] & PRESENT || frameTable[fValue-1].pageIndex<0)
		return 0;

	return atoi(nextRand(randFile))%fValue;
	
}

unsigned int FIFO_physical (int pageIndex)
{
	//Linked list store frameIndex: 0 - (fValue-1)
	static Queue FIFOList;
	PtrToQNode FIFOListElement;
	unsigned int i, selectFrame = 0, frameIndex;

	//No need to process if page exist or frame not full
	if (pageTable[pageIndex] & PRESENT || frameTable[fValue-1].pageIndex<0)
		return 0;

	//Copy frameIndexes to FIFOList
	if (FIFOList == NULL)
	{
		FIFOList = CreateQueue();
		for (i=0; i<fValue; i++)
		{
		
			frameIndex = pageTable[frameTable[i].pageIndex] & ~FLAGBITS;
			Enqueue(frameIndex, FIFOList);
		}
	}
	
	//Remove head frame for swap, then add the same element to tail
	selectFrame = FrontAndDequeue(FIFOList);
	Enqueue(selectFrame, FIFOList);
	
	/*
	printf("Current physical FIFOList: ");
	FIFOListElement = FIFOList -> Front -> Next;
	while (FIFOListElement != NULL)
	{
		printf("%d ", FIFOListElement -> Element);
		FIFOListElement = FIFOListElement -> Next;
	}
	printf("\n");
	printf("Return %u!\n",selectFrame);*/

	return selectFrame;	
}

unsigned int SecondChance_physical (int pageIndex)
{
	//Linked list store frameIndex: 0 - (fValue-1)
	static Queue FIFOList;
	PtrToQNode FIFOListElement;
	unsigned int i, selectFrame = 0, frameIndex, pageNotReferenced;

	//No need to process if page exist or frame not full
	if (pageTable[pageIndex] & PRESENT || frameTable[fValue-1].pageIndex<0)
		return 0;

	//Copy frameIndexes to FIFOList
	if (FIFOList == NULL)
	{
		FIFOList = CreateQueue();
		for (i=0; i<fValue; i++)
		{
		
			frameIndex = pageTable[frameTable[i].pageIndex] & ~FLAGBITS;
			Enqueue(frameIndex, FIFOList);
		}
	}
	
	//Repeat to remove head and add to tail, until we find element not REFERENCED
	while(1)
	{
		selectFrame = FrontAndDequeue(FIFOList);
		Enqueue(selectFrame, FIFOList);

		pageNotReferenced = pageTable[frameTable[selectFrame].pageIndex];
		//printf("pageNotReferenced = %X\n",pageNotReferenced);
		if (!(pageNotReferenced & REFERENCED))
			break;

		//Clear REFERENCED bit and give second chance
		pageTable[frameTable[selectFrame].pageIndex] &= ~REFERENCED;
	}
	
	/*
	printf("Current physical FIFOList: ");
	FIFOListElement = FIFOList -> Front -> Next;
	while (FIFOListElement != NULL)
	{
		printf("%d ", FIFOListElement -> Element);
		FIFOListElement = FIFOListElement -> Next;
	}
	printf("\n");
	printf("Return %u!\n",selectFrame);*/

	return selectFrame;	
}

unsigned int Clock_physical (int pageIndex)
{
	return SecondChance_physical (pageIndex);
}

unsigned int Clock_virtual (int pageIndex)
{
	//Linked list store virtualIndex: 0 - (fValue-1)
	static Queue FIFOList;
	static PtrToQNode FIFOListElement = 0;
	PtrToQNode FIFOListElementPrint;
	unsigned int i, selectFrame = 0, frameIndex, pageNotReferenced;

	//No need to process if page exist or frame not full
	if (pageTable[pageIndex] & PRESENT || frameTable[fValue-1].pageIndex<0)
		return 0;

	//Map pageTable to FIFOList
	if (FIFOList == NULL)
	{
		FIFOList = CreateQueue();
		for (i=0; i<VIRTUAL_PAGE_NUM; i++)
			Enqueue(i, FIFOList);

		//Create circular linked list
		FIFOList -> Rear -> Next = FIFOList -> Front -> Next;
		FIFOListElement = FIFOList -> Front -> Next;
	}

	//Repeat to remove virtual page head and add to tail, until we find element not REFERENCED
	while(1)
	{		
		if (pageTable[FIFOListElement -> Element] & PRESENT)
			if (!(pageTable[FIFOListElement -> Element] & REFERENCED))
				break;

		pageTable[FIFOListElement -> Element] &= ~REFERENCED;
		FIFOListElement = FIFOListElement -> Next;
	}
	selectFrame = pageTable[FIFOListElement -> Element] & ~FLAGBITS;
	FIFOListElement = FIFOListElement -> Next;
		
	/*
	printf("Current virtual clock FIFOList: ");
	FIFOListElementPrint = FIFOListElement;
	for (i=0; i<VIRTUAL_PAGE_NUM; i++)
	{
		printf("%d ", FIFOListElementPrint -> Element);
		FIFOListElementPrint = FIFOListElementPrint -> Next;
	}
	printf("\n");
	printf("Return page %u frame %u!\n", FIFOListElement -> Element, selectFrame);*/
	return selectFrame;	
}

unsigned int Aging_physical (int pageIndex)
{
	static unsigned int *ageVector;
	unsigned int i, selectFrame = 0, referenceBit, minimumAge = -1;
	//No need to process if page exist or frame not full
	if (pageTable[pageIndex] & PRESENT || frameTable[fValue-1].pageIndex<0)
		return 0;

	if (ageVector == NULL)
		ageVector = (unsigned int *) malloc (fValue*sizeof(int)); 

	for (i=0; i<fValue; i++)
	{
		referenceBit = (pageTable[frameTable[i].pageIndex] & REFERENCED) << 2;
		ageVector[i] >>= 1;		
		ageVector[i] |= referenceBit;
		pageTable[frameTable[i].pageIndex] &= ~REFERENCED;

		if (ageVector[i] < minimumAge)
		{
			minimumAge = ageVector[i];
			selectFrame = pageTable[frameTable[i].pageIndex] & ~FLAGBITS;
		}

		if (option & OPTION_a)
			printf("%d:%X ",i,ageVector[i]);
			
	}

	//clear swap frame aging
	ageVector[selectFrame] = 0;
	if (option & OPTION_a)
		printf("\nReturn frame %d!\n",selectFrame);

	return selectFrame;
	
}

unsigned int Aging_virtual (int pageIndex)
{
	static unsigned int *ageVector;
	unsigned int i, selectFrame = 0, selectPage = 0, referenceBit, minimumAge = -1;

	//No need to process if page exist or frame not full
	if (pageTable[pageIndex] & PRESENT || frameTable[fValue-1].pageIndex<0)
		return 0;
	
	if (ageVector == NULL)
		ageVector = (unsigned int *) malloc (VIRTUAL_PAGE_NUM*sizeof(int));

	for (i=0; i<VIRTUAL_PAGE_NUM; i++)
	{
		if (pageTable[i] & PRESENT)
		{
			referenceBit = (pageTable[i] & REFERENCED) << 2;
			ageVector[i] >>= 1;		
			ageVector[i] |= referenceBit;
			pageTable[i] &= ~REFERENCED;

			if (ageVector[i] < minimumAge)
			{
				minimumAge = ageVector[i];
				selectPage = i;
			}

			if (option & OPTION_a)
				printf("%d:%X ",i,ageVector[i]);
		}		
	}

	//clear swap frame aging
	ageVector[selectPage] = 0;

	selectFrame = pageTable[selectPage] & ~FLAGBITS;

	if (option & OPTION_a)
		printf("\nReturn frame %d!\n",selectFrame);


	return selectFrame;

	
}
