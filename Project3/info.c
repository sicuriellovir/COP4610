#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "info.h"

void printInfo(struct BPBInfo* ptr)
{
    printf ("Bytes per Sector  : %u \n", ptr->BytesPerSec);
    printf ("Sectors per Cluster : %u \n", ptr->SecPerClus);
    printf ("Reserved sector count : %u \n", ptr->RsvdSecCnt);
    printf ("Number of Fats : %u \n", ptr->NumFATs);
    printf ("Total Sectors  : %u \n", ptr->TotSec);
    printf ("FAT Size  : %u \n", ptr->FATSize);
    printf ("Root Cluster  : %u \n", ptr->RootClus);
}
