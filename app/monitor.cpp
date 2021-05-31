/**
 * File: monitor.cpp
 * Child Monitor main app.
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sem.h>
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
#include "sem_utils.hpp"

#define PERMS 0660

#define HASHTABLE_BUCKETS 10000         // Number of buckets for the Citizen Hash Table

int dir_update_notifications = 0;       // Incremented when the Parent has send a signal that indicates directory files update
int fifo_pipe_queue_messages = 0;       // Incremented when the Parent has send a signal that indicates pending information request
bool terminate = false;                 // Set to true when SIGINT/SIGQUIT received

int consume_sem_id;
int produce_sem_id;
int current_thread = 0;
int first_not_consumed;                 // The index of the first buffer element that has not been consumed
int cyclic_buffer_elements;             // total_elements - 1 is the index of the last element to be consumed

struct ThreadArgs {
    HashTable *citizens;
    LinkedList *countries;
    LinkedList *viruses;
    unsigned long bloom_size;
    char **cyclic_buffer;

    ThreadArgs(HashTable *cit, LinkedList *countr, LinkedList *v,
               unsigned long bloom_s, char **buffer)
    {
        this->citizens = cit;
        this->countries = countr;
        this->viruses = v;
        this->bloom_size = bloom_s;
        this->cyclic_buffer = buffer;
    }
};

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
        fprintf(stderr, "Invalid number of arguments given.\n");
        fprintf(stderr, "Usage: ./monitor <read pipe path> <write pipe path>\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * Opens the fifo pipes using the paths stored in argv[1] (read pipe) and argv[2] (write pipe)
 */
void openPipes(int &read_pipe_fd, int &write_pipe_fd, char const *argv[])
{
    if ( (read_pipe_fd = open(argv[1], O_RDONLY)) < 0)
    {
        perror("Failed to open read pipe.\n");
        exit(EXIT_FAILURE);
    }
    if ( (write_pipe_fd = open(argv[2], O_WRONLY)) < 0)
    {
        perror("Failed to open read pipe.\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * Release the Monitor resources.
 */
void releaseResources(char *buffer, DirectoryInfo **directories, unsigned short int num_dirs,
                      HashTable *citizens, LinkedList *countries, LinkedList *viruses,
                      int read_pipe_fd, int write_pipe_fd, pthread_t *threads, unsigned int num_threads)
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

    for (unsigned int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    delete[] threads;    
}

void* fileScanner(void *arguments)
{
    ThreadArgs *args = static_cast<ThreadArgs*>(arguments);
    int citizen_id, age;
    char *citizen_name, *country_name, *virus_name, *line_buf, *buf_copy, *temp;
    bool vaccinated;
    Date date;
    FILE *input_file;
    int current = current_thread++;
    while(true)
    {
        if (sem_down(consume_sem_id, 0) != -1)
        {
            if (first_not_consumed == cyclic_buffer_elements)
            {
                sem_up(produce_sem_id, 0);
            }
            else
            {
                sleep(1);
                printf("index: %d\n", first_not_consumed);
                printf("%d: %s\n", current, args->cyclic_buffer[first_not_consumed]);

                input_file = fopen(args->cyclic_buffer[first_not_consumed], "r");
                if (input_file != NULL) {
                    // Read records from the file
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
                                                    args->countries, args->viruses, args->citizens, args->bloom_size, NULL);
                        }
                        delete[] citizen_name;
                        delete[] country_name;
                        delete[] virus_name;
                        delete[] temp;
                        delete[] buf_copy;
                        free(line_buf);
                        fclose(input_file);
                    }
                }
                first_not_consumed++;
                sem_up(consume_sem_id, 0);
            }
        }
        else { break; }
    }
    delete args;
    pthread_exit(NULL);
}

/**
 * Scans all the files in the given directories and inserts any Vaccination Records found.
 */
