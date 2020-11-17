#include "cd.h"
#include <stdio.h>
#include <string.h>

/*changes directory to the directory name specified in dirName. takes in the start cluster of the
current directory (will be changed if cd is successful), the directory name to cd to (if possible),
the fat32 file pointer, and the BPBInfo on the fat32 file. if NULL is passed in for dirName, this
function does nothing*/
//NOTE: MAKE SURE THE BPBInfo HAS BEEN INITIALIZED USING THE BPBInfoInit FUNCTION
void cd(unsigned int* pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info)
{
    if (dirName == NULL)
        return;

    struct DIRENTRY** dirEntryArray = _getDirEntriesFromAllClusters(*pwdStartCluster, fatFile_fp, info);

    //loops through directory entries
    for (int i = 0; dirEntryArray[i] != NULL; i++)
    {
        if (!strcasecmp(dirName, dirEntryArray[i]->DIR_name))
        {
            if (dirEntryArray[i]->DIR_Attributes == 0x10)
            {
                if (dirEntryArray[i]->DIR_DataCluster != 0)
                    *pwdStartCluster = dirEntryArray[i]->DIR_DataCluster;
                else
                    *pwdStartCluster = info->RootClus;
            }
            else
                printf("Error: %s is a file, not a directory\n", dirName);

            _freeDirEntryArray(dirEntryArray);
            return;
        }
    }
    printf("Error: %s not found in the current directory\n", dirName);
    _freeDirEntryArray(dirEntryArray);
}