#ifndef RM_H
#define RM_H

#include "helpers.h"

void rm(unsigned int pwdStartCluster, char* fileName, int fatFile_fp, struct BPBInfo* info);
void _removeFile(struct DIRENTRY* entry, int fatFile_fp, struct BPBInfo* info);

#endif