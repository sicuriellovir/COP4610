#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>


void CopyRecursive( char* from , char* to)
{
    int errno,ENOENT,ENOTDIR;
    if (0 != access(from, F_OK)) {
        if (ENOENT == errno) {
             // does not exist
            printf("Directory does not exist\n");
        } 
        if (ENOTDIR == errno) {
          // not a directory
            printf("Not a valid entry\n");
        }
    }
    if (0 != access(to, F_OK)) {
        if (ENOENT == errno) {
             // does not exist
             mkdir(to,0777);
             char c2[100];
              strcpy(c2, to);
              copy_file(from,c2);
        }
    }
}