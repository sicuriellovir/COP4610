#include <unistd.h>
int main()
{
    sleep(0); //syscall 1
    sleep(0); //syscall 2
    sleep(0); //syscall 3
    sleep(0); //syscall 4
	sleep(0); //syscall 5
    sleep(0); //syscall 6
    sleep(0); //syscall 7
}
