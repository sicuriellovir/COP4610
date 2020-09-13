#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pathSearch.c"
#include "tildeExpansion.c"
#include "functions.h"

typedef struct {
    int size;
    char **items;
} tokenlist;

typedef struct {
    char* PWD;
    char* USER;
    char* MACHINE;
    char* HOME;
    char* PATH;
} eVars;

eVars* get_eVars();
void process_tokens();
void externCmdExec(char* pathToCmd, tokenlist* cmdArgs);

char *get_input(void);
tokenlist *get_tokens(char *input);

tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);
void outputRedirection(tokenlist *tokens);
void inputRedirection(tokenlist* tokens);
void redirection(tokenlist *tokens);


int main()
{
    char **command = NULL;  // holds commands (array of strings)
    int commandSize = 0;
    eVars* vars = get_eVars();
    while (1) {
        printf("%s@%s : %s> ", vars->USER, vars->MACHINE, vars->PWD);

        /* input contains the whole command
         * tokens contains substrings from input split by spaces
         */
        char *input = get_input();
        printf("whole input: %s\n", input);

        tokenlist *tokens = get_tokens(input);

        process_tokens(tokens, vars);

        command = tokens->items;
        commandSize = tokens->size;

        //built-in
        if(builtin(command) == 1)
        {
            if(builtinExecution(command, commandSize) == 1)
            {
                commandSize++;
            }
            else
            {
                printf("Error executing builtin command\n");
            }
        }
        //redirection
        else
        {
            commandSize++;
            redirection(tokens);
        }

        free(input);
        free_tokens(tokens);
    }

    return 0;
}

//returns the necessary environment variables in c-string format
eVars* get_eVars()
{
    eVars* vars = (eVars *) malloc(sizeof(eVars));
    vars->PWD = getenv("PWD");
    vars->MACHINE = getenv("MACHINE");
    vars->USER = getenv("USER");
    vars->HOME = getenv("HOME");
    vars->PATH = getenv("PATH");

    return vars;
}

//need to loop through tokens and process them. As of now, this just demonstrates
//the external command execution
void process_tokens(tokenlist* tokens, eVars* vars)
{
    char* result = pathSearch(tokens->items[0], vars->PATH);
    if (result != NULL)
    {
        tokenlist* args = new_tokenlist();
        for (int i = 1; i < tokens->size; i++)
            add_token(args, tokens->items[i]);

        printf("%s is a valid executable. Attempting to run...\n", tokens->items[0]);
        externCmdExec(result, args);
        free_tokens(args);
    }
    else
        printf("%s is not a valid executable.\n", tokens->items[0]);

    free(result);
}

//executes an external command. The pathToCmd parameter must be the full
//path to the command as a null-terminated c-string. The cmdArgs parameter
//must be a tokenlist in which each token is an argument.
void externCmdExec(char* pathToCmd, tokenlist* cmdArgs)
{
    char** args;
    //runs if there were no cmdArgs passed
    if (cmdArgs == NULL || cmdArgs->size == 0)
    {
        args = (char**) malloc(2 * sizeof(char*));
        args[0] = malloc(strlen(pathToCmd) + 1);
        strcpy(args[0], pathToCmd);
        args[1] = NULL;
    }
        //runs if there was at least one argument in cmdArgs
    else
    {
        args = (char**) malloc((cmdArgs->size + 2) * sizeof(char*));
        args[0] = malloc(strlen(pathToCmd) + 1);
        strcpy(args[0], pathToCmd);
        args[cmdArgs->size + 1] = NULL;
        for (int i = 0; i < cmdArgs->size; i++)
        {
            args[i + 1] = malloc(strlen(cmdArgs->items[i]) + 1);
            strcpy(args[i + 1], cmdArgs->items[i]);
        }
    }

    pid_t pid = fork();

    //runs if child process could not be created
    if (pid == -1)
        fprintf(stderr, "Error creating child process.\n");
        //runs in child process. Executes cmd
    else if (pid == 0)
        execv(pathToCmd, args);
        //runs in parent process. Waits for cmd execution to finish
    else
        waitpid(pid, NULL, 0);

    //loops through args (array of c-strings) and deallocates each string
    for (int i = 0; i < cmdArgs->size + 2; i++)
        free(args[i]);
}

