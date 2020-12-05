#include "cmdopen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cmdopen(unsigned int pwdStartCluster, char* dirName, char *mode, int fatFile_fp, struct BPBInfo* info, struct openFile* head)
{
	if (mode == NULL)
	{
		printf("Error: invalid mode used\n");
		return;
	}

    struct DIRENTRY** pwdEntries = _getDirEntriesFromAllClusters(pwdStartCluster, fatFile_fp, info);
    struct openFile* tempFile = NULL;

    for (int i = 0; pwdEntries[i] != NULL; i++)
    {
        if (!strcasecmp(pwdEntries[i]->DIR_name, dirName) && !OpenFile(pwdEntries[i], head))
        {
            tempFile = (struct openFile *) malloc(sizeof(struct openFile));
            tempFile->entry = pwdEntries[i];
        }
    }

    if (tempFile == NULL)
    {
        printf("Error: %s was either not found in the current directory or is already open\n", dirName);
        return;
    }

    if (!strcmp(mode, "r"))
        tempFile->mode = READONLY;
    else if (!strcmp(mode, "w"))
        tempFile->mode = WRITEONLY;
    else if (!strcmp(mode, "rw") || !strcmp(mode, "wr"))
        tempFile->mode = READANDWRITE;
    else
    {
    	printf("Error: invalid mode used\n");
    	_freeDirEntryArray(pwdEntries);
    	free(tempFile);
    	return;
    }

    addFile(head, tempFile);
    _freeDirEntryArray(pwdEntries);
    free(tempFile);
}
