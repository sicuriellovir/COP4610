#include "lseek.h"
#include <stdio.h>
#include <string.h>

//sets the lseekOffset of the passed openFile to offset if the offset is <= the size of the file and the file is open
void setLseek(unsigned int pwdStartCluster, struct openFile* files, char* fileName, unsigned int offset, int fatFile_fp, struct BPBInfo* info)
{
    struct openFile* temp = NULL;

    struct DIRENTRY** pwdEntries = _getDirEntriesFromAllClusters(pwdStartCluster, fatFile_fp, info);
    for (int i1 = 0; pwdEntries[i1] != NULL; i1++)
    {
        if (!strcasecmp(fileName, pwdEntries[i1]) && OpenFile(pwdEntries[i1], files))
        {
            for (struct openFile* i2 = files; i2 != NULL && temp == NULL; i2 = i2->next) {
                if (!strcasecmp(fileName, i2->entry->DIR_name))
                    temp = i2;
            }
        }
    }

    if (temp == NULL)
    {
        printf("Error: %s was either not found in the current directory or is not open\n", fileName);
        return;
    }

    if (offset > temp->entry->DIR_fileSize)
        printf("Error: %d is larger than the size of file %s. The size of this file is %d bytes\n",
                offset, temp->entry->DIR_name, temp->entry->DIR_fileSize);
    else
        temp->lseekOffset = offset;
}