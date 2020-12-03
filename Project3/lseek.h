#ifndef LSEEK_H
#define LSEEK_H

#include "helpers.h"

void setLseek(unsigned int pwdStartCluster, struct openFile* files, char* fileName, unsigned int offset);

#endif
