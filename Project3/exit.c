#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "exit.h"

void exitAllPrograms( void ** ptr) // pass the any open pointer you want to deallocate ...
{
	char *allocated = NULL;
    if (ptr) 
    {
        allocated = *ptr;
    }
    if (allocated)
    {
        free(allocated);
        *ptr = NULL;
    }
	exit();
}
