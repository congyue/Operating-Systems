#include"fileIO.h"

int linenum = 1, lineoffset = 0, randLinenum = 2, randLineoffset = 0;

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
	static int lastTokenLen = 0;

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
		else if (letter == '#')
		{	
			while(fgetc(inputFile) != '\n');
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
	static int lastTokenLen = 0;

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