void scanAllFiles(DirectoryInfo **directories, unsigned short int num_dirs,
                        HashTable *citizens, LinkedList *countries, LinkedList *viruses,
                        unsigned long bloom_size, char **cyclic_buffer, unsigned int cyclic_buffer_size,
                        unsigned int num_threads, pthread_t *threads)
{
    LinkedList files(delete_object_array<char>);
    
    // Iterate over the directories
    for (unsigned short i = 0; i < num_dirs; i++)
    {
        // Iterate over the contents of each directory
        LinkedList::ListIterator itr = directories[i]->contents->listHead();
        while (!itr.isNull())
        {
            // Create the full path of the file in order to open it
            char *file_path = new char[strlen(directories[i]->path) + strlen((char*)itr.getData()) + 2];
            sprintf(file_path, "%s/%s", directories[i]->path, (char*)itr.getData());
            files.append(file_path);
            itr.forward();
        }        
    }
    key_t consumer_key = ftok(".", 1);
    key_t producer_key = ftok(".", 2);
    if ( (consume_sem_id = semget(consumer_key, 1, IPC_CREAT|PERMS)) < 0)
    {
        perror("Failed to create consumer semaphore");
        exit(EXIT_FAILURE);
    }
    if ( (produce_sem_id = semget(producer_key, 1, IPC_CREAT|PERMS)) < 0)
    {
        sem_delete(consume_sem_id);
        perror("Failed to create producer semaphore");
        exit(EXIT_FAILURE);
    }
    sem_init(consume_sem_id, 0, 0);
    sem_init(produce_sem_id, 0, 1);
    for (unsigned int i = 0; i < num_threads; i++)
    {
        pthread_create(&threads[i], NULL, fileScanner, new ThreadArgs(citizens, countries, viruses, bloom_size, cyclic_buffer));
    }
    
    LinkedList::ListIterator itr = files.listHead();
    while(!itr.isNull())
    {
        sem_down(produce_sem_id, 0);
        printf("Now producing\n");
        int i;
        for (i = 0; (i < cyclic_buffer_size) && (!itr.isNull()); itr.forward(), i++)
        {
            cyclic_buffer[i] = static_cast<char*>(itr.getData());
        }
        cyclic_buffer_elements = i;
        first_not_consumed = 0;
        sem_up(consume_sem_id, 0);
    }
    sem_down(produce_sem_id, 0);
}

/**
 * Checks if any new files have been added in any country directory,
 * and scans them for new Vaccination records.
 */
void scanNewFiles(DirectoryInfo **directories, unsigned short int num_dirs,
                  HashTable *citizens, LinkedList *countries, LinkedList *viruses,
                  unsigned long bloom_size, char **cyclic_buffer, unsigned int cyclic_buffer_size)
{
    LinkedList files(delete_object_array<char>);
    
    // Iterate over the directories
    for (unsigned short i = 0; i < num_dirs; i++)
    {
        LinkedList::ListIterator *itr;
        if (directories[i]->contents->isEmpty())
        // The directory is empty
        {
            // So update and get an iterator to the beginning of the files list
            directories[i]->updateContents();
            itr = new LinkedList::ListIterator(directories[i]->contents->listHead());
        }
        else
        // Directory is not empty
        {
            // Get iterator to the last already stored file
            itr = new LinkedList::ListIterator(directories[i]->contents->listLast());
            // Update files list
            directories[i]->updateContents();
            // Make the iterator point to the next element
            // It will either be NULL (if no new files were added), or the first new file
            itr->forward();
        }

        while (!itr->isNull())
        {
            // Create the full path of the file in order to open it
            char *file_path = new char[strlen(directories[i]->path) + strlen((char*)itr->getData()) + 2];
            sprintf(file_path, "%s/%s", directories[i]->path, (char*)itr->getData());
            files.append(file_path);
            itr->forward();
        }
        delete itr;
    }
    LinkedList::ListIterator itr = files.listHead();
    while(!itr.isNull())
    {
        //sem_down(produce_sem_id, 0);
        printf("Now producing\n");
        int i;
        for (i = 0; (i < cyclic_buffer_size) && (!itr.isNull()); itr.forward(), i++)
        {
            cyclic_buffer[i] = static_cast<char*>(itr.getData());
        }
        cyclic_buffer_elements = i;
        first_not_consumed = 0;
        sem_up(consume_sem_id, 0);
        // -----------------------------
        sem_down(produce_sem_id, 0);
    }
    //sem_down(produce_sem_id, 0);
}

/**
 * Serves a /travelRequest command.
 */
