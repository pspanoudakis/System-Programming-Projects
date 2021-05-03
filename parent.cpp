#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


void sigint_handler(int sigint)
{
    printf("I got called\n");
    signal(SIGINT, sigint_handler);
}

void sigchld_handler(int sigint, siginfo_t *siginfo, void *arg)
{
    printf("child %d died\n", siginfo->si_pid);
}

int main(int argc, char const *argv[])
{
    int pid1, pid2;
    struct sigaction handler;
    handler.sa_sigaction = &sigchld_handler;
    handler.sa_flags = SA_SIGINFO | SA_NOCLDWAIT;
    sigfillset(&(handler.sa_mask));
    sigaction(SIGCHLD, &handler, NULL);
    pid1 = fork();
    switch (pid1)
    {
        case -1:
            exit(EXIT_FAILURE);
        case 0:
            execl("./child1", "child1", NULL);
        default:
            /*parent*/
            pid2 = fork();
            switch (pid2)
            {
                case -1:
                    exit(EXIT_FAILURE);
                case 0:
                    /* child */
                    execl("./child2", "child2", NULL);
                default:
                    int i = 0;
                    while (i != 3)
                    {
                        sleep(30);
                        i++;
                    }
            }
    }
    return 0;
}
