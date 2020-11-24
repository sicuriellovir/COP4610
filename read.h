#ifndef READ_H
#define READ_H

#include "helpers.h"

void readFile(struct openFile** files, char* fileName, unsigned int size, int fatFile_fp, struct BPBInfo* info);

#endif
