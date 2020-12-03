#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "helpers.h"
#include "size.h"

void printSize(char* fileName)
{
	FILE* fp = fopen(fileName, "r"); 

    // checking if the file exist or not 
    if (fp == NULL) { 
        printf("File Not Found! in current directory\n"); 
        return; 
    } 
    fseek(fp, 0L, SEEK_END); 
    // calculating the size of the file 
    long int res = ftell(fp); 
  	 printf("Size of the file is %ld bytes \n", res); 
    // closing the file 
    fclose(fp); 
}
