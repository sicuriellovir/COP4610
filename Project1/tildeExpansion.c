#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* tildeExpand(char* token, char* home)
{
    int size = strlen(token);

    char* expandedToken = (char *) malloc(strlen(home) + size);
    strcpy(expandedToken, home);
    for (int i = 1; i < size; i++)
        strncat(expandedToken, &token[i], 1);

    printf("After expansion: %s\n", expandedToken);
    return expandedToken;
}