#include <stdlib.h>
#include <fcntl.h>
#include "exit.h"

//The only memory that needs to be freed in the current implementation is the openFile linked list. The file pointer
//for the fat32 image also needs to be closed, so they are passed to this and dealt with appropriately
void exitAllPrograms(struct openFile* head, int fatFile_fp)
{
	_freeOpenFileLL(head);
	close(fatFile_fp);
	exit(0);
}
