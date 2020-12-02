#include "creatt.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

void creatt(unsigned int pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info)
{
    unsigned int *tempCluster = NULL, *currentCluster = &pwdStartCluster;
    unsigned char byteBuff[4];
    unsigned char byte;

    int i, ifspace = 0, atOffset = 0;

    while(currentCluster != NULL)
    {
        ifspace = 0;
        if (*currentCluster == info->RootClus)
            i = 0;
        else
            i = 1;
        for (;i * 64 < info->BytesPerSec * info->SecPerClus; i++)
        {
            unsigned int offset = getByteOffsetFromCluster(*currentCluster, info);

            lseek(fatFile_fp, offset + (i * 64), SEEK_SET);
            read(fatFile_fp, &byte, 1);

            if (byte != 0xE5 && byte != 0x00)
                continue;
            ifspace = 1;
            atOffset = offset + (i * 64);
            break;
        }

        if (ifspace != 1)
        {
            tempCluster = currentCluster;
            currentCluster = _getNextCluster(*currentCluster, fatFile_fp, info);
        }
        else
            break;
    }

    if (ifspace != 1)
    {
        int ncluster1 = nextEmptyClus(fatFile_fp, info);
        int ncluster2 = nextEmptyClus(fatFile_fp, info);

        int EOFVal = 0x0FFFFFF8;
        if (ncluster1 == -1 || ncluster2 == -1)
        {
            printf("Error: No more free clusters are available\n");
            return;
        }
        lseek(fatFile_fp, info->RsvdSecCnt * info->BytesPerSec + (4 * ncluster1), SEEK_SET);
        write(fatFile_fp, &EOFVal, 4);
        lseek(fatFile_fp, info->RsvdSecCnt * info->BytesPerSec + (4 * ncluster2), SEEK_SET);
        write(fatFile_fp, &EOFVal, 4);

        lseek(fatFile_fp, info->RsvdSecCnt * info->BytesPerSec + (4 * *tempCluster), SEEK_SET);
        write(fatFile_fp, ncluster1, 4);

        unsigned int newOffset = getByteOffsetFromCluster(ncluster1, info);
        byte = 0x41;
        lseek(fatFile_fp, newOffset, SEEK_SET);
        write(fatFile_fp, &byte, 1);

        lseek(fatFile_fp, newOffset + 32, SEEK_SET);
        write(fatFile_fp, dirName, strlen(dirName));

        byte = 0x20;
        for (i = 0; i < 11 - strlen(dirName); i++)
            write(fatFile_fp, &byte, 1);

        //byte = 0x10;
        write(fatFile_fp, &byte, 1);

        lseek(fatFile_fp, newOffset + 52, SEEK_SET);
        write(fatFile_fp, ncluster2 + 2, 2);

        lseek(fatFile_fp, newOffset + 58, SEEK_SET);
        write(fatFile_fp, ncluster2, 2);

        memset(byteBuff, 0, 4);
        lseek(fatFile_fp, newOffset + 60, SEEK_SET);
        write(fatFile_fp, byteBuff, 4);
    }
    else
    {
        int ncluster = nextEmptyClus(fatFile_fp, info);
        int EOFVal = 0x0FFFFFF8;

        if (ncluster == -1)
        {
            printf("Error: No more free clusters are available\n");
            return;
        }

        lseek(fatFile_fp, info->RsvdSecCnt * info->BytesPerSec + (4 * ncluster), SEEK_SET);
        write(fatFile_fp, &EOFVal, 4);

        byte = 0x41;
        lseek(fatFile_fp, atOffset, SEEK_SET);
        write(fatFile_fp, &byte, 1);

        lseek(fatFile_fp, atOffset + 32, SEEK_SET);
        write(fatFile_fp, dirName, strlen(dirName));

        byte = 0x20;
        for (i = 0; i < 11 - strlen(dirName); i++)
            write(fatFile_fp, &byte, 1);

        //byte = 0x10;
        write(fatFile_fp, &byte, 1);

        lseek(fatFile_fp, atOffset + 52, SEEK_SET);
        write(fatFile_fp, &ncluster + 2, 2);

        lseek(fatFile_fp, atOffset + 58, SEEK_SET);
        write(fatFile_fp, &ncluster, 2);

        memset(byteBuff, 0, 4);
        lseek(fatFile_fp, atOffset + 60, SEEK_SET);
        write(fatFile_fp, byteBuff, 4);
    }
}
