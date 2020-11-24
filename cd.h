#ifndef CD_H
#define CD_H

#include "helpers.h"

void cd(unsigned int* pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info);

#endif
