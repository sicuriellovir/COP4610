#ifndef CMDCLOSE_H
#define CMDCLOSE_H

#include "helpers.h"

void cmdclose(unsigned int pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info, struct openFile *head, struct openFile *ptr);
#endif
