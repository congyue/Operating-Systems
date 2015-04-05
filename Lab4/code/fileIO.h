#ifndef _FILEIO_H
#define _FILEIO_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX_LINE 512

extern int linenum, lineoffset, randLinenum, randLineoffset;

FILE * inputFile, * randFile;

FILE * readFile(char * path);
char * nextToken(FILE * inputFile);
char * nextRand(FILE * inputFile);

#endif 
