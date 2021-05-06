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
#include "app/app_utils.hpp"
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
    if (mkfifo("./write_fifo", 0666) < 0 )
    {
        perror("Error creating fifo");
        return 1;
    }
    if (mkfifo("./read_fifo", 0666) < 0 )
    {
        perror("Error creating fifo");
        return 1;
    }
    int write_pipe_fd = open("./write_fifo", O_WRONLY);
    if (write_pipe_fd < 0)
    {
        perror("Error opening fifo");
        return 1;
    }
    int read_pipe_fd = open("./read_fifo", O_RDONLY);
    if (write_pipe_fd < 0)
    {
        perror("Error opening fifo");
        return 1;
    }
    char buffer[3];
    char *answer;
    int buffer_size = 3;
    sendInt(write_pipe_fd, 3, buffer, buffer_size);
    sendLongInt(write_pipe_fd, 10000, buffer, buffer_size);

    sendShortInt(write_pipe_fd, 2, buffer, buffer_size);
    sendString(write_pipe_fd, "./a/France", buffer, buffer_size);
    sendString(write_pipe_fd, "./a/Greece", buffer, buffer_size);
    //sendMessageType(write_pipe_fd, SEARCH_STATUS, buffer, buffer_size);
    //sendInt(write_pipe_fd, 14, buffer, buffer_size);
    //receiveString(read_pipe_fd, answer, buffer, buffer_size);
    //printf("%s", answer);
    //free(answer);
    sendMessageType(write_pipe_fd, TRAVEL_REQUEST, buffer, buffer_size);
    sendInt(write_pipe_fd, 1, buffer, buffer_size);
    sendDate(write_pipe_fd, Date(6, 5, 2021), buffer, buffer_size);
    sendString(write_pipe_fd, "COVID-19", buffer, buffer_size);
    char request_result;
    receiveMessageType(read_pipe_fd, request_result, buffer, buffer_size);
    receiveString(read_pipe_fd, answer, buffer, buffer_size);
    printf("%s", answer);
    free(answer);
    close(read_pipe_fd);
    close(write_pipe_fd);
    unlink("./write_fifo");
    unlink("./read_fifo");
    return 0;
}
