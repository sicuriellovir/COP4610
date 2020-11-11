#include "cd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*changes directory to the directory name specified in dirName. takes in the start cluster of the
current directory (will be changed if cd is successful), the directory name to cd to (if possible),
 the fat32 file pointer, and the BPBInfo on the fat32 file. if NULL is passed in for dirName, this
 function does nothing*/
void cd(unsigned int* pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info)
{
    if (dirName == NULL)
        return;

    struct DIRENTRY** dirEntryArray = _getDirEntriesFromFirstCluster(*pwdStartCluster, fatFile_fp, info);

    //copies dirName to tempStr and converts it to uppercase
    char* tempStr = malloc((strlen(dirName) + 1) * sizeof(char));
    for (int i = 0; dirName[i] != '\0'; i++)
        tempStr[i] = toupper(dirName[i]);
    tempStr[strlen(dirName)] = '\0';

    //loops through directory entries
    for (int i = 0; dirEntryArray[i] != NULL; i++)
    {
        if (strncmp(tempStr, dirEntryArray[i]->DIR_name, strlen(dirEntryArray[i]->DIR_name)) == 0)
        {
            if (dirEntryArray[i]->DIR_Attributes == 0x10)
                *pwdStartCluster = dirEntryArray[i]->DIR_DataCluster;
            else
                printf("Error: %s is a file, not a directory\n", dirName);

            _freeDirEntryArray(dirEntryArray);
            free(tempStr);
            return;
        }
    }
    printf("Error: %s not found in the current directory\n", dirName);
    _freeDirEntryArray(dirEntryArray);
    free(tempStr);
}