#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char const *argv[])
{
    printf("I'm child 2. pid: %d\n", getpid());
    pause();
    return 0;
}
