#include "creatt.h"
#include <stdio.h>
#include <string.h>

void creatt(unsigned int pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info)
{
    struct DIRENTRY dirEntryArray;
    unsigned int eofValue = 0x00000000;
    unsigned int tempCluster;
    int FAT = info->BytesPerSec * ((info->NumFATs * info->FATSize) + info->RsvdSecCnt);

    tempCluster = nextEmptyClus(fatFile_fp, info);

    write(fatFile_fp, &eofValue, 4, info->RsvdSecCnt * info->BytesPerSec + tempCluster * 4);

    int i;
    for(i=0; i < strlen(dirName); i++)
    {
        dirEntryArray.DIR_name[i] = dirName[i];
    }

    for(i = strlen(dirName); i < 8; i++)
    {
        dirEntryArray.DIR_name[i] = 32;
    }

    unsigned int byteOffSet = info->BytesPerSec * (FAT + (tempCluster - 2) * info->SecPerClus);
    dirEntryArray.DIR_FstClusLO = 0xFFFF & tempCluster;
    dirEntryArray.DIR_FstClusHI = (tempCluster >> 16) & 0xFFFF;
    dirEntryArray.DIR_fileSize = 0;
    dirEntryArray.DIR_EntryByteOffset = 0;

    byteOffSet = info->BytesPerSec * (FAT + ( pwdStartCluster - 2) * info->SecPerClus);
    struct DIRENTRY temp;

    while(1)
    {
        read(fatFile_fp, &temp, sizeof(temp), byteOffSet);
        if(temp.DIR_name[0] == 0)
        {
            write(fatFile_fp, &dirEntryArray, sizeof(dirEntryArray), byteOffSet);
            return;
        }
        byteOffSet = byteOffSet + 32;
    };
}