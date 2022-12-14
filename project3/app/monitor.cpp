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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <climits>

#include "../include/linked_list.hpp"
#include "../include/hash_table.hpp"
#include "../include/utils.hpp"
#include "../include/messaging.hpp"
#include "app_utils.hpp"
#include "parse_utils.hpp"
#include "sem_utils.hpp"

#define PERMS 0660

#define CONNECTION_TIMEOUT_SECS 5
#define HASHTABLE_BUCKETS 10000         // Number of buckets for the Citizen Hash Table

int dir_update_notifications = 0;       // Incremented when the Parent has send a signal that indicates directory files update
int pending_messages = 0;       // Incremented when the Parent has send a signal that indicates pending information request
bool terminate = false;                 // Set to true when SIGINT/SIGQUIT received

unsigned int ftok_id;
int sem_id;
int first_not_consumed;                 // The index of the first buffer element that has not been consumed
int cyclic_buffer_elements;             // total_elements - 1 is the index of the last element to be consumed

/**
 * The struct to be passed to the threads, with all the required structures
 * and information.
 */
struct ThreadArgs {
    HashTable *citizens;
    LinkedList *countries;
    LinkedList *viruses;
    unsigned long bloom_size;
    char **cyclic_buffer;
    int thread_num;

    ThreadArgs(HashTable *cit, LinkedList *countr, LinkedList *v,
               unsigned long bloom_s, char **buffer, int i) :
        citizens(cit),
        countries(countr),
        viruses(v),
        bloom_size(bloom_s),
        cyclic_buffer(buffer),
        thread_num(i)
        { }
};

void sigusr1_handler(int s)
{
    dir_update_notifications++;
    signal(SIGUSR1, sigusr1_handler);
}

void sigusr2_handler(int s)
{
    pending_messages++;
    signal(SIGUSR2, sigusr2_handler);
}

void sigint_handler(int s)
{
    terminate = true;
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigint_handler);
}

/**
 * Connects to the socket in the given port.
 * 
 * @return TRUE if the connection was successful, FALSE otherwise.
 */
bool socketConnect(int &socket_fd, uint16_t port)
{
    struct sockaddr_in servaddr;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        fprintf(stderr, "Failed to create socket\n");
        return false;
    }
    bzero(&servaddr, sizeof(servaddr));
    
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    printf("hostname: %s\n", hostname);
    struct hostent *host = gethostbyname(hostname);
    servaddr.sin_family = AF_INET;
    memcpy(&(servaddr.sin_addr), host->h_addr, host->h_length);
    servaddr.sin_port = htons(port);

    // The parent process socket already listens, so just try to connect once.
    if (connect(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        perror("Failed to connect to socket");
        return false;
    }    
    return true;
}

/**
 * Release the Monitor resources.
 */
