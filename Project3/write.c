#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "helpers.h"

void writeToFile(char* file , char * stringToWrite , int offset)
{
	FILE* fp = fopen(file, "a"); 

    // checking if the file exist or not 
    if (fp == NULL) { 
        printf("File Not Found! in current directory\n"); 
        return; 
    } 
    
    fseek (fp, offset, SEEK_SET);
    fwrite (stringToWrite , 1 ,  sizeof(stringToWrite)/sizeof(char), fp);

    fclose(fp); 
}