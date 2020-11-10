#ifndef LS_H
#define LS_H

#include "BPBInfo.h"

struct DIRENTRY {
    unsigned char DIR_name[12];
    unsigned char DIR_Attributes;
    unsigned int DIR_DataCluster;
} __attribute__((packed));

void ls(unsigned int pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info);
struct DIRENTRY** _getDirEntriesFromFirstCluster(unsigned int firstCluster, int fatFile_fp, struct BPBInfo* info);
unsigned int* _isLastCluster(unsigned int cluster, int fatFile_fp, struct BPBInfo* info);
void _freeDirEntryArray(struct DIRENTRY** entries);

#endif
