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

int dir_update_notifications = 0;
int fifo_pipe_queue_messages = 0;
bool terminate = false;

void sigusr1_handler(int s)
{
    dir_update_notifications++;
    signal(SIGUSR1, sigusr1_handler);
}

void sigusr2_handler(int s)
{
    fifo_pipe_queue_messages++;
    signal(SIGUSR2, sigusr2_handler);
}

void sigint_handler(int s)
{
    terminate = true;
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigint_handler);
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

void scanAllFiles(DirectoryInfo **directories, unsigned short int num_dirs,
                        HashTable *citizens, LinkedList *countries, LinkedList *viruses,
                        unsigned long bloom_size)
{
    int citizen_id, age;
    char *citizen_name, *country_name, *virus_name;
    bool vaccinated;
    Date date;
    FILE *input_file;
    char *line_buf, *buf_copy, *temp;
    
    for (unsigned short i = 0; i < num_dirs; i++)
    {        
        LinkedList::ListIterator itr = directories[i]->contents->listHead();
        while (!itr.isNull())
        {
            char *file_path = new char[strlen(directories[i]->path) + strlen((char*)itr.getData()) + 2];
            sprintf(file_path, "%s/%s", directories[i]->path, (char*)itr.getData());
            input_file = fopen(file_path, "r");
            if (input_file == NULL)
            {
                continue;
            }
            while( (line_buf = fgetline(input_file)) != NULL)
            {
                buf_copy = new char[strlen(line_buf) + 1];
                temp = new char[strlen(line_buf) + 3];
                sprintf(temp, "~ %s", line_buf);            // just a "hack" so that the parsing function
                                                            // can get all the line tokens using strtok.
                strcpy(buf_copy, line_buf);
                
                strtok(temp, " ");       

                // Parse the line
                if (insertCitizenRecordParse(citizen_id, citizen_name, country_name, age, virus_name, vaccinated, date, NULL))
                // If parsing was successful, try to insert the Record.
                {
                    insertVaccinationRecord(citizen_id, citizen_name, country_name, age, virus_name, vaccinated, date,
                                            countries, viruses, citizens, bloom_size, NULL);
                }
                delete[] citizen_name;
                delete[] country_name;
                delete[] virus_name;
                delete[] temp;
                delete[] buf_copy;
                free(line_buf);
            }
            delete[] file_path;
            fclose(input_file);
            itr.forward();
        }        
    }
}

void scanNewFiles(DirectoryInfo **directories, unsigned short int num_dirs,
                  HashTable *citizens, LinkedList *countries, LinkedList *viruses,
                  unsigned long bloom_size)
{
    int citizen_id, age;
    char *citizen_name, *country_name, *virus_name;
    bool vaccinated;
    Date date;
    FILE *input_file;
    char *line_buf, *buf_copy, *temp;
    
    for (unsigned short i = 0; i < num_dirs; i++)
    {
        LinkedList::ListIterator *itr;
        if (directories[i]->contents->isEmpty())
        {
            directories[i]->updateContents();
            itr = new LinkedList::ListIterator(directories[i]->contents->listHead());
        }
        else
        {
            itr = new LinkedList::ListIterator(directories[i]->contents->listLast());
            directories[i]->updateContents();
            itr->forward();
        }

        while (!itr->isNull())
        {
            char *file_path = new char[strlen(directories[i]->path) + strlen((char*)itr->getData()) + 2];
            sprintf(file_path, "%s/%s", directories[i]->path, (char*)itr->getData());
            input_file = fopen(file_path, "r");
            if (input_file == NULL)
            {
                continue;
            }
            while( (line_buf = fgetline(input_file)) != NULL)
            {
                buf_copy = new char[strlen(line_buf) + 1];
                temp = new char[strlen(line_buf) + 3];
                sprintf(temp, "~ %s", line_buf);            // just a "hack" so that the parsing function
                                                            // can get all the line tokens using strtok.
                strcpy(buf_copy, line_buf);
                
                strtok(temp, " ");       

                // Parse the line
                if (insertCitizenRecordParse(citizen_id, citizen_name, country_name, age, virus_name, vaccinated, date, NULL))
                // If parsing was successful, try to insert the Record.
                {
                    insertVaccinationRecord(citizen_id, citizen_name, country_name, age, virus_name, vaccinated, date,
                                            countries, viruses, citizens, bloom_size, NULL);
                }
                delete[] citizen_name;
                delete[] country_name;
                delete[] virus_name;
                delete[] temp;
                delete[] buf_copy;
                free(line_buf);
            }
            delete[] file_path;
            fclose(input_file);
            itr->forward();
        }
        delete itr;        
    }
}

void serveRequest(int read_pipe_fd, int write_pipe_fd, char *buffer, unsigned int buffer_size,
                  HashTable *citizens, LinkedList *countries, LinkedList *viruses,
                  unsigned int &accepted_requests, unsigned int &rejected_requests)
{
    char msg_type;
    receiveRequestType(read_pipe_fd, msg_type, buffer, buffer_size);

    switch (msg_type)
    {
        case BLOOM_TRANSFER:
            /* code */
            break;
        case TRAVEL_REQUEST:

            break;
        case TRAVEL_STATS:
            break;    
        default:
            break;
    }
}

void sendBloomFilters(int write_pipe_fd, char *buffer, unsigned int buffer_size,
                      LinkedList *viruses)
{
    // Inform how many bloom filters will be sent
    sendInt(write_pipe_fd, viruses->getNumElements(), buffer, buffer_size);    

    for (LinkedList::ListIterator itr = viruses->listHead(); !itr.isNull(); itr.forward())
    {
        VirusRecords *virus = static_cast<VirusRecords*>(itr.getData());
        // Send the name of the virus that the bloom filter refers to
        sendString(write_pipe_fd, virus->virus_name, buffer, buffer_size);
        // Send the bloom filter
        sendBloomFilter(write_pipe_fd, virus->filter, buffer, buffer_size);
    }
}

int main(int argc, char const *argv[])
{
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigint_handler);
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

    scanAllFiles(directories, num_dirs, citizens, countries, viruses, bloom_size);

    //sendBloomFilters(write_pipe_fd, buffer, buffer_size, viruses);

    unsigned int accepted_requests = 0, rejected_requests = 0;
    //sigset_t set;
    //sigemptyset(&set);
    //sigfillset(&set);
    int sig;

    while ( !terminate )
    {
        if (dir_update_notifications == 0 && fifo_pipe_queue_messages == 0)
        {
            pause();
            //sigwait(&set, &sig);
        }
        if (dir_update_notifications > 0)
        {
            scanNewFiles(directories, num_dirs, citizens, countries, viruses, bloom_size);
            printf("updated\n");
            dir_update_notifications--;
        }
        if (fifo_pipe_queue_messages > 0)
        {
            serveRequest(read_pipe_fd, write_pipe_fd, buffer, buffer_size, citizens, countries, viruses,
                         accepted_requests, rejected_requests);
            fifo_pipe_queue_messages--;
        }
    }
    releaseResources(buffer, directories, num_dirs, citizens, countries, viruses, read_pipe_fd, write_pipe_fd);
    printf("cleaned\n");
    char logfile_name [20];
    sprintf(logfile_name, "log_file.%d", getpid());
    FILE *logfile;

    if ((logfile = fopen(logfile_name, "w")) == NULL)
    {
        fprintf(stderr, "Failed to create log file: %s\n", logfile_name);
    }

    // write in log file
    fclose(logfile);
    return 0;
}
