#include <stdio.h>
#include <string.h>
#include "size.h"

void printSize(char* fileName, unsigned int pwdStartCluster, int fatFile_fp, struct BPBInfo* info)
{
	struct DIRENTRY** entries = _getDirEntriesFromAllClusters(pwdStartCluster, fatFile_fp, info);
	int i = 0;

	for (; entries[i] != NULL; i++)
    {
	    if (!strcasecmp(entries[i], fileName))
	        break;
    }

	if (entries[i] == NULL)
    {
	    printf("Error: %s was not found in the current directory\n", fileName);
	    return;
    }

	printf("%s is %u bytes long\n", fileName, entries[i]->DIR_fileSize);
}