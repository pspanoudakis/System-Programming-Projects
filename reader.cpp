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


int main(int argc, char const *argv[])
{
    int pipe_fd = open("./test_fifo", O_RDWR);
    if (pipe_fd < 0)
    {
        perror("Error opening fifo");
        return 1;
    }
    char buffer[1];
    /*
    char *string1, *string2, *string3;
    char buffer[1];
    receiveString(pipe_fd, string1, buffer, 1);
    receiveString(pipe_fd, string2, buffer, 1);
    receiveString(pipe_fd, string3, buffer, 1);
    */
    BloomFilter *filter;
    unsigned int n;
    char msg_code = 0;
    while (msg_code != EXIT_MSG)
    {
        read(pipe_fd, &msg_code, sizeof(char));

        switch (msg_code)
        {
            case INT_TRANSFER:
                receiveInt(pipe_fd, n, buffer, 1);
                filter = new BloomFilter(n);
                break;
            case BLOOM_TRANSFER:
                updateBloomFilter(pipe_fd, *filter, buffer, 1);
                if (filter->isPresent((void*)"hello"))
                {
                    printf("hello\n");
                }
                if (filter->isPresent((void*)"world"))
                {
                    printf("world\n");
                }
                break;
        }        
    }
    delete filter;
    close (pipe_fd);/*
    printf("len: %lu\n", strlen(string1));
    printf("len: %lu\n", strlen(string2));
    printf("len: %lu\n", strlen(string3));
    printf("%s%s%s\n", string1, string2, string3);
    free(string1);
    free(string2);
    free(string3);*/
    return 0;
}
