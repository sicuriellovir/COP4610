#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pathSearch.c"
#include "tildeExpansion.c"

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

char *get_input(void);
tokenlist *get_tokens(char *input);

tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

int main()
{
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

//need to loop through tokens and process them
void process_tokens(tokenlist* tokens, eVars* vars)
{

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
