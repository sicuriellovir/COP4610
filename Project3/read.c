#include "read.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

void readFile(unsigned int pwdStartCluster, struct openFile* files, char* fileName, unsigned int size, int fatFile_fp, struct BPBInfo* info) {
    struct openFile *temp = NULL;
    unsigned int bytesPerClus = info->BytesPerSec * info->SecPerClus;
    unsigned int clusterNumInChain;
    unsigned int numClustersToRead;
    unsigned int* currentCluster;
    unsigned int tempCluster;
    unsigned int offset = 0;
    unsigned int bytesRead = 0;
    unsigned char* byteBuff;

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

    else if (temp->entry->DIR_Attributes != 0x20) {
        printf("Error: %s is a directory, not a file\n", temp->entry->DIR_name);
        return;
    }
    else if (temp->mode == WRITEONLY) {
        printf("Error: file %s is not opened for reading\n", temp->entry->DIR_name);
        return;
    }

    clusterNumInChain = temp->lseekOffset / bytesPerClus;
    currentCluster = &temp->entry->DIR_DataCluster;

    /*this loop sets dataStartCluster to the first cluster that the data
    will be read from (the data may span across multiple clusters)*/
    for (int i = 0; i < clusterNumInChain; i++)
    {
        tempCluster = *currentCluster;
        if (currentCluster != &temp->entry->DIR_DataCluster)
            free(currentCluster);
        currentCluster = _getNextCluster(tempCluster, fatFile_fp, info);
    }

    if (temp->lseekOffset + size > temp->entry->DIR_fileSize)
        size = temp->entry->DIR_fileSize - temp->lseekOffset;

    //calculates the number of clusters to read from (how many clusters the data spans across)
    numClustersToRead = (((temp->lseekOffset % bytesPerClus) + size) / bytesPerClus) + 1;

    byteBuff = (unsigned char*) malloc(sizeof(char) * (size + 1));
    offset = getByteOffsetFromCluster(*currentCluster, info) + (temp->lseekOffset % bytesPerClus);
    lseek(fatFile_fp, offset, SEEK_SET);
    if (size < bytesPerClus - (temp->lseekOffset % bytesPerClus))
    {
        read(fatFile_fp, byteBuff, size);
        bytesRead += size;
    }
    else
    {
        read(fatFile_fp, byteBuff, (bytesPerClus - (temp->lseekOffset % bytesPerClus)));
        bytesRead += bytesPerClus - (temp->lseekOffset % bytesPerClus);
    }

    for (int i = 1; i < numClustersToRead; i++)
    {
        tempCluster = *currentCluster;
        if (currentCluster != &temp->entry->DIR_DataCluster)
            free(currentCluster);
        currentCluster = _getNextCluster(tempCluster, fatFile_fp, info);

        offset = getByteOffsetFromCluster(*currentCluster, info);
        lseek(fatFile_fp, offset, SEEK_SET);
        if ((size - bytesRead) / bytesPerClus >= 1)
        {
            read(fatFile_fp, byteBuff + bytesRead, bytesPerClus);
            bytesRead += bytesPerClus;
        }
        else
        {
            read(fatFile_fp, byteBuff + bytesRead, size - bytesRead);
            bytesRead += size - bytesRead;
        }
    }
    byteBuff[size] = '\0';

    temp->lseekOffset += size;

    if (size != 0)
        printf("%s\n", byteBuff);
}