#ifndef BPBINFO_H
#define BPBINFO_H

struct BPBInfo {
    unsigned short BytesPerSec;
    unsigned char SecPerClus;
    unsigned short RsvdSecCnt;
    unsigned char NumFATs;
    unsigned int FATSize;
    unsigned int RootClus;
    unsigned int TotSec;
};

void BPBInfoInit(struct BPBInfo* info, int fatFile_fp);

#endif
