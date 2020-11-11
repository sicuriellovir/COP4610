#include "ls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*Implementation for ls. Takes in a c-string for the present working directory path, a c-string holding
the name of the directory to perform ls on (if this is NULL, ls will be performed on the current directory),
the file pointer for the FAT32 file, and the BPBInfo for the file.*/
//NOTE: MAKE SURE THE BPBInfo HAS BEEN INITIALIZED USING THE BPBInfoInit FUNCTION
void ls(unsigned int pwdStartCluster, char* dirName, int fatFile_fp, struct BPBInfo* info)
{
    struct DIRENTRY** dirEntryArray = _getDirEntriesFromFirstCluster(pwdStartCluster, fatFile_fp, info);

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
                    ls(dirEntryArray[i]->DIR_DataCluster, NULL, fatFile_fp, info);
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
}