tokenlist *new_tokenlist(void)
{
    tokenlist *tokens = (tokenlist *) malloc(sizeof(tokenlist));
    tokens->size = 0;
    tokens->items = (char **) malloc(sizeof(char *));
    tokens->items[0] = NULL; /* make NULL terminated */
    return tokens;
}

void add_token(tokenlist *tokens, char *item)
{
    int i = tokens->size;

    tokens->items = (char **) realloc(tokens->items, (i + 2) * sizeof(char *));
    tokens->items[i] = (char *) malloc(strlen(item) + 1);
    tokens->items[i + 1] = NULL;
    strcpy(tokens->items[i], item);

    tokens->size += 1;
}

char *get_input(void)
{
    char *buffer = NULL;
    int bufsize = 0;

    char line[5];
    while (fgets(line, 5, stdin) != NULL) {
        int addby = 0;
        char *newln = strchr(line, '\n');
        if (newln != NULL)
            addby = newln - line;
        else
            addby = 5 - 1;

        buffer = (char *) realloc(buffer, bufsize + addby);
        memcpy(&buffer[bufsize], line, addby);
        bufsize += addby;

        if (newln != NULL)
            break;
    }

    buffer = (char *) realloc(buffer, bufsize + 1);
    buffer[bufsize] = 0;

    return buffer;
}

tokenlist *get_tokens(char *input)
{
    char *buf = (char *) malloc(strlen(input) + 1);
    strcpy(buf, input);

    tokenlist *tokens = new_tokenlist();

    char *tok = strtok(buf, " ");
    while (tok != NULL) {
        add_token(tokens, tok);
        tok = strtok(NULL, " ");
    }

    free(buf);
    return tokens;
}

void free_tokens(tokenlist *tokens)
{
    for (int i = 0; i < tokens->size; i++)
        free(tokens->items[i]);

    free(tokens);
}

void outputRedirection(tokenlist* tokens)
{
    int i;
    int operator = 0;

    for (i = 0; i < (tokens->size)-1; ++i)
    {
        if (strcmp((tokens->items)[i], ">") != 0)
            continue;
        operator = i;
    }

    if (operator != tokens->size)
    {
        if (tokens->size >= 3)
        {
            tokenlist t;
            t.items = NULL;
            t.size = 0;
            int j = 0;
            while (j < operator)
            {
                add_token(&t, (tokens->items)[j]);
                ++j;
            }

            t.items = (char **) (t.size == 0 ? malloc(sizeof(char *)) : realloc(t.items,(t.size + 1) * sizeof(char *)));

            t.items[t.size] = (char *) NULL;
            t.size++;

            char fileName[256];
            strcpy(fileName, (tokens->items)[tokens->size - 2]);
            int file = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);

            if (file < 0)
            {
                printf("Error Opening Output File.\n");
                exit(1);
            }
        }
        else
            printf("Error With Output Redirection.\n");
    }
    else
        printf("Missing Name For Redirection.\n");
}

void inputRedirection(tokenlist* tokens)
{
    int i = 0;
    int operator = 0;
    while (i < (tokens->size)-1)
    {
        if (strcmp((tokens->items)[i], "<") != 0)
            continue;
        operator = i;
        i++;
    }

    if (operator != tokens->size) 
    {
        if (tokens->size >= 3)
        {
            tokenlist t;
            t.items = NULL;
            t.size = 0;
            int j = 0;
            while (j < operator)
            {
                add_token(&t, (tokens->items)[j]);
                ++j;
            }

            t.items = (char **) (t.size == 0 ? malloc(sizeof(char *)) : realloc(t.items,(t.size + 1) * sizeof(char *)));

            t.items[t.size] = (char *) NULL;
            t.size++;

            //read
            char fileName[256];
            strcpy(fileName, (tokens->items)[tokens->size - 2]);
            int file = open(fileName, O_RDONLY);

            if (file < 0) 
            {
                printf("Error Opening Input File.\n");
                exit(1);
            }
        } 
        else
            printf("Error With Input Redirection.\n");
    } 
    else
        printf("Missing Name For Redirection.\n");
}

void redirection(tokenlist* tokens)
{
    int i = 0;
    char temp = 0;

    while (i < (tokens->size)-1)
    {
        if (strcmp((tokens->items)[i], ">") == 0)
            temp = '>';
        else
        {
            if (strcmp((tokens->items)[i], "<") == 0)
                temp = '<';
        }
        ++i;
    }

    if (temp == '<')
        inputRedirection(tokens);
    else if (temp == '>')
        outputRedirection(tokens);
    
}
