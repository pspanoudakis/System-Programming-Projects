#include <unistd.h>

int main(int argc, char const *argv[])
{
    unlink("./test_fifo");
    return 0;
}
