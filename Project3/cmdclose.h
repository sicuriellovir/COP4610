#ifndef CMDCLOSE_H
#define CMDCLOSE_H

#include "helpers.h"

void cmdclose(char* fileName, struct openFile *head, unsigned int pwdStartCluster, int fatFile_fp, struct BPBInfo* info);
#endif
