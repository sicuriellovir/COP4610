#ifndef HELPERS_H
#define HELPERS_H

//holds data corresponding to the characteristics of a fat32 image
struct BPBInfo {
    unsigned short BytesPerSec;
    unsigned char SecPerClus;
    unsigned short RsvdSecCnt;
    unsigned char NumFATs;
    unsigned int FATSize;
    unsigned int RootClus;
    unsigned int TotSec;
};


//holds data corresponding to a directory entry
struct DIRENTRY {
    unsigned char DIR_name[12];
    unsigned char DIR_Attributes;
    unsigned int DIR_DataCluster;
    unsigned int DIR_EntryByteOffset;
    unsigned int DIR_fileSize;
    unsigned short DIR_FstClusHI;   
    unsigned short DIR_FstClusLO;
} __attribute__((packed));

//these are the different modes a file can be opened in
enum fileMode{READONLY, WRITEONLY, READANDWRITE};

//holds data corresponding to an open file. each file will have a DIRENTRY and the mode the file was opened in
struct openFile {
    struct DIRENTRY* entry;
    unsigned int lseekOffset;
    enum fileMode mode;
};

void BPBInfoInit(struct BPBInfo* info, int fatFile_fp);
struct DIRENTRY** _getDirEntriesFromAllClusters(unsigned int firstCluster, int fatFile_fp, struct BPBInfo* info);
struct DIRENTRY** _getDirEntriesFromCluster(unsigned int clusterNum, int fatFile_fp, struct BPBInfo* info);
unsigned int* _getNextCluster(unsigned int cluster, int fatFile_fp, struct BPBInfo* info);
void _setClusterAsAvailable(unsigned int cluster, int fatFile_fp, struct BPBInfo* info);
void _removeClusterData(unsigned int cluster, int fatFile_fp, struct BPBInfo* info);
void _freeDirEntryArray(struct DIRENTRY** entries);
void _freeOpenFileArray(struct openFile** files);
unsigned int getByteOffsetFromCluster(unsigned int cluster, struct BPBInfo* info);
int nextEmptyClus(int image, struct BPBInfo* info);



#endif
