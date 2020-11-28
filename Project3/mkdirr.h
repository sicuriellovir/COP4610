#ifndef MKDIRR_H
#define MKDIRR_H
#include "helpers.h"

void mkdirr(unsigned int pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info);
#endif