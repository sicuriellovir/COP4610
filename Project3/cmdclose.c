#include "cmdclose.h"
#include <stdio.h>
#include <string.h>

void cmdclose(unsigned int pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info, struct openFile *head, struct openFile *ptr)
{
    struct openFile *secondPtr;
    int i;
    for(i = strlen(dirName); i < 11; i++)
        strcat(dirName, " ");
    

    if(OpenFile(dirName, head) == 0)
        printf("File not found.\n");
    else
    {
        for(ptr = head; ptr !=NULL; secondPtr = ptr)
        {
            if (strncmp(ptr->entry->DIR_name, dirName, 11) != 0)
                continue;
            if(secondPtr != NULL) 
                secondPtr->next = ptr->next;
            free(ptr);
            printf("File Closed\n");
            return;
        }

    }
}
