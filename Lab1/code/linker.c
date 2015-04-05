#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAX_LINE 2048
#define MAX_MMap 2048
#define MAX_Symbol 256
#define Machine_Size 512
#define MAX_Module Machine_Size
#define MAX_Name 16
#define MAX_Def 16
#define MAX_Use MAX_Def

#define DUPLICATE 1<<1
#define OVERFLOW 1<<2

#define OVERFLOW_A 1<<1
#define OVERFLOW_R OVERFLOW
#define OVERFLOW_E 1<<3
#define NOT_DEF 1<<4
#define ILLEGAL_I 1<<6
#define ILLEGAL_O 1<<7

void __parseerror(int errcode);
void __semanticerror(int errorcode);
FILE * readFile(char * path);

//Print symbol table and parse error 
void printPass1 ();

//Print memory map and semantic error
void printPass2 ();

//Process input file and return next token
char * nextToken(FILE * inputFile);

//Process current token and change to next state
int processToken ();

//Process module and generate memory map
int processModule ();

//pass1 implementation
int pass1(FILE * inputFile);

//pass2 implementation
int pass2(FILE * inputFile);

int isNum(char * str);
int isSym(char * str);
int isAddr(char * str);
int isEOF(char * str);

int linenum = 1, lineoffset = 0;

enum tokenType {DEFNUM=1, DEFSYM, DEFVAL, USENUM, USESYM, ADDRSIZE, ADDRTYPE, ADDRNUM};

//symbolt table
typedef struct symbolType 
{
	char name[MAX_Name+1];
	int value;
	unsigned int exception;
	int defmodule;
	int original;
	int used;
} symbolType;

symbolType * symbolTable [MAX_Symbol];

//memory map
typedef struct memoryType
{
	int value;
	unsigned int exception;
	char * missSymbol;
} memoryType;

memoryType * memoryMap [MAX_MMap];

//instruction structure to record instruction
typedef struct instrType
{
	char type;
	char instr[10]; 
}instrType;

//structure for each module
typedef struct moduleType
{
	int number;
	int address;

	int defNum;
	int useNum;
	int size;

	symbolType defList[MAX_Def];
	char useList[MAX_Use][MAX_Name+1];
	instrType instrList[Machine_Size];

} moduleType;

moduleType * moduleTable [MAX_Module];

//structure to record unused symbols in uselist
typedef struct notUsedType
{
	int module;
	char symbol [MAX_Name];
} notUsedType;
notUsedType * notUsedByModule [MAX_Symbol];



