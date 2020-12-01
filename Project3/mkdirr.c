#include "mkdirr.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void mkdirr(unsigned int pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info)
{
    struct DIRENTRY dirEntryArray;
    int tempCluster = pwdStartCluster;
    int i = 0;
    int nCluster;
    int ifspace;
    int atOffset;

    while (true)
    {
        ifspace = 0;
        i = 0;
        for (;i*sizeof(dirEntryArray) < info->BytesPerSec;i++)
        {
            int offset = getByteOffsetFromCluster(pwdStartCluster,info) + i*sizeof(dirEntryArray);

            lseek(fatFile_fp, offset, SEEK_SET);
            pread(&dirEntryArray, sizeof(struct DIRENTRY), 1, fatFile_fp);

            if (dirEntryArray.DIR_name[0] == 0xE5 || dirEntryArray.DIR_name[0] == 0x00)
            {
                ifspace = 1;
                atOffset = offset;
                break;
            }
        }

        if(ifspace == 1)
            break;

        lseek(fatFile_fp, 0x4000 + (4*pwdStartCluster), SEEK_SET);
        pread(&nCluster, sizeof(int), 1, fatFile_fp);

        if (nCluster != 0x0FFFFFF8 && nCluster != 0x0FFFFFFF && nCluster != 0x00000000)
            pwdStartCluster = nCluster;
        else
        {
            ifspace = 0;
            break;
        }
    }

    if(ifspace == 1)
    {
        struct DIRENTRY temp;

        strcpy(temp.DIR_name, dirName);
        temp.DIR_Attributes = 0x10;

        temp.DIR_fileSize = 0;

        int empty = nextEmptyClus(fatFile_fp, info);
        temp.DIR_FstClusHI = empty / 0x100;
        temp.DIR_FstClusLO = empty % 0x100;

        lseek(fatFile_fp, atOffset, SEEK_SET);
        pwrite(&temp, 1, sizeof(struct DIRENTRY), fatFile_fp);
    }
    else
    {
        struct DIRENTRY addDir;
        int t = nextEmptyClus(fatFile_fp, info);

        int rootDir = 0x0FFFFFF8;

        if(t != -1)
        {
            lseek(fatFile_fp, 0x4000 + (4 * t), SEEK_SET);
            pwrite(&rootDir, 1, sizeof(int), fatFile_fp);

            lseek(fatFile_fp, 0x4000 + (4 * tempCluster), SEEK_SET);
            pwrite(&t, 1, sizeof(int), fatFile_fp);

            int newOffset = getByteOffsetFromCluster(t, info);

            strcpy(addDir.DIR_name, dirName);
            addDir.DIR_Attributes = 0x10;
            addDir.DIR_fileSize = 0;

            int newEmpty = nextEmptyClus(fatFile_fp, info);
            addDir.DIR_FstClusHI = newEmpty / 0x100;
            addDir.DIR_FstClusLO = newEmpty % 0x100;

            lseek(fatFile_fp, newOffset, SEEK_SET);
            pwrite(&addDir, 1, sizeof(struct DIRENTRY), fatFile_fp);
        }
    }
}
