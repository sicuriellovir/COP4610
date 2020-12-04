#include "mvv.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>


void mvv(unsigned int* pwdStartCluster, char* FROM, char*TO, int fatFile_fp, struct BPBInfo* info)
{   
    struct DIRENTRY** dirEntryArray = _getDirEntriesFromAllClusters(*pwdStartCluster, fatFile_fp, info);
    struct DIRENTRY** dirEntryArray1 = _getDirEntriesFromAllClusters(*pwdStartCluster, fatFile_fp, info);
    struct DIRENTRY** dirEntryArray2 = _getDirEntriesFromAllClusters(*pwdStartCluster, fatFile_fp, info);

  // "." & ".." FROM
    if(pwdStartCluster != info->RootClus) 
    {
        if(!strcmp(FROM, ".") || !strcmp(FROM, "..")) 
        {
            printf("Invalid file\n");
            return;
        }
        if(!strcmp(TO, ".")) 
            return;    
    }

    for (int i = 0; dirEntryArray1[i] != NULL; i++)
    {
        for (int i = 0; dirEntryArray2[i] != NULL; i++)
        {
            if (!strcasecmp(TO, dirEntryArray1[i]->DIR_name))
            {
                //if TO is a directory
                if (dirEntryArray[i]->DIR_Attributes == 0x10)
                {
                    //Find the DIRENTRY corresponding to FROM in the current directory
                    //*pwdStartCluster = dirEntryArray2[i]->DIR_DataCluster; 
                    //Allocate a new DIRENTRY under TO directory
                    //Copy the contents of the previously found DIRENTRY
                    //• Set the original DIRENTRY as empty
                    //• First byte as 0x0 if last entry
                    //• First byte as 0xE5 otherwise
                }
                   
                else
                {
                    //if TO is not a directory
                    //Change file/directory name
                    //• Only if TO does not exist
                    //• Find the DIRENTRY associated to TO
                    //• Set the field DIR_Name with the value set by TO
                }
                   
                _freeDirEntryArray(dirEntryArray);
                return;
            }
            else if (!strcasecmp(FROM, dirEntryArray2[i]->DIR_name))
            {
                if ((dirEntryArray2[i]->DIR_Attributes == 0x10) && dirEntryArray1[i]->DIR_Attributes != 0x10 )
                {
                    //FROM is a dir and TO is a file
                    printf( "Cannot move directory: invalid destination argument”\n");
                }

            }

        }
    }
    // printf("Error: %s not found in the current directory\n", dirName);
    // _freeDirEntryArray(dirEntryArray);
}