void serveTravelRequest(int read_pipe_fd, int write_pipe_fd, char *buffer, unsigned int buffer_size,
                        HashTable *citizens, LinkedList *viruses, unsigned int &accepted_requests, unsigned int &rejected_requests)
{
    unsigned int citizen_id;
    // Receive citizen ID
    receiveInt(read_pipe_fd, citizen_id, buffer, buffer_size);
    Date date;
    // Receive date
    receiveDate(read_pipe_fd, date, buffer, buffer_size);
    Date date6monthsPrior;
    // Get the date that is 6 months prior
    date6monthsPrior.set6monthsPrior(date);
    char *virus_name;
    // Get virus name
    receiveString(read_pipe_fd, virus_name, buffer, buffer_size);
    std::string answer;
    char answer_type;

    // Get the VirusRecords structure for the specified virus
    VirusRecords *virus = (VirusRecords*)viruses->getElement(virus_name, compareNameVirusRecord);
    free(virus_name);
    if (virus == NULL)
    // Virus not found
    {
        answer.append("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");
        answer_type = TRAVEL_REQUEST_REJECTED;
        rejected_requests++;
    }
    else
    // Virus found
    {
        // Find record for this citizen
        VaccinationRecord *record = static_cast<VaccinationRecord*>(virus->getVaccinationRecord(citizen_id));
        if (record == NULL)
        {
            answer.append("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");
            answer_type = TRAVEL_REQUEST_REJECTED;
            rejected_requests++;
        }
        else
        // Record found
        {
            // Check if the Vaccination date is not more than 6 months older
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
    // Send the answer type and the answer string
    sendMessageType(write_pipe_fd, answer_type, buffer, buffer_size);
    sendString(write_pipe_fd, answer.c_str(), buffer, buffer_size);
}

/**
 * Serves a /searchVaccinationStatus request.
 */
void serveSearchStatusRequest(int read_pipe_fd, int write_pipe_fd, char *buffer, unsigned int buffer_size,
                              HashTable *citizens, LinkedList *viruses)
{
    // Receive the ID of the citizen
    unsigned int citizen_id;
    receiveInt(read_pipe_fd, citizen_id, buffer, buffer_size);
    // Try to find the citizen
    CitizenRecord *citizen = static_cast<CitizenRecord*>(citizens->getElement(&citizen_id, compareIdToCitizen));    
    if (citizen == NULL)
    // Citizen not found
    {
        sendMessageType(write_pipe_fd, CITIZEN_NOT_FOUND, buffer, buffer_size);
    }
    else
    // Citizen found
    {
        // The string with the Vaccination Status of the citizen for
        // all viruses will be stored here
        std::string answer;
        answer.append(citizen->toString());

        // Iterate over the viruses
        for (LinkedList::ListIterator itr = viruses->listHead(); !itr.isNull(); itr.forward())
        {
            VirusRecords *virus =  static_cast<VirusRecords*>(itr.getData());
            // Append the string with the Vaccination Status of the citizen for this virus
            virus->getVaccinationStatusString(citizen_id, answer);
        }
        // Send the answer to the Parent
        sendMessageType(write_pipe_fd, CITIZEN_FOUND, buffer, buffer_size);
        sendString(write_pipe_fd, answer.c_str(), buffer, buffer_size);
    }
}

/**
 * Sends the Monitor bloom filters to the Parent process.
 */
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

/**
 * To be called when the Monitor has been notified that the Parent Monitor has
 * requested information, in order to serve the request.
 */
void serveRequest(int read_pipe_fd, int write_pipe_fd, char *buffer, unsigned int buffer_size,
                  HashTable *citizens, LinkedList *countries, LinkedList *viruses,
                  unsigned int &accepted_requests, unsigned int &rejected_requests)
{
    char msg_type;
    // Receive the type of the request
    receiveMessageType(read_pipe_fd, msg_type, buffer, buffer_size);

    switch (msg_type)
    // Call the corresponding routine to serve it
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

/**
 * Creates the log file for this Monitor. Prints all the countries, as well as
 * statistics regarding the travel requests handled by the Monitor.
 */
void createLogFile(unsigned int &accepted_requests, unsigned int &rejected_requests, LinkedList *countries)
{
    // Create the logfile name string
    std::stringstream logfile_name_stream;
    logfile_name_stream << "log_files/log_file." << getpid();
    const char *logfile_name = copyString(logfile_name_stream.str().c_str());
    FILE *logfile;

    // Create the logfile
    if ((logfile = fopen(logfile_name, "w")) == NULL)
    {
        fprintf(stderr, "Failed to create log file: %s\n", logfile_name);
        delete [] logfile_name;
        return;
    }
    
    delete [] logfile_name;
    // Write all country names in the logfile
    for (LinkedList::ListIterator itr = countries->listHead(); !itr.isNull(); itr.forward())
    {
        CountryStatus *country =  static_cast<CountryStatus*>(itr.getData());
        fprintf(logfile, "%s\n", country->country_name);
    }
    // Print travel request counters
    fprintf(logfile, "TOTAL TRAVEL REQUESTS %d\n", accepted_requests + rejected_requests);
    fprintf(logfile, "ACCEPTED %d\n", accepted_requests);
    fprintf(logfile, "REJECTED %d\n", rejected_requests);
    
    fclose(logfile);
}

void parseArgs(int argc, char const *argv[], uint16_t &port, unsigned int &num_threads, unsigned int &buffer_size,
               unsigned int &cyclic_buffer_size, DirectoryInfo **&directories, unsigned long &bloom_size)
{
    if (argc < 11)
    {
        fprintf(stderr, "Invalid number of arguments given.\n");
        fprintf(stderr, "Usage: ./monitor <read pipe path> <write pipe path>\n");
        exit(EXIT_FAILURE);
    }
    int num_dirs = argc - 11;
    directories = new DirectoryInfo*[num_dirs];
    for (unsigned int i = 0; i < num_dirs; i++)
    {
        directories[i] = new DirectoryInfo(argv[11 + i]);
        directories[i]->addContents();
    }
}

int main(int argc, char const *argv[])
{
    // Register signal handlers
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigint_handler);

    // Check arguments and open the fifo pipes assigned to this Monitor
    checkArgc(argc);
    int read_pipe_fd, write_pipe_fd;
    openPipes(read_pipe_fd, write_pipe_fd, argv);

    unsigned int num_threads = 3;
    pthread_t *threads = new pthread_t[num_threads];

    // Getting buffer size & creating the buffer to use for pipe I/O
    unsigned int buffer_size;
    char temp_buffer[sizeof(unsigned int)];
    receiveInt(read_pipe_fd, buffer_size, temp_buffer, sizeof(unsigned int));
    char *buffer = new char[buffer_size];

    // Read Bloom Filter size
    unsigned long bloom_size;
    receiveLongInt(read_pipe_fd, bloom_size, buffer, buffer_size);
    // Read number of directories that will be assigned to this monitor
    unsigned int num_dirs;
    receiveInt(read_pipe_fd, num_dirs, buffer, buffer_size);
    DirectoryInfo **directories = new DirectoryInfo*[num_dirs];

    // Read num_dirs directory paths, store them and get their contents
    char *str;
    for (unsigned int i = 0; i < num_dirs; i++)
    {
        str = NULL;
        receiveString(read_pipe_fd, str, buffer, buffer_size);
        directories[i] = new DirectoryInfo(str);
        directories[i]->addContents();
        free(str);
    }

    // Create structures to be used for storing record-related information
    HashTable *citizens = new HashTable(HASHTABLE_BUCKETS, delete_object<CitizenRecord>, citizenHashObject);
    LinkedList *countries = new LinkedList(delete_object<CountryStatus>);
    LinkedList *viruses = new LinkedList(delete_object<VirusRecords>);

    // Cyclic Buffer
    int cyclic_buffer_size = 4;
    char **cyclic_buffer = new char*[cyclic_buffer_size];

    // Scan all the files and insert all records found
    scanAllFiles(directories, num_dirs, citizens, countries, viruses, bloom_size, cyclic_buffer, cyclic_buffer_size,
                 num_threads, threads);
    // Send all the bloom filters to the parent process
    sendBloomFilters(write_pipe_fd, buffer, buffer_size, viruses);

    // Travel request counters
    unsigned int accepted_requests = 0, rejected_requests = 0;

    // Loop until SIGINT/SIGQUIT received
    while ( !terminate )
    {
        // No non-served signals for now
        if (dir_update_notifications == 0 && fifo_pipe_queue_messages == 0)
        // Suspend the Monitor until any signal is received
        {
            pause();
        }
        if (dir_update_notifications > 0)
        // Received indication that new files have been added in the given directories
        {
            dir_update_notifications--;
            scanNewFiles(directories, num_dirs, citizens, countries, viruses, bloom_size, cyclic_buffer, cyclic_buffer_size);
            sendBloomFilters(write_pipe_fd, buffer, buffer_size, viruses);
        }
        if (fifo_pipe_queue_messages > 0)
        // Received indication that the Parent process has requested informations
        {
            fifo_pipe_queue_messages--;
            serveRequest(read_pipe_fd, write_pipe_fd, buffer, buffer_size, citizens, countries, viruses,
                         accepted_requests, rejected_requests);
        }
    }

    sem_delete(consume_sem_id);
    sem_delete(produce_sem_id);
    // Create log file and release resouces
    createLogFile(accepted_requests, rejected_requests, countries);
    releaseResources(buffer, directories, num_dirs, citizens, countries, viruses, read_pipe_fd, write_pipe_fd,
                     threads, num_threads);

    return 0;
}
