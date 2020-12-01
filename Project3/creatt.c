#include "creatt.h"
#include <stdio.h>
#include <string.h>

void creatt(unsigned int pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info)
{
    struct DIRENTRY dirEntryArray;
    unsigned int tempCluster = pwdStartCluster;

    int i, ncluster, ifspace, atOffset = 0;

    while(1)
    {
        ifspace = 0;
        i = 0;
        for (;i * sizeof(dirEntryArray) < info->BytesPerSec; i++)
        {
            int offset = getByteOffsetFromCluster(pwdStartCluster, info) + i*sizeof(dirEntryArray);

            lseek(fatFile_fp, offset, SEEK_SET);
            pread(&dirEntryArray, sizeof(struct DIRENTRY), 1, fatFile_fp);

            if (dirEntryArray.DIR_name[0] != 0xE5 && dirEntryArray.DIR_name[0] != 0x00)
                continue;
            ifspace = 1;
            atOffset = offset;
            break;
        }

        if (ifspace != 1)
        {

            lseek(fatFile_fp, 0x4000 + (4 * pwdStartCluster), SEEK_SET);
            pread(&ncluster, sizeof(int), 1, fatFile_fp);

            if (ncluster == 0x0FFFFFF8 || ncluster == 0x0FFFFFFF || ncluster == 0x00000000) {
                ifspace = 0;
                break;
            }
            else
                pwdStartCluster = ncluster;
        }
        else
            break;
    }

    if (ifspace != 1)
    {
        struct DIRENTRY addDir;
        struct DIRENTRY um;
        int i = nextEmptyClus(fatFile_fp, info);

        int rootDir = 0x0FFFFFF8;
        if (i == -1)
            return;
        lseek(fatFile_fp, 0x4000 + (4 * i), SEEK_SET);
        pwrite(&rootDir, 1, sizeof(int), fatFile_fp);

        lseek(fatFile_fp, 0x4000 + (4 * tempCluster), SEEK_SET);
        pwrite(&i, 1, sizeof(int), fatFile_fp);

        int newOffset = getByteOffsetFromCluster(i, info);
        strcpy(addDir.DIR_name, dirName);
        addDir.DIR_Attributes = 0;
        addDir.DIR_fileSize = 0;

        int newEmpty = nextEmptyClus(fatFile_fp, info);
        addDir.DIR_FstClusHI = newEmpty / 0x100;
        addDir.DIR_FstClusLO = newEmpty % 0x100;

        lseek(fatFile_fp, newOffset, SEEK_SET);
        pwrite(&addDir, 1, sizeof(struct DIRENTRY), fatFile_fp);
    }
    else
    {
        struct DIRENTRY temp;

        strcpy(temp.DIR_name, dirName);
        temp.DIR_Attributes = 0;
        temp.DIR_fileSize = 0;

        int empty = nextEmptyClus(fatFile_fp, info);
        temp.DIR_FstClusHI = empty / 0x100;
        temp.DIR_FstClusLO = empty % 0x100;

        lseek(fatFile_fp, atOffset, SEEK_SET);
        pwrite(&temp, 1, sizeof(struct DIRENTRY), fatFile_fp);
    }
}
