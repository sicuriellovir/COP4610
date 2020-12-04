#include "rm.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

/*removes a file from the current directory and frees its data cluster(s). takes in the start
cluster of the current directory, the file name to remove (if possible), the fat32 file pointer,
and the BPBInfo on the fat32 file. if NULL is passed in for fileName, this function does nothing*/
//NOTE: MAKE SURE THE BPBInfo HAS BEEN INITIALIZED USING THE BPBInfoInit FUNCTION
void rm(unsigned int pwdStartCluster, char* fileName, int fatFile_fp, struct BPBInfo* info)
{
    if (fileName == NULL)
        return;

    struct DIRENTRY** dirEntryArray = _getDirEntriesFromAllClusters(pwdStartCluster, fatFile_fp, info);

    //loops through directory entries
    for (int i = 0; dirEntryArray[i] != NULL; i++)
    {
        if (!strcasecmp(fileName, dirEntryArray[i]->DIR_name))
        {
            if (dirEntryArray[i]->DIR_Attributes == 0x20)
                _removeFile(dirEntryArray[i], fatFile_fp, info);
            else
                printf("Error: %s is a directory, not a file\n", fileName);

            _freeDirEntryArray(dirEntryArray);
            return;
        }
    }
    printf("Error: %s not found in the current directory\n", fileName);
    _freeDirEntryArray(dirEntryArray);
}

//Removes a file's direntry, clears the file's data (replaced with all 0's), and sets each cluster as available.
//NOTE: MAKE SURE THE BPBInfo HAS BEEN INITIALIZED USING THE BPBInfoInit FUNCTION
void _removeFile(struct DIRENTRY* entry, int fatFile_fp, struct BPBInfo* info)
{
    unsigned int* currentCluster = &entry->DIR_DataCluster;
    unsigned int temp;
    unsigned char byte = 0xE5;
    unsigned char byteBuff[63];

    memset(byteBuff, 0, 63);

    while (currentCluster != NULL)
    {
        _removeClusterData(*currentCluster, fatFile_fp, info);

        temp = *currentCluster;
        if (currentCluster != &entry->DIR_DataCluster)
            free(currentCluster);
        currentCluster = _getNextCluster(temp, fatFile_fp, info);

        _setClusterAsAvailable(temp, fatFile_fp, info);
    }

    //removes the dir entry and sets the first byte of the entry to 0xE5 (indicates the entry is available)
    lseek(fatFile_fp, entry->DIR_EntryByteOffset, SEEK_SET);
    write(fatFile_fp, &byte, 1);
    write(fatFile_fp, &byteBuff, 63);
}