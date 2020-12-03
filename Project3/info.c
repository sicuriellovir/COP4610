#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "helpers.h"

void printInfo(BPBInfo* ptr)
{
    printf ("Bytes per Sector  : %u", ptr->BytesPerSec);
    printf ("Sectors per Cluster : %u", ptr->SecPerClus);
    printf ("Reserved sector count : %u", ptr->RsvdSecCnt);
    printf ("Number of Fats : %u", ptr->NumFATs);
    printf ("Total Sectors  : %u", ptr->TotSec);
    printf ("FAT Size  : %u", ptr->FATSize);
    printf ("Root Cluster  : %u", ptr->RootClus);
}