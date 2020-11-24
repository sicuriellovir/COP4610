#include "ls.h"
#include <stdio.h>
#include <string.h>

/*Implementation for ls. Takes in a c-string for the present working directory path, a c-string holding
the name of the directory to perform ls on (if this is NULL, ls will be performed on the current directory),
the file pointer for the FAT32 file, and the BPBInfo for the file.*/
//NOTE: MAKE SURE THE BPBInfo HAS BEEN INITIALIZED USING THE BPBInfoInit FUNCTION
void ls(unsigned int pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info)
{
    struct DIRENTRY** dirEntryArray = _getDirEntriesFromAllClusters(pwdStartCluster, fatFile_fp, info);

    if (dirName == NULL)
    {
        //prints entries in the current directory
        for (int i = 0; dirEntryArray[i] != NULL; i++)
            printf("%s\n", dirEntryArray[i]->DIR_name);

        _freeDirEntryArray(dirEntryArray);
        return;
    }
    else
    {
        //loops through directory entries
        for (int i = 0; dirEntryArray[i] != NULL; i++)
        {
            if (!strcasecmp(dirName, dirEntryArray[i]->DIR_name))
            {
                if (dirEntryArray[i]->DIR_Attributes == 0x10)
                    ls(dirEntryArray[i]->DIR_DataCluster, NULL, fatFile_fp, info);
                else
                    printf("Error: %s is a file, not a directory\n", dirName);

                _freeDirEntryArray(dirEntryArray);
                return;
            }
        }
        printf("Error: %s not found in the current directory\n", dirName);
        _freeDirEntryArray(dirEntryArray);
    }
}