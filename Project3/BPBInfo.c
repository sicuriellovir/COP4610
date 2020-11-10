#include "BPBInfo.h"
#include <stdio.h>
#include <fcntl.h>

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