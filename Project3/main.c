#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "ls.h"
#include "cd.h"
#include "rm.h"
#include "lseek.h"
#include "read.h"

char** getTokens(char* str);
void freeTokenArray(char** tokenArray);

int main() {
    //character buffer for the FAT32 filename
    char filename[51];
    //character buffer for user input (1024 chars plus null char)
    char input[1025] = "\0";
    //holds the STARTING cluster number of the current directory
    unsigned int pwdStartCluster;
    //file pointer for the FAT32 image file
    int imgFile;
    //NULL TERMINATED array of openFiles to keep track of the files that are open and an int
    //to keep track of the number of open files
    struct openFile** openFiles = (struct openFile**) malloc(sizeof(struct openFile*));
    unsigned int numOpenFiles = 0;
    openFiles[0] = NULL;

    printf("Enter image filename (up to 50 chars): ");

    //gets input from user (max 50 characters) and puts it in filename buffer
    scanf("%50[^\n]", filename);
    getchar();

    //open the file for reading and writing
    imgFile = open(filename, O_RDWR);
    if (imgFile == -1)
    {
        printf("Error: Could not open file %s\n", filename);
        close(imgFile);
        return 0;
    }

    struct BPBInfo fileInfo;
    BPBInfoInit(&fileInfo, imgFile);
    pwdStartCluster = fileInfo.RootClus;

    char** tokenArray;

    while (strncmp(input, "exit", 1024) != 0)
    {
        input[0] ='\0';
        printf("$ ");

        //gets input from user (max 1024 characters) and puts it in input buffer
        scanf("%1024[^\n]", input);
        getchar();

        tokenArray = getTokens(input);

        if (tokenArray[0] == NULL) {}
        else if (!strcmp(tokenArray[0], "exit"))
        {
            //call exit
        }
        else if (!strcmp(tokenArray[0], "info"))
        {
            //call info
        }
        else if (!strcmp(tokenArray[0], "size"))
        {
            //call size
        }
        else if (!strcmp(tokenArray[0], "ls"))
            ls(pwdStartCluster, tokenArray[1], imgFile, &fileInfo);
        else if (!strcmp(tokenArray[0], "cd"))
            cd(&pwdStartCluster, tokenArray[1], imgFile, &fileInfo);
        else if (!strcmp(tokenArray[0], "creat"))
        {
            //call creat
        }
        else if (!strcmp(tokenArray[0], "mkdir"))
        {
            //call mkdir
        }
        else if (!strcmp(tokenArray[0], "mv"))
        {
            //call mv
        }
        else if (!strcmp(tokenArray[0], "open"))
        {
            //call open
        }
        else if (!strcmp(tokenArray[0], "close"))
        {
            //call close
        }
        else if (!strcmp(tokenArray[0], "lseek"))
        {
            setLseek(openFiles, tokenArray[1], atoi(tokenArray[2]));
        }
        else if (!strcmp(tokenArray[0], "read"))
            readFile(openFiles, tokenArray[1], atoi(tokenArray[2]), imgFile, &fileInfo);
        else if (!strcmp(tokenArray[0], "write"))
        {
            //call write
        }
        else if (!strcmp(tokenArray[0], "rm"))
            rm(pwdStartCluster, tokenArray[1], imgFile, &fileInfo);
        else if (!strcmp(tokenArray[0], "cp"))
        {
            //call cp
        }
        else if (tokenArray[0] != NULL)
            printf("Error: %s is an unknown command\n", tokenArray[0]);

        freeTokenArray(tokenArray);
    }

    _freeOpenFileArray(openFiles);
    close(imgFile);
    return 0;
}

char** getTokens(char* str)
{
    char* temp;
    char* buf = (char*) malloc(strlen(str) + 1);
    char** tokens = (char**) malloc(sizeof(char*));
    tokens[0] = NULL;
    strcpy(buf, str);

    temp = strtok(buf, " ");
    for (int i = 0; temp != NULL; i++)
    {
        tokens = (char**) realloc(tokens, sizeof(char*) * (i + 2));
        tokens[i] = (char*) malloc(sizeof(char) * (strlen(temp) + 1));
        strcpy(tokens[i], temp);
        tokens[i + 1] = NULL;
        temp = strtok(NULL, " ");
    }

    free(buf);
    return tokens;
}

void freeTokenArray(char** tokenArray)
{
    for (int i = 0; tokenArray[i] != NULL; i++)
        free(tokenArray[i]);
    free(tokenArray);
}
