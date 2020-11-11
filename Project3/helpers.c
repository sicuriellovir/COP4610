#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

void BPBInfoInit(struct BPBInfo* info, int fatFile_fp)
{
    //byte buffer for the code below
    unsigned char buff[4];

    //initializes the BPBInfo data to 0
    info->BytesPerSec = 0;
    info->FATSize = 0;
    info->NumFATs = 0;
    info->RootClus = 0;
    info->RsvdSecCnt = 0;
    info->SecPerClus = 0;
    info->TotSec = 0;

    /*Gets the number of bytes per sector in this file. This number is 2 bytes (unsigned short)
    and is specified in a fat32 image file at offset 11 (refer to the fat32 specifications
    page 8 for details*/
    lseek(fatFile_fp, 11, SEEK_SET);
    read(fatFile_fp, buff, 2);
    /*The 2 byte number is in little endian, so the second byte read is actually the first
    byte of the value we are looking for. Shifting the second byte to the left by 8 bits
    takes care of the conversion between endianness*/
    info->BytesPerSec |= buff[1] << 8;
    info->BytesPerSec |= buff[0];

    //does the same thing as above, but for the sectors per cluster, details in fat32 specs
    lseek(fatFile_fp, 13, SEEK_SET);
    /*this value is only 1 byte, so endianness doesn't matter and we can read the value directly
    into our variable*/
    read(fatFile_fp, &info->SecPerClus, 1);

    //gets reserved region size
    lseek(fatFile_fp, 14, SEEK_SET);
    read(fatFile_fp, buff, 2);
    info->RsvdSecCnt |= buff[1] << 8;
    info->RsvdSecCnt |= buff[0];

    //gets number of FATs
    lseek(fatFile_fp, 16, SEEK_SET);
    read(fatFile_fp, &info->NumFATs, 1);

    //gets the FAT size
    lseek(fatFile_fp, 36, SEEK_SET);
    read(fatFile_fp, buff, 4);
    //this value is 4 bytes, so once again we have to deal with the endianness by shifting
    info->FATSize |= buff[3] << 24;
    info->FATSize |= buff[2] << 16;
    info->FATSize |= buff[1] << 8;
    info->FATSize |= buff[0];

    //gets the root cluster number
    lseek(fatFile_fp, 44, SEEK_SET);
    read(fatFile_fp, buff, 4);
    info->RootClus |= buff[3] << 24;
    info->RootClus |= buff[2] << 16;
    info->RootClus |= buff[1] << 8;
    info->RootClus |= buff[0];

    //get the total number of sectors
    lseek(fatFile_fp, 32, SEEK_SET);
    read(fatFile_fp, buff, 4);
    info->TotSec |= buff[3] << 24;
    info->TotSec |= buff[2] << 16;
    info->TotSec |= buff[1] << 8;
    info->TotSec |= buff[0];
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
    unsigned char* tempChar;

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

            tempChar = strchr(tempDirEntry->DIR_name, 0x20);
            if (tempChar != NULL)
                *tempChar = '\0';

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