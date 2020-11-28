#include "mkdirr.h"
#include <stdio.h>
#include <string.h>

void mkdirr(unsigned int pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info)
{
    struct DIRENTRY dirEntryArray;
    unsigned int eofValue = 0x0FFFFFF8;
    unsigned int tempCluster;
    int FAT = info->BytesPerSec * ((info->NumFATs * info->FATSize) + info->RsvdSecCnt);


    tempCluster = nextEmptyClus(fatFile_fp, info);

    write(fatFile_fp, &eofValue, 4, info->RsvdSecCnt * info->BytesPerSec + tempCluster * 4);

    int i;
    for(i = 0; i < strlen(dirName); i++)
        dirEntryArray.DIR_name[i] = dirName[i];
    

    for(i = strlen(dirName); i < 8; i++)
        dirEntryArray.DIR_name[i] = 32;
    

    unsigned int byteOffSet;
    dirEntryArray.DIR_FstClusLO = 0xFFFF & tempCluster;
    dirEntryArray.DIR_FstClusHI = (tempCluster >> 16) & 0xFFFF;
    dirEntryArray.DIR_Attributes = 0;
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
            break;
        }
        byteOffSet = byteOffSet + 32;
    }


    struct DIRENTRY parentDirEntry;

    strncpy(parentDirEntry.DIR_name, "..         ", 11);
    parentDirEntry.DIR_FstClusLO = 0xFFFF & pwdStartCluster;
    parentDirEntry.DIR_FstClusHI = (pwdStartCluster >> 16) & 0xFFFF;
    parentDirEntry.DIR_Attributes = 0x10;
    parentDirEntry.DIR_fileSize = 0;
    byteOffSet = info->BytesPerSec * (FAT + (tempCluster - 2) * info->SecPerClus);
    write(fatFile_fp, &parentDirEntry, sizeof(parentDirEntry), byteOffSet);

    strncpy(dirEntryArray.DIR_name, ".          ", 11);
    write(fatFile_fp, &dirEntryArray, sizeof(dirEntryArray), byteOffSet + 32);

}
