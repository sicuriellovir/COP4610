#include "cmdopen.h"
#include <stdio.h>
#include <string.h>

void cmdopen(unsigned int pwdStartCluster, char* dirName, char *mode, int fatFile_fp, struct BPBInfo* info, struct openFile* head, struct openFile* ptr)
{
    struct DIRENTRY tempDir;
    unsigned int tempCluster = pwdStartCluster;
    unsigned int byteOffSet;
    char* space = " ";
    int i;
    int FAT = info->RsvdSecCnt + (info->NumFATs * info->FATSize);

    for(i = strlen(dirName); i < 11; i++)
        strcat(dirName, space);


    if (strcmp(mode, "w") == 0 || strcmp(mode, "r") == 0 || strcmp(mode, "wr") == 0 || strcmp(mode, "rw") == 0)
    {
        if (!(OpenFile(dirName, head) == 1))
        {
            while (tempCluster < 0x0FFFFFF8)
            {
                byteOffSet = info->BytesPerSec * (FAT + (tempCluster - 2) * info->SecPerClus);
                do
                {
                    pread(fatFile_fp, &tempDir, sizeof(struct DIRENTRY), byteOffSet);
                    if (tempDir.DIR_Attributes != 15)
                    {
                        if (strncmp(tempDir.DIR_name, dirName, 11) == 0)
                        {

                            if (!(tempDir.DIR_Attributes & 0x10))
                            {
                                printf("File opened in %s mode \n", mode);
                                addFile(fatFile_fp, dirName, mode, head, ptr);
                            }
                            else 
                                printf("Error opening directory\n");
                            return;
                        }
                    }
                    byteOffSet = byteOffSet + 32;
                } while (tempDir.DIR_name[0] != 0);
                
                pread(fatFile_fp, &tempCluster, 4, info->RsvdSecCnt * info->BytesPerSec + tempCluster * 4);

            }
            printf("File not found\n");
        } 
    }
}
