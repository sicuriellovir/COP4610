#ifndef RMDIRR_H
#define RMDIRR_H

#include "helpers.h"

void _removeDir(struct DIRENTRY* entry, int fatFile_fp, struct BPBInfo* info);
void rmdirr(unsigned int pwdStartCluster, char* fileName, int fatFile_fp, struct BPBInfo* info);

#endif
