#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "pipe_msg.hpp"
#include "include/linked_list.hpp"
#include "include/hash_table.hpp"
#include "include/utils.hpp"
#include "parent_monitor_utils.hpp"
#include "app/app_utils.hpp"
#include "app/parse_utils.hpp"

#define HASHTABLE_BUCKETS 10000         // Number of buckets for the Citizen Hash Table
#define MAX_BLOOM_SIZE 1000000          // Maximum Bloom Filter size allowed

int sigusr1_received = 0;
int fifo_pipe_queue_messages = 0;
bool terminate = false;

void sigusr1_handler(int s)
{
    sigusr1_received++;
}

void sigusr2_handler(int s)
{
    fifo_pipe_queue_messages++;
}

void sigkill_handler(int s)
{
    terminate = true;
}

void checkArgc(int argc)
{
    if (argc != 3)
    {
        perror("Invalid number of arguments given.\n");
        perror("Usage: ./monitor <read pipe path> <write pipe path>\n");
        exit(EXIT_FAILURE);
    }
}

void openPipes(int &read_pipe_fd, int &write_pipe_fd, char const *argv[])
{
    if ( (read_pipe_fd = open(argv[1], O_RDONLY)) < 0)
    {
        perror("Failed to open read pipe.\n");
    }
    if ( (write_pipe_fd = open(argv[2], O_WRONLY)) < 0)
    {
        perror("Failed to open read pipe.\n");
    }
}

void releaseResources(char *buffer, DirectoryInfo **directories, unsigned short int num_dirs,
                      HashTable *citizens, LinkedList *countries, LinkedList *viruses,
                      int read_pipe_fd, int write_pipe_fd)
{
    delete[] buffer;
    for (unsigned short i = 0; i < num_dirs; i++)
    {
        delete directories[i];
    }
    delete[] directories;

    close(read_pipe_fd);
    close(write_pipe_fd);
    delete countries;
    delete viruses;
    delete citizens;
}

int main(int argc, char const *argv[])
{
    checkArgc(argc);
    int read_pipe_fd, write_pipe_fd;
    openPipes(read_pipe_fd, write_pipe_fd, argv);

    // Getting buffer size & creating the buffer to use for pipe I/O
    unsigned int buffer_size;
    char temp_buffer[sizeof(unsigned int)];
    receiveInt(read_pipe_fd, buffer_size, temp_buffer, sizeof(unsigned int));
    char *buffer = new char[buffer_size];

    // Read Bloom Filter size
    unsigned long bloom_size;
    receiveLongInt(read_pipe_fd, bloom_size, buffer, buffer_size);
    // Read directory paths
    unsigned short int num_dirs;
    receiveShortInt(read_pipe_fd, num_dirs, buffer, buffer_size);

    DirectoryInfo **directories = new DirectoryInfo*[num_dirs];
    char *str;
    for (unsigned short i = 0; i < num_dirs; i++)
    {
        str = NULL;
        receiveString(read_pipe_fd, str, buffer, buffer_size);
        directories[i] = new DirectoryInfo(str);
        directories[i]->addContents();
        free(str);
    }

    HashTable *citizens = new HashTable(HASHTABLE_BUCKETS, delete_object<CitizenRecord>, citizenHashObject);
    LinkedList *countries = new LinkedList(delete_object<CountryStatus>);
    LinkedList *viruses = new LinkedList(delete_object<VirusRecords>);

    int citizen_id, age;
    char *citizen_name, *country_name, *virus_name;
    bool vaccinated;
    Date date;

    while (!terminate)
    {
        if (sigusr1_received == 0 && fifo_pipe_queue_messages == 0)
        {
            pause();
        }
        if (sigusr1_received > 0)
        {
            // rescandir, get new data
            sigusr1_received--;
        }
        if (fifo_pipe_queue_messages > 0)
        {
            // get message
            fifo_pipe_queue_messages--;
        }
    }
    releaseResources(buffer, directories, num_dirs, citizens, countries, viruses, read_pipe_fd, write_pipe_fd);
    return 0;
}
