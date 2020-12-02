#ifndef CMDOPEN_H
#define CMDOPEN_H

#include "helpers.h"

void cmdopen(unsigned int pwdStartCluster, char* dirName, char *mode, int fatFile_fp, struct BPBInfo* info, struct FileFAT* head, struct FileFAT* ptr);
#endif