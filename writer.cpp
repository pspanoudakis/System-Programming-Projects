#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "include/bloom_filter.hpp"
#include "pipe_msg.hpp"

#define BLOOM_TRANSFER 1
#define INT_TRANSFER 2
#define MSG_TYPE_3 3
#define MSG_TYPE_4 4
#define EXIT_MSG 5


void sigint_handler(int sigint)
{
    printf("I got called\n");
    signal(SIGINT, sigint_handler);
}

void sigchld_handler(int sigint, siginfo_t *siginfo, void *arg)
{
    printf("I got called\n");
}

int main(int argc, char const *argv[])
{
    if (mkfifo("./test_fifo", 0666) < 0 )
    {
        perror("Error creating fifo");
        return 1;
    }
    int pipe_fd = open("./test_fifo", O_RDWR);
    if (pipe_fd < 0)
    {
        perror("Error opening fifo");
        return 1;
    }
    char buffer[2];
    /*
    sendString(pipe_fd, "hello", buffer, 2);
    sendString(pipe_fd, " ", buffer, 2);
    sendString(pipe_fd, "world", buffer, 2);
    */
    
    unsigned int n = 512;
    BloomFilter filter(n);
    char msg_code;

    for (int i = 0; i < 3; i++)
    {
        switch (i)
        {
            case 0:
                msg_code = INT_TRANSFER;
                write(pipe_fd, &msg_code, sizeof(char));
                sendInt(pipe_fd, n, buffer, 2);
                break;
            case 1:
                filter.markAsPresent((void*)"hello");
                msg_code = BLOOM_TRANSFER;
                write(pipe_fd, &msg_code, sizeof(char));
                sendBloomFilter(pipe_fd, filter, buffer, 2);
                break;
            case 2:
                filter.markAsPresent((void*)"world");
                msg_code = BLOOM_TRANSFER;
                write(pipe_fd, &msg_code, sizeof(char));
                sendBloomFilter(pipe_fd, filter, buffer, 2);
                break;
        }
        if (filter.isPresent((void*)"hello"))
        {
            printf("hello\n");
        }
        if (filter.isPresent((void*)"world"))
        {
            printf("world\n");
        }
    }
    msg_code = EXIT_MSG;
    write(pipe_fd, &msg_code, sizeof(char));
    
    close (pipe_fd);
    /*
    if (unlink("./test_fifo") < 0)
    {
        perror("Failed to delete fifo");
        return 1;
    }
    */
    return 0;
}
