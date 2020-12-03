#include "lseek.h"
#include <stdio.h>
#include <string.h>

//sets the lseekOffset of the passed openFile to offset if the offset is <= the size of the file and the file is open
void setLseek(unsigned int pwdStartCluster, struct openFile* files, char* fileName, unsigned int offset)
{
    struct openFile* temp = NULL;

    for (struct openFile* i = files; i != NULL && temp == NULL; i = i->next) {
        if (!strcasecmp(fileName, i->entry->DIR_name))
            temp = i;
    }

    if (temp == NULL)
    {
        printf("Error: %s is not an open file\n", fileName);
        return;
    }

    if (offset > temp->entry->DIR_fileSize)
        printf("Error: %d is larger than the size of file %s. The size of this file is %d bytes\n",
                offset, temp->entry->DIR_name, temp->entry->DIR_fileSize);
    else
        temp->lseekOffset = offset;
}