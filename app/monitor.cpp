/**
 * File: parent_monitor.cpp
 * Child Monitor main app.
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <sstream>

#include "../include/linked_list.hpp"
#include "../include/hash_table.hpp"
#include "../include/utils.hpp"
#include "pipe_msg.hpp"
#include "app_utils.hpp"
#include "parse_utils.hpp"

#define HASHTABLE_BUCKETS 10000         // Number of buckets for the Citizen Hash Table
#define MAX_BLOOM_SIZE 1000000          // Maximum Bloom Filter size allowed

int dir_update_notifications = 0;
int fifo_pipe_queue_messages = 0;
bool terminate = false;

void sigusr1_handler(int s)
{
    dir_update_notifications++;
    //printf("Got sigusr1\n");
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

void serveTravelRequest(int read_pipe_fd, int write_pipe_fd, char *buffer, unsigned int buffer_size,
                        HashTable *citizens, LinkedList *viruses, unsigned int &accepted_requests, unsigned int &rejected_requests)
{
    unsigned int citizen_id;
    receiveInt(read_pipe_fd, citizen_id, buffer, buffer_size);
    Date date;
    receiveDate(read_pipe_fd, date, buffer, buffer_size);
    Date date6monthsPrior;
    date6monthsPrior.set6monthsPrior(date);
    char *virus_name;
    receiveString(read_pipe_fd, virus_name, buffer, buffer_size);
    std::string answer;
    char answer_type;

    VirusRecords *virus = (VirusRecords*)viruses->getElement(virus_name, compareNameVirusRecord);
    free(virus_name);
    if (virus == NULL)
    {
        //answer.append("ERROR: The specified virus has no records in the given country")
        answer.append("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");
        answer_type = TRAVEL_REQUEST_REJECTED;
        rejected_requests++;
    }
    else
    {
        VaccinationRecord *record = static_cast<VaccinationRecord*>(virus->getVaccinationRecord(citizen_id));
        if (record == NULL)
        {
            answer.append("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");
            answer_type = TRAVEL_REQUEST_REJECTED;
            rejected_requests++;
        }
        else
        {
            if (record->date.isBetween(date6monthsPrior, date))
            {
                answer.append("REQUEST ACCEPTED - HAPPY TRAVELS\n");
                answer_type = TRAVEL_REQUEST_ACCEPTED;
                accepted_requests++;
            }
            else
            {
                answer.append("REQUEST REJECTED - YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE\n");
                answer_type = TRAVEL_REQUEST_REJECTED;
                rejected_requests++;
            }            
        }
    }
    
    // Only the message type could be sent though...
    sendMessageType(write_pipe_fd, answer_type, buffer, buffer_size);
    sendString(write_pipe_fd, answer.c_str(), buffer, buffer_size);
}

void serveSearchStatusRequest(int read_pipe_fd, int write_pipe_fd, char *buffer, unsigned int buffer_size,
                              HashTable *citizens, LinkedList *viruses)
{
    unsigned int citizen_id;
    receiveInt(read_pipe_fd, citizen_id, buffer, buffer_size);
    CitizenRecord *citizen = static_cast<CitizenRecord*>(citizens->getElement(&citizen_id, compareIdToCitizen));
    
    if (citizen == NULL)
    {
        //answer.append("A citizen with the specified ID was not found.\n");
        sendMessageType(write_pipe_fd, CITIZEN_NOT_FOUND, buffer, buffer_size);
    }
    else
    {
        std::string answer;
        answer.append(citizen->toString());

        for (LinkedList::ListIterator itr = viruses->listHead(); !itr.isNull(); itr.forward())
        {
            VirusRecords *virus =  static_cast<VirusRecords*>(itr.getData());
            virus->getVaccinationStatusString(citizen_id, answer);
        }
        sendMessageType(write_pipe_fd, CITIZEN_FOUND, buffer, buffer_size);
        sendString(write_pipe_fd, answer.c_str(), buffer, buffer_size);
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

void serveRequest(int read_pipe_fd, int write_pipe_fd, char *buffer, unsigned int buffer_size,
                  HashTable *citizens, LinkedList *countries, LinkedList *viruses,
                  unsigned int &accepted_requests, unsigned int &rejected_requests)
{
    char msg_type;
    receiveMessageType(read_pipe_fd, msg_type, buffer, buffer_size);

    switch (msg_type)
    {
        case TRAVEL_REQUEST:
            serveTravelRequest(read_pipe_fd, write_pipe_fd, buffer, buffer_size,
                               citizens, viruses, accepted_requests, rejected_requests);
            break;  
        case SEARCH_STATUS:
            serveSearchStatusRequest(read_pipe_fd, write_pipe_fd, buffer, buffer_size, citizens, viruses);
            break;
        default:
            // Should never be reached, or something is wrong
            break;
    }
}

void createLogFile(unsigned int &accepted_requests, unsigned int &rejected_requests, LinkedList *countries)
{
    std::stringstream logfile_name_stream;
    logfile_name_stream << "log_files/log_file." << getpid();
    const char *logfile_name = copyString(logfile_name_stream.str().c_str());
    FILE *logfile;

    if ((logfile = fopen(logfile_name, "w")) == NULL)
    {
        fprintf(stderr, "Failed to create log file: %s\n", logfile_name);
        delete [] logfile_name;
        return;
    }
    
    delete [] logfile_name;
    for (LinkedList::ListIterator itr = countries->listHead(); !itr.isNull(); itr.forward())
    {
        CountryStatus *country =  static_cast<CountryStatus*>(itr.getData());
        fprintf(logfile, "%s\n", country->country_name);
    }
    fprintf(logfile, "TOTAL TRAVEL REQUESTS %d\n", accepted_requests + rejected_requests);
    fprintf(logfile, "ACCEPTED %d\n", accepted_requests);
    fprintf(logfile, "REJECTED %d\n", rejected_requests);
    
    fclose(logfile);
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
    unsigned int num_dirs;
    receiveInt(read_pipe_fd, num_dirs, buffer, buffer_size);

    DirectoryInfo **directories = new DirectoryInfo*[num_dirs];
    char *str;
    for (unsigned int i = 0; i < num_dirs; i++)
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
    sendBloomFilters(write_pipe_fd, buffer, buffer_size, viruses);

    unsigned int accepted_requests = 0, rejected_requests = 0;
    //sigset_t set;
    //sigemptyset(&set);
    //sigfillset(&set);
    //int sig;

    while ( !terminate )
    {
        if (dir_update_notifications == 0 && fifo_pipe_queue_messages == 0)
        {
            pause();
            //sigwait(&set, &sig);
            //sigsuspend(&set);
        }
        if (dir_update_notifications > 0)
        {
            dir_update_notifications--;
            scanNewFiles(directories, num_dirs, citizens, countries, viruses, bloom_size);
            sendBloomFilters(write_pipe_fd, buffer, buffer_size, viruses);
        }
        if (fifo_pipe_queue_messages > 0)
        {
            fifo_pipe_queue_messages--;
            serveRequest(read_pipe_fd, write_pipe_fd, buffer, buffer_size, citizens, countries, viruses,
                         accepted_requests, rejected_requests);
        }
    }
    createLogFile(accepted_requests, rejected_requests, countries);
    releaseResources(buffer, directories, num_dirs, citizens, countries, viruses, read_pipe_fd, write_pipe_fd);

    return 0;
}
