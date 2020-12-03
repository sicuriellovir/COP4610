#include "cmdclose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cmdclose(char* fileName, struct openFile *head)
{
    struct openFile* temp = NULL;

    for (struct openFile* i = head; i != NULL && temp == NULL; i = i->next) 
    {
        if (!strcasecmp(fileName, i->entry->DIR_name))
                Remove(fileName, head);
        else
            printf("Error: could not close file\n");
    }
}
