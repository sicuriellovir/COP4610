#ifndef HELPERS_H
#define HELPERS_H

struct BPBInfo {
    unsigned short BytesPerSec;
    unsigned char SecPerClus;
    unsigned short RsvdSecCnt;
    unsigned char NumFATs;
    unsigned int FATSize;
    unsigned int RootClus;
    unsigned int TotSec;
};

struct DIRENTRY {
    unsigned char DIR_name[12];
    unsigned char DIR_Attributes;
    unsigned int DIR_DataCluster;
    unsigned int DIR_EntryByteOffset;
    unsigned int DIR_fileSize;
} __attribute__((packed));

void BPBInfoInit(struct BPBInfo* info, int fatFile_fp);
struct DIRENTRY** _getDirEntriesFromAllClusters(unsigned int firstCluster, int fatFile_fp, struct BPBInfo* info);
struct DIRENTRY** _getDirEntriesFromCluster(unsigned int clusterNum, int fatFile_fp, struct BPBInfo* info);
unsigned int* _isLastCluster(unsigned int cluster, int fatFile_fp, struct BPBInfo* info);
void _setClusterAsAvailable(unsigned int cluster, int fatFile_fp, struct BPBInfo* info);
void _removeClusterData(unsigned int cluster, int fatFile_fp, struct BPBInfo* info);
void _freeDirEntryArray(struct DIRENTRY** entries);

#endif
