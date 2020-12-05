#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "helpers.h"
#include <errno.h>
#include <unistd.h>

void copyFile(char* file , struct DIRENTRY* dir)
{
    if (0 != access(dir->DIR_name, F_OK))
    {
        if (ENOENT == errno) {
     // does not exist
            printf("Does not exist ... \n");
        }
        if (ENOTDIR == errno) {
     // not a directory
            printf ("Not a directory ...\n");
        }
        return;
    }
    FILE* fp = fopen(file, "r"); 

    // checking if the file exist or not 
    if (fp == NULL) { 
        printf("File Not Found in current directory!\n"); 
        return; 
    }
}
