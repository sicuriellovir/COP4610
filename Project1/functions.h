#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

char builtin (char ** cmd);
int builtinExecution (char ** command, int numCommands, int commandsExecuted);
void cdcmd(char *path);
void exitcmd(int cmds);
void echocmd(char ** command);
void jobscmd(int b[], int b_size);

//part 10
char builtin (char ** cmd)
{
    int builtins = 4;
    char *builtstring[] = { "cd", "exit", "jobs", "echo", NULL};
    int i = 0;

    for (;i < builtins; i++)
    {
        if(strcmp(cmd[0], builtstring[i]) == 0)
            return 1;
    }
    return 0;
}

int builtinExecution (char ** command, int numCommands, int commandsExecuted)
{
    char *builtstring[] = { "cd", "exit", "jobs", "echo", NULL};
    // cd
    if(strcmp(command[0], builtstring[0]) == 0)
    {
        //work in progress..
        cdcmd(command[1]);
        return 1;
    }
        // exit
    else if(strcmp(command[0], builtstring[1]) == 0)
        exitcmd(commandsExecuted);
        //jobs
    else if(strcmp(command[0], builtstring[2]) == 0)
    {
        // we need part 9 to do jobs
        // call jobscmd(int b[], int b_size)
        return 1;
    }
        //echo
    else if(strcmp(command[0], builtstring[3]) == 0)
    {
        echocmd(command);
        return 1;
    }
    else
        return -1;

    return 0;
}

void cdcmd(char *path)
{
    //work in progress
    if(chdir(path) == 0)
    {
        if (path[0] != '/')
        {
            char* PWD = getenv("PWD");
            char* extendedPath = (char*) malloc((strlen(PWD) + strlen(path) + 2) * sizeof(char));
            strcpy(extendedPath, PWD);
            strcat(extendedPath, "/");
            strcat(extendedPath, path);
            setenv("PWD", extendedPath, 1);
        }
        else
            setenv("PWD", path, 1);
    }
    else
    {
        printf("%s: No such file or directory.\n", path);
    }
}

void exitcmd(int cmds)
{
    printf("Commands executed: %d\n", cmds);
    exit(0);
}

void echocmd(char ** command)
{

    char ** temp = command;
    temp++;
    int counter = 1;

    while (*temp != NULL)
    {
        char * i = *temp;

        if (strcmp(i, "$") != 0)
        {
            if (i[0] != '$')
                printf("%s ", command[counter]);
            else
            {
                char *c = *temp;
                char c2[strlen(c) - 1];
                strcpy(c2, &c[1]);
                char *s = c2;
                int j = 0;
                while (*s)
                {
                    c2[j] = toupper((unsigned char) *s);
                    ++s;
                    j++;
                }

                char *env = getenv(c2);

                env == NULL ? printf("%s", "[ERROR!] ") : printf("%s ", env);
            }
        }
        else
        {
            printf("%s ", command[counter]);
        }
        counter++;
        ++temp;
    }
    printf("\n");
}

void jobscmd(int b[], int b_size)
{
    int i = 0;
    for (i = 0; i < b_size; i++)
    {
        printf("%d \n", b[i]);
    }
}