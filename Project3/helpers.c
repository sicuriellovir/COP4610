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
//NOTE: MAKE SURE THE BPBInfo HAS BEEN INITIALIZED USING THE BPBInfoInit FUNCTION
struct DIRENTRY** _getDirEntriesFromAllClusters(unsigned int firstCluster, int fatFile_fp, struct BPBInfo* info)
{
    unsigned int *currentCluster = &firstCluster;
    unsigned int temp;

    unsigned int numDirEntries = 0;
    struct DIRENTRY** dirEntryArray = (struct DIRENTRY**) malloc(sizeof(struct DIRENTRY*));
    struct DIRENTRY** tempDirEntryArray;

    dirEntryArray[0] = NULL;

    while (currentCluster != NULL)
    {
        tempDirEntryArray = _getDirEntriesFromCluster(*currentCluster, fatFile_fp, info);
        for (int i = 0; tempDirEntryArray[i] != NULL; i++)
        {
            dirEntryArray = (struct DIRENTRY**) realloc(dirEntryArray, sizeof(struct DIRENTRY*) * (numDirEntries + 2));
            dirEntryArray[numDirEntries] = tempDirEntryArray[i];
            numDirEntries++;
        }
        free(tempDirEntryArray);

        temp = *currentCluster;
        if (currentCluster != &firstCluster)
            free(currentCluster);
        currentCluster = _getNextCluster(temp, fatFile_fp, info);
    }

    dirEntryArray[numDirEntries] = NULL;
    return dirEntryArray;
}

//Calculates and returns a NULL terminated array of c-strings with the name of every dir entry in a cluster
//Takes in the cluster number of the directory, the file pointer for the fat32 file, and the BPBInfo for the file
//NOTE: MAKE SURE THE BPBInfo HAS BEEN INITIALIZED USING THE BPBInfoInit FUNCTION
struct DIRENTRY** _getDirEntriesFromCluster(unsigned int clusterNum, int fatFile_fp, struct BPBInfo* info)
{
    unsigned int byteOffset = getByteOffsetFromCluster(clusterNum, info);
    unsigned int offset;
    unsigned int entryByteOffset = 64;
    unsigned char byteBuff[4];
    unsigned char byte = 0;
    unsigned char* tempChar;

    unsigned int numDirEntries = 0;
    struct DIRENTRY** dirEntryArray = (struct DIRENTRY**) malloc(sizeof(struct DIRENTRY*));
    struct DIRENTRY* tempDirEntry;

    if (clusterNum != info->RootClus)
    {
        lseek(fatFile_fp, byteOffset, SEEK_SET);
        read(fatFile_fp, &byte, 1);
        if (byte == '.')
        {
            tempDirEntry = (struct DIRENTRY *) malloc(sizeof(struct DIRENTRY));
            strcpy(tempDirEntry->DIR_name, ".");
            tempDirEntry->DIR_Attributes = 0x10;
            tempDirEntry->DIR_DataCluster = clusterNum;
            tempDirEntry->DIR_EntryByteOffset = byteOffset;
            dirEntryArray[0] = tempDirEntry;
            numDirEntries++;
        }
    }
    else
        dirEntryArray[0] = NULL;

    offset = byteOffset + 32;

    while (offset - byteOffset < info->BytesPerSec * info->SecPerClus)
    {
        lseek(fatFile_fp, offset - 32, SEEK_SET);
        read(fatFile_fp, &byte, 1);
        if (byte != 0xE5 && byte != 0)
        {
            lseek(fatFile_fp, offset, SEEK_SET);
            tempDirEntry = (struct DIRENTRY *) malloc(sizeof(struct DIRENTRY));
            read(fatFile_fp, tempDirEntry->DIR_name, 11);
            tempDirEntry->DIR_name[11] = '\0';

            tempChar = strchr(tempDirEntry->DIR_name, 0x20);
            if (tempChar != NULL)
                *tempChar = '\0';

            lseek(fatFile_fp, offset + 11, SEEK_SET);
            read(fatFile_fp, &tempDirEntry->DIR_Attributes, 1);

            if (!strcmp(tempDirEntry->DIR_name, ".."))
                tempDirEntry->DIR_EntryByteOffset = offset;
            else
                tempDirEntry->DIR_EntryByteOffset = offset - 32;

            lseek(fatFile_fp, offset + 20, SEEK_SET);
            read(fatFile_fp, byteBuff, 2);
            lseek(fatFile_fp, offset + 26, SEEK_SET);
            read(fatFile_fp, byteBuff + 2, 2);
            tempDirEntry->DIR_DataCluster = byteBuff[0] << 24;
            tempDirEntry->DIR_DataCluster |= byteBuff[1] << 16;
            tempDirEntry->DIR_DataCluster |= byteBuff[3] << 8;
            tempDirEntry->DIR_DataCluster |= byteBuff[2];

            if (tempDirEntry->DIR_DataCluster == 0)
                tempDirEntry->DIR_DataCluster = info->RootClus;

            lseek(fatFile_fp, offset + 28, SEEK_SET);
            read(fatFile_fp, &tempDirEntry->DIR_fileSize, 4);

            dirEntryArray = (struct DIRENTRY **) realloc(dirEntryArray, sizeof(struct DIRENTRY *) * (numDirEntries + 2));
            dirEntryArray[numDirEntries] = tempDirEntry;
            numDirEntries++;
        }

        offset += entryByteOffset;
    }

