#include <stdio.h>
#include <fcntl.h>
#include "ls.h"
#include "BPBInfo.h"

int main() {
    //character buffer for the FAT32 filename
    char filename[50];
    //c-string to hold the present working directory
    char pwd[] = "/";
    //holds the STARTING cluster number of the current directory
    unsigned int pwdStartCluster;
    //file pointer for the FAT32 image file
    int imgFile;

    printf("Enter image filename (up to 50 chars): ");
    scanf("%s", filename);

    //open the file for reading and writing
    imgFile = open(filename, O_RDWR);
    if (imgFile == -1)
    {
        printf("Error: Could not open file %s", filename);
        close(imgFile);
        return 0;
    }

    struct BPBInfo fileInfo;
    BPBInfoInit(&fileInfo, imgFile);

    printf("File listing for / (root directory)\n");
    ls(fileInfo.RootClus, NULL, imgFile, &fileInfo);
    printf("\nFile listing for /red\n");
    ls(fileInfo.RootClus, "red", imgFile, &fileInfo);

    close(imgFile);
    return 0;
}