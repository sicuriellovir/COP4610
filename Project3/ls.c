#include "ls.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

/*Implementation for ls. Takes in a c-string for the present working directory path, a c-string holding
the name of the directory to perform ls on (if this is NULL, ls will be performed on the current directory),
the file pointer for the FAT32 file, and the BPBInfo for the file.*/
//NOTE: MAKE SURE THE BPBInfo HAS BEEN INITIALIZED USING THE BPBInfoInit FUNCTION
void ls(unsigned int pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info)
{
    struct DIRENTRY** dirEntryArray = _getDirEntriesFromFirstCluster(pwdStartCluster, fatFile_fp, info);

    if (dirName == NULL)
    {
        //prints entries in the current directory
        for (int i = 0; dirEntryArray[i] != NULL; i++)
            printf("%s\n", dirEntryArray[i]->DIR_name);

        _freeDirEntryArray(dirEntryArray);
        return;
    }
    else
    {
        //copies dirName to tempStr and converts it to uppercase
        char* tempStr = malloc(strlen(dirName) * sizeof(char));
        strcpy(tempStr, dirName);
        for (int i = 0; tempStr[i] != '\0'; i++)
            tempStr[i] = toupper(tempStr[i]);

        //loops through directory entries
        for (int i = 0; dirEntryArray[i] != NULL; i++)
        {
            if (strncmp(tempStr, dirEntryArray[i]->DIR_name, strlen(tempStr)) == 0)
            {
                if (dirEntryArray[i]->DIR_Attributes == 0x10)
                    ls(dirEntryArray[i]->DIR_DataCluster, NULL, fatFile_fp, info);
                else
                    printf("Error: %s is a file, not a directory\n", dirName);

                _freeDirEntryArray(dirEntryArray);
                free(tempStr);
                return;
            }
        }
        printf("Error: %s not found in the current directory\n", dirName);
        _freeDirEntryArray(dirEntryArray);
        free(tempStr);
    }
}

//Calculates and returns a NULL terminated array of c-strings with the name of every entry in a directory
//Takes in the FIRST CLUSTER of the directory, the file pointer for the fat32 file, and the BPBInfo for the file
struct DIRENTRY** _getDirEntriesFromFirstCluster(unsigned int firstCluster, int fatFile_fp, struct BPBInfo* info)
{
    const unsigned int firstDataSector = info->RsvdSecCnt + (info->NumFATs * info->FATSize);
    unsigned int *currentCluster = &firstCluster;
    unsigned int byteOffset;
    unsigned int offset;
    unsigned int entryByteOffset = 64;
    unsigned char byteBuff[4];
    unsigned char byte = 0;
    unsigned int temp;

    unsigned int numDirNames = 0;
    struct DIRENTRY** dirEntryArray = (struct DIRENTRY**) malloc(sizeof(struct DIRENTRY*));
    struct DIRENTRY* tempDirEntry;

    if (firstCluster != info->RootClus)
    {
        tempDirEntry = (struct DIRENTRY*) malloc(sizeof(struct DIRENTRY));
        strcpy(tempDirEntry->DIR_name, ".");
        tempDirEntry->DIR_Attributes = 0x10;
        tempDirEntry->DIR_DataCluster = firstCluster;
        dirEntryArray[0] = tempDirEntry;
        numDirNames++;
    }
    else
        dirEntryArray[0] = NULL;

    while (currentCluster != NULL)
    {
        byteOffset = (firstDataSector + ((*currentCluster - 2) * info->SecPerClus)) * info->BytesPerSec;
        offset = byteOffset + 32;

        read(fatFile_fp, &byte, 1);
        while (byte != 0 && offset - byteOffset < info->BytesPerSec * info->SecPerClus)
        {
            lseek(fatFile_fp, offset, SEEK_SET);
            tempDirEntry = (struct DIRENTRY*) malloc(sizeof(struct DIRENTRY));
            read(fatFile_fp, tempDirEntry->DIR_name, 11);
            tempDirEntry->DIR_name[11] = '\0';

            lseek(fatFile_fp, offset + 11, SEEK_SET);
            read(fatFile_fp, &tempDirEntry->DIR_Attributes, 1);

            lseek(fatFile_fp, offset + 20, SEEK_SET);
            read(fatFile_fp, byteBuff, 2);
            lseek(fatFile_fp, offset + 26, SEEK_SET);
            read(fatFile_fp, byteBuff + 2, 2);
            tempDirEntry->DIR_DataCluster = byteBuff[0] << 24;
            tempDirEntry->DIR_DataCluster |= byteBuff[1] << 16;
            tempDirEntry->DIR_DataCluster |= byteBuff[3] << 8;
            tempDirEntry->DIR_DataCluster |= byteBuff[2];

            dirEntryArray = (struct DIRENTRY**) realloc(dirEntryArray, sizeof(struct DIRENTRY*) * (numDirNames + 2));
            dirEntryArray[numDirNames] = tempDirEntry;
            numDirNames += 1;

            offset += entryByteOffset;
            lseek(fatFile_fp, offset, SEEK_SET);
            read(fatFile_fp, &byte, 1);
        }
        temp = *currentCluster;
        if (currentCluster != &firstCluster)
            free(currentCluster);
        currentCluster = _isLastCluster(temp, fatFile_fp, info);
    }

    dirEntryArray[numDirNames] = NULL;
    return dirEntryArray;
}

//Determines if the passed cluster is the last one. Returns NULL if it is the last cluster, otherwise returns
//a pointer to the next cluster number
unsigned int* _isLastCluster(unsigned int cluster, int fatFile_fp, struct BPBInfo* info)
{
    unsigned char buff[4];
    unsigned int offset = (info->RsvdSecCnt * info->BytesPerSec) + (cluster * 4);
    unsigned int* entry = (unsigned int*) malloc(sizeof(unsigned int));
    *entry = 0;

    lseek(fatFile_fp, offset, SEEK_SET);
    read(fatFile_fp, buff, 4);

    *entry |= buff[3] << 24;
    *entry |= buff[2] << 16;
    *entry |= buff[1] << 8;
    *entry |= buff[0];

    //this if condition tests if the entry indicates that this is the last cluster
    if (*entry == 0x0FFFFFF8 || *entry == 0x0FFFFFF9 || *entry == 0x0FFFFFFA || *entry == 0x0FFFFFFB
    || *entry == 0x0FFFFFFC || *entry == 0x0FFFFFFD || *entry == 0x0FFFFFFE || *entry == 0x0FFFFFFF
    || *entry == 0xFFFFFFFF)
        return NULL;
    else
        return entry;
}

//deallocates a NULL TERMINATED array of direntries
void _freeDirEntryArray(struct DIRENTRY** entries)
{
    for (int i = 0; entries[i] != NULL; i++)
        free(entries[i]);
    free(entries);
}