int main(int argc, char * argv[])
{
	char * path;
	FILE * inputFile;
	
	if (argc != 1)
		path = argv[1];
	
	inputFile = readFile(path);

	if (!pass1(inputFile))
		return 0;

	if (!pass2(inputFile))
		return 0;
	
	return 1;
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

//Status change as: DEFNUM, DEFSYM, DEFVAL, USENUM, USESYM, ADDRSIZE, ADDRTYPE, ADDRNUM
int processToken(char * token)
{
	static enum tokenType expect = DEFNUM;
	static int moduleNum = 0, defListIdx = 0, useListIdx = 0, instrListIdx = 0;
	static int instrCount = 0, symbolTableIdx = 0, duplicateFlag = 0;
	int i=0;

	//Initialize module and assign number of definition list
	if (expect == DEFNUM)
	{
		if (isEOF(token))
			return 1;

		if (!isNum(token))
		{
			__parseerror(0);
			exit(1);
		}

		if (atoi(token) > MAX_Def)
		{
			__parseerror(4);
			exit(1);
		}

		moduleTable[moduleNum] = (moduleType *) malloc (sizeof(moduleType));

		if (moduleNum != 0)
		{
			moduleTable[moduleNum]->address = 
				moduleTable[moduleNum-1]->address +
				moduleTable[moduleNum-1]->size;
		}
	
		moduleTable[moduleNum]->number = moduleNum+1;
		if (moduleTable[moduleNum]->defNum = atoi(token))
			expect = DEFSYM;
		else
			expect = USENUM;
		//printf("DEFNUM complete!\n");
		return 1;
	}	

	//Assign locally defined symbol name
	if (expect == DEFSYM)
	{
		if(!isSym(token) || isEOF(token))
		{
			__parseerror(1);
			exit(1);
		}

		if(strlen(token) > MAX_Name)
		{
			__parseerror(3);
			exit(1);
		}

		strcpy(moduleTable[moduleNum] ->
				defList[defListIdx].name,token);

		for(i=0;symbolTable[i]!=NULL;i++)
		{
			if(!strcmp(symbolTable[i]->name,token))
			{
				symbolTable[i]->exception |= DUPLICATE;
				//printf("Duplicate detected! ");
				duplicateFlag = 1;
				expect = DEFVAL;
				//printf("DEFSYM complete!\n");
				return 1;
			}
		}

		if (symbolTableIdx >= MAX_Symbol)
		{
			__parseerror(6);
			exit(1);
		}
		
		symbolTable[symbolTableIdx] = (symbolType *) malloc(sizeof(symbolType));
		strcpy(symbolTable[symbolTableIdx]->name, token);
		symbolTable[symbolTableIdx]->defmodule = moduleNum + 1;
		expect = DEFVAL;
		//printf("DEFSYM complete!\n");
		return 1;
	}

	//Assign locally defined symbol value
	if (expect == DEFVAL)
	{
		if (!isNum(token) || isEOF(token))
		{
			__parseerror(0);
			exit(1);
		}

		moduleTable[moduleNum] ->
			defList[defListIdx++].value = 
				atoi(token) + moduleTable[moduleNum]->address;

		if (defListIdx < moduleTable[moduleNum] -> defNum)
			expect = DEFSYM;
		else
		{
			defListIdx = 0;
			expect = USENUM;
		}

		if (!duplicateFlag)
		{
			symbolTable[symbolTableIdx]->original = atoi(token);
			symbolTable[symbolTableIdx++]->value = 
				atoi(token) + moduleTable[moduleNum]->address;
		}

		duplicateFlag = 0;	
		//printf("DEFVAL complete!\n");
		return 1;

	}

	//Assign number of using list
	if (expect == USENUM)
	{
		if (!isNum(token) || isEOF(token))
		{
			__parseerror(0);
			exit(1);
		}

		if (atoi(token) > MAX_Use)
		{
			__parseerror(5);
			exit(1);
		}

		if (moduleTable[moduleNum]->useNum = atoi(token))
			expect = USESYM;
		else
			expect = ADDRSIZE;

		//printf("USENUM complete!\n");
		return 1;
	}

	if (expect == USESYM)
	{
		if(!isSym(token) || isEOF(token))
		{
			__parseerror(1);
			exit(1);
		}

		strcpy(moduleTable[moduleNum] ->
				useList[useListIdx++], token);

		if (useListIdx < moduleTable[moduleNum] -> useNum)
			expect = USESYM;
		else
		{
			useListIdx = 0;
			expect = ADDRSIZE;
		}

		//printf("USESYM complete!\n");
		return 1;
	}

	if (expect == ADDRSIZE)
	{
		if (!isNum(token) || isEOF(token))
		{
			__parseerror(0);
			exit(1);
		}

		if ((instrCount += atoi(token)) > 512)
		{
			__parseerror(7);
			exit(1);
		}

		if (moduleTable[moduleNum]->size = atoi(token))
			expect = ADDRTYPE;
		else
			expect = DEFNUM;

		for(i=0;i<symbolTableIdx;i++)
		{
			if (symbolTable[i]->defmodule == moduleNum+1)
				if (symbolTable[i]->original >= moduleTable[moduleNum]->size)
				{
					symbolTable[i]->value = 0;
					symbolTable[i]->exception |= OVERFLOW;
				}
		}
		
		if (expect == DEFNUM)
			moduleNum++;

		//printf("ADDRSIZE complete!\n");
		return 1;
	}

	if (expect == ADDRTYPE)
	{
		if(!isAddr(token) || isEOF(token))
		{
			__parseerror(2);
			exit(1);
		}
		
		moduleTable[moduleNum] -> 
			instrList[instrListIdx].type = *token;

		expect = ADDRNUM;

		//printf("ADDRTYPE complete!\n");
		return 1;

	}

	if (expect == ADDRNUM)
	{
		if (!isNum(token) || isEOF(token))
		{
			__parseerror(0);
			exit(1);
		}
		
		strcpy(moduleTable[moduleNum] -> 
			instrList[instrListIdx++].instr, token);
		
		if (instrListIdx < moduleTable[moduleNum] -> size)
		{
			expect = ADDRTYPE;
		}
		else
		{
			instrListIdx = 0;
			moduleNum++;
			expect = DEFNUM;
		}

		//printf("ADDRNUM complete!\n");
		return 1;

		
	}

	
	return 1;
}

int pass1(FILE * inputFile)
{ 	
	char * token;
	while (token = nextToken(inputFile))
	{
		//printf("%s,",token);
		processToken(token);	
	}
	processToken("EOF");

	printPass1();
	return 1;
}

int pass2(FILE * inputFile)

{
	processModule();
	printPass2();
	return 1;
}

void __parseerror(int errcode) 
{
	static char* errstr[] = 
	{
		"NUM_EXPECTED",
		"SYM_EXPECTED",
		"ADDR_EXPECTED",
		"SYM_TOLONG",
		"TO_MANY_DEF_IN_MODULE",
		"TO_MANY_USE_IN_MODULE",
		"TO_MANY_SYMBOLS",
		"TO_MANY_INSTR",
	};
	printf("Parse Error line %d offset %d: %s\n", linenum, lineoffset, errstr[errcode]);
}

/*
	int number;
	int address;

	int defNum;
	int useNum;
	int size;

	symbolType defList[MAX_Def];
	char useList[MAX_Use][MAX_Name+1];
	instrType instrList[Machine_Size];
 */
int processModule()
{
	int i=0, j=0, k=0, memoryMapIdx=0, useCode=0, relativeAddr = 0;
	char symbol[MAX_Name], foundFlag=0, notUsedIdx =0;
	for(i=0;moduleTable[i]!=NULL;i++)
	{
		//Scan if any symbol is in uselist but not actually used
		for(j=0;moduleTable[i]->useList[j][0]!='\0';j++)
		{
			for (k=0;k<moduleTable[i]->size;k++)
			{
				if (moduleTable[i]->instrList[k].type == 'E')
					if (atoi(moduleTable[i]->instrList[k].instr+1) == j)
					{
						foundFlag=1;
						break;
					}
			}
			if (foundFlag != 1)
			{
				notUsedByModule[notUsedIdx] = 
					(notUsedType *) malloc(sizeof(notUsedType));
				notUsedByModule[notUsedIdx]->module = i+1;
				strcpy(notUsedByModule[notUsedIdx]->symbol,
						moduleTable[i]->useList[j]);

			}

			foundFlag = 0;

		}

		for(j=0;j<moduleTable[i]->size;j++)
		{
			memoryMap[memoryMapIdx] = (memoryType *) malloc(sizeof(memoryType));
			switch (moduleTable[i]->instrList[j].type)
			{
				case 'A':
					//printf("Enter A\n");
					if (strlen(moduleTable[i]->instrList[j].instr)>4)
					{
						memoryMap[memoryMapIdx]->exception |= ILLEGAL_O;
						memoryMap[memoryMapIdx]->value = 9999;
						break;
					}
					memoryMap[memoryMapIdx]->value = 
						atoi(moduleTable[i]->instrList[j].instr);
	
					
					if ((memoryMap[memoryMapIdx]->value%1000) >= Machine_Size)
					{
						memoryMap[memoryMapIdx]->exception |= OVERFLOW_A;
						memoryMap[memoryMapIdx]->value /= 1000;
						memoryMap[memoryMapIdx]->value *= 1000;
					}
					//printf("value = %d\n",memoryMap[memoryMapIdx]->value);
					break;

				case 'I':
					//printf("Enter I\n");
					if (strlen(moduleTable[i]->instrList[j].instr)>4)
					{
						memoryMap[memoryMapIdx]->exception |= ILLEGAL_I;
						memoryMap[memoryMapIdx]->value = 9999;
						break;
					}
					memoryMap[memoryMapIdx]->value = 
						atoi(moduleTable[i]->instrList[j].instr);
					//printf("value = %d\n",memoryMap[memoryMapIdx]->value);
					break;

				case 'R':
					//printf("Enter R\n");
					if (strlen(moduleTable[i]->instrList[j].instr)>4)
					{
						memoryMap[memoryMapIdx]->exception |= ILLEGAL_O;
						memoryMap[memoryMapIdx]->value = 9999;
						break;
					}
					relativeAddr =atoi(moduleTable[i]->instrList[j].instr);
					if (relativeAddr%1000 >= moduleTable[i]->size)
					{
						memoryMap[memoryMapIdx]->exception |= OVERFLOW_R;
						relativeAddr /= 1000;
						relativeAddr *= 1000;
					}

					memoryMap[memoryMapIdx]->value = 
						relativeAddr + moduleTable[i]->address;
					

					if (memoryMap[memoryMapIdx]->value%1000 >= Machine_Size)
					{
						memoryMap[memoryMapIdx]->exception |= OVERFLOW_A;
						memoryMap[memoryMapIdx]->value /= 1000;
						memoryMap[memoryMapIdx]->value *= 1000;
					}
					//printf("value = %d\n",memoryMap[memoryMapIdx]->value);
					break;

				case 'E':
					//printf("Enter E\n");
					if (strlen(moduleTable[i]->instrList[j].instr)>4)
					{	
						//Illegal op code if over 4 digits
						memoryMap[memoryMapIdx]->exception |= ILLEGAL_O;
						memoryMap[memoryMapIdx]->value = 9999;
						break;
					}

					useCode = atoi(moduleTable[i]->instrList[j].instr+1);
					if (useCode >= moduleTable[i]->useNum)
					{
						//exceed use list, treat as immediate
						memoryMap[memoryMapIdx]->exception |= OVERFLOW_E;
						memoryMap[memoryMapIdx]->value = 
							atoi(moduleTable[i]->instrList[j].instr);

					}
					else
						//replace with absolute address
					{
						strcpy(symbol, moduleTable[i]->useList[useCode]);
						for (k=0;symbolTable[k]!=NULL;k++)
						{
							//printf("searching %s...\n",symbol);
							if(!strcmp(symbolTable[k]->name,symbol))
							{	//printf("Found %s!\n",symbol);
								memoryMap[memoryMapIdx]->value = 
									(symbolTable[k]->value) + 
									1000*(*moduleTable[i]->
										instrList[j].instr 
										- 48);
								symbolTable[k]->used = 1;
								break;
							}
						
						}

						//Symbol not defined, assign zero
						if (symbolTable[k] == NULL)
						{
						memoryMap[memoryMapIdx]->exception |= NOT_DEF;
						memoryMap[memoryMapIdx]->value =
							*moduleTable[i]->instrList[j].instr-48;
						memoryMap[memoryMapIdx]->value *= 1000;
						memoryMap[memoryMapIdx]->missSymbol = 
							(char *) malloc(sizeof(char)*MAX_Name);
						strcpy(memoryMap[memoryMapIdx]->missSymbol,symbol);
						}
					}
					
					
					
					//printf("value = %d\n",memoryMap[memoryMapIdx]->value);
					break;
				default :return 0;
			}
			memoryMapIdx++;

		}
	}

	return 1;
}

void __semanticerror(int errcode) 
{
	static char* errstr[] = 
	{
		"Error: Absolute address exceeds machine size; zero used",
		"Error: Relative address exceeds module size; zero used",
		"Error: External address exceeds length of uselist; treated as immediate",
		"Error: %s is not defined; zero used",//Not used. Implement individually.
		"Error: This variable is multiple times defined; first value used",
		"Error: Illegal immediate value; treated as 9999",
		"Error: Illegal opcode; treated as 9999",
	};
	printf("%s", errstr[errcode]);
}


void printPass1()
{
	int i=0, j=0;
	
	for (i=0;symbolTable[i]!=NULL;i++)
	{
		if (symbolTable[i]->exception & OVERFLOW)
		{
		printf("Warning: Module %d: %s to big %d (max=%d) assume zero relative\n", symbolTable[i]->defmodule, symbolTable[i]->name, symbolTable[i]->original, moduleTable[symbolTable[i]->defmodule-1]->size-1);
		}

	}
	printf("Symbol Table\n");
	for(i=0;symbolTable[i] != NULL;i++)
	{
		printf("%s=%d ",symbolTable[i]->name,symbolTable[i]->value);
		if (symbolTable[i]->exception & DUPLICATE)
			__semanticerror(4);

		printf("\n");
	}
		j = 0;
}


void printPass2()
{
	int i = 0;
	printf("\nMemory Map\n");
	for (i=0; memoryMap[i]!=NULL; i++)
	{
		printf("%03d: %04d ",i,memoryMap[i]->value);
		if (memoryMap[i]->exception & OVERFLOW_A)
			__semanticerror(0);
		else if (memoryMap[i]->exception & OVERFLOW_R)
			__semanticerror(1);
		else if (memoryMap[i]->exception & OVERFLOW_E)
			__semanticerror(2);
		else if (memoryMap[i]->exception & NOT_DEF)
			printf("Error: %s is not defined; zero used",memoryMap[i]->missSymbol);
		else if (memoryMap[i]->exception & ILLEGAL_I)
			__semanticerror(5);
		else if (memoryMap[i]->exception & ILLEGAL_O)
			__semanticerror(6);
		
		printf("\n");
	}

	printf("\n");

	for (i=0; notUsedByModule[i]!=NULL; i++)
			printf("Warning: In Module %d %s appeared in the uselist but was not actually used\n", notUsedByModule[i]->module, notUsedByModule[i]->symbol);

	for (i=0; symbolTable[i]!=NULL; i++)
	{
		if (symbolTable[i]->used != 1)
			printf("Warning: %s was defined in module %d but never used\n",
					symbolTable[i]->name,
					symbolTable[i]->defmodule);
	}
}

int isSym(char * str)
{
	if ((*str<='Z')&&(*str>='A') || (*str<='z')&&(*str>='a'))
	{
		while(*(++str))
		{
			if ((*str<='Z')&&(*str>='A') || (*str<='z')&&(*str>='a')|| 
					(*str<='9')&&(*str>='0'))
				continue;
			else
			{
				return 0;
			}
		}
		
	}

	else
		return 0;	
	return 1;
}

int isNum(char * str)
{
	while(*str != '\0')
	{
		if(*str>'9' || *str<'0')
			return 0;
		str++;
	}
	return 1;
}

int isAddr(char * str)
{
	if(strlen(str)==1)
		switch (*str)
		{
			case 'I': return 1; 
				  break;
			case 'R': return 1; 
				  break;
			case 'E': return 1; 
				  break;
			case 'A': return 1; 
				  break;
			default: return 0; break;
		}
	
	return 0;
}

int isEOF(char * str)
{
	return !strcmp(str,"EOF");
}

