#ifndef CMDCLOSE_H
#define CMDCLOSE_H

#include "helpers.h"

void cmdclose(unsigned int pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info, struct FileFAT *head, struct FileFAT *ptr);
#endif