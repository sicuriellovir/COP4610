#include "cmdclose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cmdclose(char* fileName, struct openFile *head, unsigned int pwdStartCluster, int fatFile_fp, struct BPBInfo* info)
{
    struct openFile* temp = NULL;

    struct DIRENTRY** pwdEntries = _getDirEntriesFromAllClusters(pwdStartCluster, fatFile_fp, info);
    for (int i1 = 0; pwdEntries[i1] != NULL; i1++)
    {
        if (!strcasecmp(fileName, pwdEntries[i1]) && OpenFile(pwdEntries[i1], head))
        {
            for (struct openFile* i2 = head; i2 != NULL && temp == NULL; i2 = i2->next) {
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

    Remove(temp->entry, head);
}