void releaseResources(char *buffer, DirectoryInfo **directories, unsigned short int num_dirs,
                      HashTable *citizens, LinkedList *countries, LinkedList *viruses,
                      int socket_fd, pthread_t *threads, unsigned int num_threads)
{
    delete[] buffer;
    for (unsigned short i = 0; i < num_dirs; i++)
    {
        delete directories[i];
    }
    delete[] directories;

    close(socket_fd);
    delete countries;
    delete viruses;
    delete citizens;

    for (unsigned int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    delete[] threads;    
}

/**
 * @brief The cyclic buffer consuming threads routine.
 * 
 * Attempts to consume a file path from the given buffer.
 * If a path is successfully consumed, the file is opened and scanned for new Vaccination Records.
 * If the buffer is found as fully consumed, the parent thread will be notified
 * to place more file paths in it.
 * 
 * @param arguments A dynamically created ThreadArgs struct.
 */
void* fileScanner(void *arguments)
{
    ThreadArgs *args = static_cast<ThreadArgs*>(arguments);
    int citizen_id, age;
    char *citizen_name, *country_name, *virus_name, *line_buf, *buf_copy, *temp;
    bool vaccinated;
    Date date;
    FILE *input_file;
    // Attempt to consume a buffer element
    while(sem_down(sem_id, 0) != -1)
    {
        if (first_not_consumed == cyclic_buffer_elements)
        // No available elements
        {
            // So notify the parent thread that the buffer is empty
            sem_up(sem_id, 1);
            // And continue with the loop
        }
        else
        // There is an available element
        {
            #ifdef SHOW_CYCLIC_BUFFER_MSG
            printf("index: %d\n", first_not_consumed);
            printf("Thread %d: %s\n", args->thread_num, args->cyclic_buffer[first_not_consumed]);
            #endif

            // Open the file in the buffer element path
            input_file = fopen(args->cyclic_buffer[first_not_consumed], "r");
            if (input_file != NULL) {
                // Read records from the file
                while( (line_buf = fgetline(input_file)) != NULL)
                // Read the file line by line
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
                }
                fclose(input_file);
            }
            // Mark the next element as the first not consumed element
            first_not_consumed++;
            // End of critical section
            sem_up(sem_id, 0);
        }
    }
    // At this point we expect that the semaphore set has been deleted by
    // the parent thread.
    if (errno != EIDRM)
    // If this is not the case, indicate it.
    {
        perror("semdown");
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
    // All files will be stored here
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
    // Create semaphore set
    key_t sem_key = ftok(".", ftok_id);
    // Get 2 semapthores
    if ( (sem_id = semget(sem_key, 2, IPC_CREAT|PERMS)) < 0)
    {
        perror("Failed to create semaphore set");
        exit(EXIT_FAILURE);
    }
    // Initialize semaphore 0 to 0, and semaphore 1 to 1.
    sem_init(sem_id, 0, 0);     // when this is up, it means someone can safely read (or attempt to read)
    sem_init(sem_id, 1, 1);     // when this is up, it means the parent thread can fill up the buffer with new files
    for (unsigned int i = 0; i < num_threads; i++)
    {
        // Each thread handles its own argument struct, and deletes it before terminating.
        pthread_create(&threads[i], NULL, fileScanner, new ThreadArgs(citizens, countries, viruses, bloom_size, cyclic_buffer, i));
    }
    // Start iterating over the files
    LinkedList::ListIterator itr = files.listHead();
    while(!itr.isNull())
    {
        // Attempt to fill up buffer with new files
        sem_down(sem_id, 1);

        #ifdef SHOW_CYCLIC_BUFFER_MSG
        printf("Now producing\n");
        #endif

        int i;
        // Fill up the buffer with new files (or place them all if the buffer has enough space)
        for (i = 0; (i < cyclic_buffer_size) && (!itr.isNull()); itr.forward(), i++)
        {
            cyclic_buffer[i] = static_cast<char*>(itr.getData());
        }
        // Make sure no one reads more than i elements
        cyclic_buffer_elements = i;
        first_not_consumed = 0;

        // Notify the threads to consume the buffer elememts
        sem_up(sem_id, 0);
    }
    // Wait for all elements to be consumed
    sem_down(sem_id, 1);
}

/**
 * Checks if any new files have been added in any country directory,
 * and scans them for new Vaccination records.
 */
void scanNewFiles(DirectoryInfo **directories, unsigned short int num_dirs,
                  HashTable *citizens, LinkedList *countries, LinkedList *viruses,
                  unsigned long bloom_size, char **cyclic_buffer, unsigned int cyclic_buffer_size)
{
    // All new files will be stored here
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
    // Start iterating over the new files
    LinkedList::ListIterator itr = files.listHead();
    while(!itr.isNull())
    {
        // Semaphore 1 is already down, so new files can be added to the buffer
        #ifdef SHOW_CYCLIC_BUFFER_MSG
        printf("Now producing\n");
        #endif

        int i;
        // Fill up the buffer with new files (or place them all if the buffer has enough space)
        for (i = 0; (i < cyclic_buffer_size) && (!itr.isNull()); itr.forward(), i++)
        {
            cyclic_buffer[i] = static_cast<char*>(itr.getData());
        }
        // Make sure no one reads more than i elements
        cyclic_buffer_elements = i;
        first_not_consumed = 0;

        // Notify the threads to consume the buffer elememts
        sem_up(sem_id, 0);
        // Wait till all elements have been consumed
        sem_down(sem_id, 1);
    }
}

/**
 * Serves a /travelRequest command.
 */
void serveTravelRequest(int socket_fd, char *buffer, unsigned int buffer_size,
                        HashTable *citizens, LinkedList *viruses, unsigned int &accepted_requests, unsigned int &rejected_requests)
{
    unsigned int citizen_id;
    // Receive citizen ID
    receiveInt(socket_fd, citizen_id, buffer, buffer_size);
    Date date;
    // Receive date
    receiveDate(socket_fd, date, buffer, buffer_size);
    Date date6monthsPrior;
    // Get the date that is 6 months prior
    date6monthsPrior.set6monthsPrior(date);
    char *virus_name;
    // Get virus name
    receiveString(socket_fd, virus_name, buffer, buffer_size);
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
    sendMessageType(socket_fd, answer_type, buffer, buffer_size);
    sendString(socket_fd, answer.c_str(), buffer, buffer_size);
}

/**
 * Serves a /searchVaccinationStatus request.
 */
void serveSearchStatusRequest(int socket_fd, char *buffer, unsigned int buffer_size,
                              HashTable *citizens, LinkedList *viruses)
{
    // Receive the ID of the citizen
    unsigned int citizen_id;
    receiveInt(socket_fd, citizen_id, buffer, buffer_size);
    // Try to find the citizen
    CitizenRecord *citizen = static_cast<CitizenRecord*>(citizens->getElement(&citizen_id, compareIdToCitizen));    
    if (citizen == NULL)
    // Citizen not found
    {
        sendMessageType(socket_fd, CITIZEN_NOT_FOUND, buffer, buffer_size);
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
        sendMessageType(socket_fd, CITIZEN_FOUND, buffer, buffer_size);
        sendString(socket_fd, answer.c_str(), buffer, buffer_size);
    }
}

/**
 * Sends the Monitor bloom filters to the Parent process.
 */
void sendBloomFilters(int socket_fd, char *buffer, unsigned int buffer_size,
                      LinkedList *viruses)
{
    // Inform how many bloom filters will be sent
    sendInt(socket_fd, viruses->getNumElements(), buffer, buffer_size);    

    for (LinkedList::ListIterator itr = viruses->listHead(); !itr.isNull(); itr.forward())
    {
        VirusRecords *virus = static_cast<VirusRecords*>(itr.getData());
        // Send the name of the virus that the bloom filter refers to
        sendString(socket_fd, virus->virus_name, buffer, buffer_size);
        // Send the bloom filter
        sendBloomFilter(socket_fd, virus->filter, buffer, buffer_size);
    }
}

/**
 * To be called when the Monitor has been notified that the Parent Monitor has
 * requested information, in order to serve the request.
 */
void serveRequest(int socket_fd, char *buffer, unsigned int buffer_size,
                  HashTable *citizens, LinkedList *countries, LinkedList *viruses,
                  unsigned int &accepted_requests, unsigned int &rejected_requests)
{
    char msg_type;
    // Receive the type of the request
    receiveMessageType(socket_fd, msg_type, buffer, buffer_size);

    switch (msg_type)
    // Call the corresponding routine to serve it
    {
        case TRAVEL_REQUEST:
            serveTravelRequest(socket_fd, buffer, buffer_size,
                               citizens, viruses, accepted_requests, rejected_requests);
            break;  
        case SEARCH_STATUS:
            serveSearchStatusRequest(socket_fd, buffer, buffer_size, citizens, viruses);
            break;
        case MONITOR_EXIT:
            terminate = true;
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
    logfile_name_stream << "./log_files/log_file." << getpid();
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

int main(int argc, char const *argv[])
{
    // Register signal handlers
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigint_handler);

    int socket_fd;
    unsigned int num_dirs, buffer_size, cyclic_buffer_size, num_threads;
    unsigned long bloom_size;
    uint16_t port;
    DirectoryInfo **directories;

    // Check, parse and store the arguments
    if (!childCheckparseArgs(argc, argv, port, num_threads, buffer_size,
                   cyclic_buffer_size, directories, bloom_size, num_dirs))
    {
        exit(EXIT_FAILURE);
    }
    // Connect to socket at the given port
    if (!socketConnect(socket_fd, port))
    {
        exit(EXIT_FAILURE);
    }

    pthread_t *threads = new pthread_t[num_threads];
    char *buffer = new char[buffer_size];

    // Create structures to be used for storing record-related information
    HashTable *citizens = new HashTable(HASHTABLE_BUCKETS, delete_object<CitizenRecord>, citizenHashObject);
    LinkedList *countries = new LinkedList(delete_object<CountryStatus>);
    LinkedList *viruses = new LinkedList(delete_object<VirusRecords>);

    char **cyclic_buffer = new char*[cyclic_buffer_size];

    receiveInt(socket_fd, ftok_id, buffer, buffer_size);
    // Scan all the files and insert all records found
    scanAllFiles(directories, num_dirs, citizens, countries, viruses, bloom_size, cyclic_buffer, cyclic_buffer_size,
                 num_threads, threads);
    // Send all the bloom filters to the parent process
    sendBloomFilters(socket_fd, buffer, buffer_size, viruses);

    // Travel request counters
    unsigned int accepted_requests = 0, rejected_requests = 0;

    // Loop until SIGINT/SIGQUIT received
    while ( !terminate )
    {
        // No non-served signals for now
        if (dir_update_notifications == 0 && pending_messages == 0)
        // Suspend the Monitor until any signal is received
        {
            pause();
        }
        if (dir_update_notifications > 0)
        // Received indication that new files have been added in the given directories
        {
            dir_update_notifications--;
            scanNewFiles(directories, num_dirs, citizens, countries, viruses, bloom_size, cyclic_buffer, cyclic_buffer_size);
            sendBloomFilters(socket_fd, buffer, buffer_size, viruses);
        }
        if (pending_messages > 0)
        // Received indication that the Parent process has requested informations
        {
            pending_messages--;
            serveRequest(socket_fd, buffer, buffer_size, citizens, countries, viruses,
                         accepted_requests, rejected_requests);
        }
    }

    // By deleting the semaphore set, the threads will terminate (see fileScanner routine)
    sem_delete(sem_id);
    // Create log file and release resouces
    createLogFile(accepted_requests, rejected_requests, countries);
    delete[] cyclic_buffer;
    releaseResources(buffer, directories, num_dirs, citizens, countries, viruses, socket_fd,
                     threads, num_threads);

    return 0;
}
