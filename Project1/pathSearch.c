#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//returns 0 if there is a valid executable file at path that we have
//permission to execute, 0 otherwise
int doesExecutableExist(char* path)
{
    if (access(path, X_OK) == 0)
        return 1;
    return 0;
}

//searches through the path (passed in as a c-string) for an executable
//with the name passed into cmd as a c-string. Returns 1 if found, 0 otherwise
int pathSearch(char* cmd, char* path)
{
    char *buf = (char *) malloc(strlen(path) + strlen(cmd) + 1);
    strcpy(buf, path);
    char *tok = strtok(buf, ":");
    while (tok != NULL) {
        char* temp = (char *) malloc(strlen(tok) + strlen(cmd) + 2);
        strcpy(temp, tok);
        strcat(temp, "/");
        strcat(temp, cmd);
        printf("Checking: %s\n", temp);
        if (doesExecutableExist(temp))
            return 1;
        tok = strtok(NULL, ":");
        free(temp);
    }
    free(buf);
    return 0;
}