    dirEntryArray[numDirEntries] = NULL;
    return dirEntryArray;
}

//Sets a cluster as available
//NOTE: MAKE SURE THE BPBInfo HAS BEEN INITIALIZED USING THE BPBInfoInit FUNCTION
void _setClusterAsAvailable(unsigned int cluster, int fatFile_fp, struct BPBInfo* info)
{
    unsigned int offset = (info->RsvdSecCnt * info->BytesPerSec) + (cluster * 4);
    unsigned char byteBuff[4];

    memset(byteBuff, 0, 4);

    lseek(fatFile_fp, offset, SEEK_SET);
    write(fatFile_fp, byteBuff, 4);
}

//Calculates and returns the next cluster after the passed cluster or NULL if the passed
//cluster was the last one
//NOTE: MAKE SURE THE BPBInfo HAS BEEN INITIALIZED USING THE BPBInfoInit FUNCTION
unsigned int* _getNextCluster(unsigned int cluster, int fatFile_fp, struct BPBInfo* info)
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
    if (*entry >= 0x0FFFFFF8)
        return NULL;
    else
        return entry;
}

//Replaces the contents of a cluster with all 0s
//NOTE: MAKE SURE THE BPBInfo HAS BEEN INITIALIZED USING THE BPBInfoInit FUNCTION
void _removeClusterData(unsigned int cluster, int fatFile_fp, struct BPBInfo* info)
{
    const unsigned int firstDataSector = info->RsvdSecCnt + (info->NumFATs * info->FATSize);
    unsigned int byteOffset;
    unsigned char byteBuff[info->BytesPerSec * info->SecPerClus];

    //initializes byte buffer to all 0's
    memset(byteBuff, 0, info->BytesPerSec * info->SecPerClus);

    byteOffset = (firstDataSector + ((cluster - 2) * info->SecPerClus)) * info->BytesPerSec;

    lseek(fatFile_fp, byteOffset, SEEK_SET);
    write(fatFile_fp, byteBuff, info->BytesPerSec * info->SecPerClus);
}

//gets the byte offset of the passed cluster
unsigned int getByteOffsetFromCluster(unsigned int cluster, struct BPBInfo* info)
{
    const unsigned int firstDataSector = info->RsvdSecCnt + (info->NumFATs * info->FATSize);
    return (firstDataSector + ((cluster - 2) * info->SecPerClus)) * info->BytesPerSec;
}

//Deallocates a NULL TERMINATED array of direntries
void _freeDirEntryArray(struct DIRENTRY** entries)
{
    for (int i = 0; entries[i] != NULL; i++)
        free(entries[i]);
    free(entries);
}

//Deallocates a linked list
void _freeOpenFileLL(struct openFile* files)
{
    struct openFile* i = files;
    struct openFile* temp;
    while (i != NULL)
    {
        temp = i->next;
        free(i->entry);
        free(i);
        i = temp;
    }
}

int nextEmptyClus(int fatFile_fp, struct BPBInfo* info)
{
    
    unsigned int tempClus = info->RootClus;
    unsigned int clusValue;
    
    do{
        tempClus++;
        pread(fatFile_fp, &clusValue, 4, info->RsvdSecCnt * info->BytesPerSec + tempClus * 4);

    }while(clusValue != 0);
    
    return tempClus;   
}

void createEmptyDirEntry(int fatFile_fp, unsigned int offSet){
    struct DIRENTRY temp;
    
    int i;
    
    for( i = 0; i < 11; i++){
        temp.DIR_name[i] = 0;
    }
    
    temp.DIR_Attributes = 0;
    temp.DIR_FstClusHI = 0;
    temp.DIR_FstClusLO = 0;
    temp.DIR_fileSize = 0;
    temp.DIR_DataCluster = 0;
    temp.DIR_EntryByteOffset = 0;
    
    pwrite(fatFile_fp, &temp, 32, offSet);
}

void addFile(struct openFile* head, struct openFile* ptr)
{
    if (!strcmp(head->entry->DIR_name, ""))
    {
        memcpy(head->entry, ptr->entry, sizeof(struct DIRENTRY));
        head->mode = ptr->mode;
        head->lseekOffset = 0;
        return;
    }

    struct openFile* temp = (struct openFile*) malloc(sizeof(struct openFile));
    temp->entry = (struct DIRENTRY*) malloc(sizeof(struct DIRENTRY));
    struct openFile *i = head;
    while(i->next != NULL)
        i = i->next;
    memcpy(temp->entry, ptr->entry, sizeof(struct DIRENTRY));
    temp->mode = ptr->mode;
    temp->lseekOffset = 0;
    temp->previous = i;
    temp->next = NULL;
    i->next = temp;
}

int OpenFile(struct DIRENTRY* entry, struct openFile* head)
{
        for(struct openFile* ptr = head; ptr != NULL; ptr = ptr->next)
        {
            if(!strcasecmp(ptr->entry->DIR_name, entry->DIR_name) && ptr->entry->DIR_DataCluster == entry->DIR_DataCluster)
                return 1;
        }
        return 0;
}


