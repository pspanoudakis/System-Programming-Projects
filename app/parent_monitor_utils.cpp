/**
 * File: parent_monitor_utils.cpp
 * Implementations of Classes & Routines used by the Parent Monitor.
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string>
#include <sstream>

#include "../include/linked_list.hpp"
#include "../include/bloom_filter.hpp"
#include "../include/rb_tree.hpp"
#include "../include/utils.hpp"
#include "parent_monitor_utils.hpp"
#include "app_utils.hpp"
#include "../include/messaging.hpp"

MonitorInfo::MonitorInfo(): io_fd(-1), socket_fd(-1), process_id(-1), ftok_arg(-1),
subdirs(new LinkedList(delete_object_array<char>)) { }

MonitorInfo::~MonitorInfo()
{
    delete subdirs;
}

bool MonitorInfo::createSocket(uint16_t &port)
{
    socklen_t len;
    struct sockaddr_in servaddr, cli;
    
    if( (this->io_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Failed to create child socket");
        return false;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = 0;

    if (bind(this->io_fd, (sockaddr*)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("Failed to bind child socket");
        return false;
    }
    if (listen(this->io_fd, 1) == -1)
    {
        perror("Failed to listen child socket");
        return false;
    }
    //len = sizeof(servaddr);
    getsockname(this->io_fd, (sockaddr*)&servaddr, &len);
    port = ntohs(servaddr.sin_port);
    return true;
}

bool MonitorInfo::establishConnection()
{
    struct sockaddr_in cli;
    socklen_t len;
    if ( (this->io_fd = accept(this->socket_fd, (sockaddr*)&cli, &len)) == -1)
    {
        perror("Failed to accept child connection");
        return false;
    }
    return true;
}

void MonitorInfo::terminateConnection()
{
    close(socket_fd);
}

VirusRequests::VirusRequests(const char *name):
virus_name(copyString(name)), requests_tree(new RedBlackTree(compareTravelRequests)) { }

VirusRequests::~VirusRequests()
{
    delete[] virus_name;
    // The Tree destructor does not delete node data when deleting the nodes,
    // but the clear method does.
    requests_tree->clear(delete_object<TravelRequest>);
    delete requests_tree;
}

CountryMonitor::CountryMonitor(const char *name, MonitorInfo *monitor_info):
country_name(copyString(name)), monitor(monitor_info), virus_requests(new LinkedList(delete_object<VirusRequests>)) { }

CountryMonitor::~CountryMonitor()
{
    delete virus_requests;
    delete[] country_name;
}

TravelRequest::TravelRequest(Date &request_date, bool is_accepted):
date(request_date), accepted(is_accepted) { }

TravelRequest::~TravelRequest() { }

VirusFilter::VirusFilter(const char *name, unsigned long size): virus_name(copyString(name)), filter(new BloomFilter(size)) { }

VirusFilter::~VirusFilter()
{
    delete[] virus_name;
    delete filter;
}

/**
 * @brief Assigns the Country Directories to the Monitors (Round-Robin style).
 * 
 * @param path The path of the Directory that contains the Country Directories.
 * @param countries An array of pointers to CountryMonitors. When this routine returns,
 * it will contain 1 CountryMonitor for each country that had a directory.
 * @param monitors An array of pointers to MonitorInfos. When this routine returns, it
 * will contain 1 MonitorInfo for each Monitor to be created.
 */
bool assignMonitorDirectories(char *path, CountryMonitor **&countries, MonitorInfo **&monitors,
                              unsigned int num_monitors, struct dirent **&directories, unsigned int &num_dirs)
{
    unsigned int i;
    directories = NULL;
    // Scan the given directory path
    num_dirs = scandir(path, &directories, NULL, alphasort);
    if (num_dirs == -1)
    {
        fprintf(stderr, "Failed to scan directory: %s\n", path);
        delete[] path;
        return false;
    }

    // Create the Countries array
    countries = new CountryMonitor*[num_dirs - 2];
    for (i = 0; i < num_dirs - 2; i++)
    {
        countries[i] = NULL;
    }
    // Create the Monitors array. If a Monitor is not created,
    // the corresponding element will be NULL.
    monitors = new MonitorInfo*[num_monitors];
    for (i = 0; i < num_monitors; i++)
    {
        monitors[i] = NULL;
    }

    i = 0;  // This will be used as monitor counter

    // Skip directories[0] and directories[1] ("." and ".." respectively)
    for (unsigned int j = 2; j < num_dirs; j++)
    {
        if (monitors[i] == NULL)
        // This monitor has not been created yet
        {
            monitors[i] = new MonitorInfo();
        }
        // Create the full path of the current Country Directory
        std::string directory_path(path);
        directory_path.append("/");
        directory_path.append(directories[j]->d_name);
        // Story the Directory path in the MonitorInfo
        monitors[i]->subdirs->append(copyString(directory_path.c_str()));
        // Create the CountryMonitor for this Country
        countries[j - 2] = new CountryMonitor(directories[j]->d_name, monitors[i]);
        // Go to the next monitor in Round-Robin style
        i++;
        i = i % num_monitors;
    }

    return true;
}

/**
 * Creates the child Monitor processes. Stores the number of created processes in active_monitors.
 */
void createMonitors(MonitorInfo **monitors, unsigned int num_monitors, unsigned int &active_monitors)
{
    int pid;
    unsigned int i;
    // Iterate over the Monitor array and create a child for each element. When a NULL slot is found,
    // it means no more Monitors have been created, so stop creating more children.
    for(i = 0; (i < num_monitors && monitors[i] != NULL); i++)
    {
        pid = fork();
        switch (pid)
        {
            case -1:
                // fork failed
                perror("Failed to create child process\n");
                exit(EXIT_FAILURE);
            case 0:
                // Child
                //execl("./Monitor", "Monitor", monitors[i]->write_pipe_path, monitors[i]->read_pipe_path, NULL);
                execl("./monitor", "monitor", NULL);
                _Exit(EXIT_FAILURE);
            default:
                // Parent
                if (!monitors[i]->establishConnection())
                {
                    exit(EXIT_FAILURE);
                }
                monitors[i]->process_id = pid;
        }
    }
    // The Monitor array may have some empty slots, so store the number of the non-empty slots.
    active_monitors = i;
}

/**
 * Replaces a dead child Monitor with a new one.
 */
void restoreChild(MonitorInfo *monitor, char *buffer, unsigned int buffer_size, unsigned long int bloom_size,
                  LinkedList *viruses)
{
    uint16_t port;
    // Delete the Monitor pipes and create them again.
    monitor->terminateConnection();
    if ( !monitor->createSocket(port) )
    {
        exit(EXIT_FAILURE);
    }
    // Create a new child Monitor
    int new_pid = fork();
    switch (new_pid)
    {
        case -1:
            // fork failed
            perror("Failed to create child process\n");
            exit(EXIT_FAILURE);
        case 0:
            // child
            // execl("./Monitor", "Monitor", monitor->write_pipe_path, monitor->read_pipe_path, NULL);
            execl("./monitor", "monitor", NULL);
            _Exit(EXIT_FAILURE);
        default:
            // The Parent will send the required information to the new process
            monitor->process_id = new_pid;
            if ( !monitor->establishConnection() )
            {
                exit(EXIT_FAILURE);
            }
            // Send buffer size, bloom size and the Country directories assigned to this monitor
            //sendInt(fd, buffer_size, buffer, buffer_size);

            // Send int to be given to ftok
            sendInt(monitor->io_fd, monitor->ftok_arg, buffer, buffer_size);

            /*
            sendLongInt(fd, bloom_size, buffer, buffer_size);
            sendInt(fd, monitor->subdirs->getNumElements(), buffer, buffer_size);
            for (LinkedList::ListIterator itr = monitor->subdirs->listHead(); !itr.isNull(); itr.forward())
            {
                sendString(fd, static_cast<char*>(itr.getData()), buffer, buffer_size);
            }
            close(fd);
            */
            // Receive Bloom Filters
            unsigned int num_filters;
            receiveInt(monitor->io_fd, num_filters, buffer, buffer_size);
            for (unsigned int j = 0; j < num_filters; j++)
            {
                char *virus_name;
                receiveString(monitor->io_fd, virus_name, buffer, buffer_size);
                VirusFilter *virus = static_cast<VirusFilter*>(viruses->getElement(virus_name, compareNameVirusFilter));
                if (virus == NULL)
                {
                    viruses->append(new VirusFilter(virus_name, bloom_size));
                    virus = static_cast<VirusFilter*>(viruses->getLast());
                }
                free(virus_name);
                updateBloomFilter(monitor->io_fd, virus->filter, buffer, buffer_size);
            }
    }
}

/**
 * Restores any dead child Monitor processes.
 */
void checkAndRestoreChildren(MonitorInfo **monitors, unsigned int num_monitors, char *buffer, unsigned int buffer_size,
                             unsigned long int bloom_size, LinkedList *viruses, int &sigchld_counter)
{
    int wait_pid;
    for(unsigned int i = 0; i < num_monitors; i++)
    {
        // Check if the child is alive
        wait_pid = waitpid(monitors[i]->process_id, NULL, WNOHANG);
        if(wait_pid > 0)
        // It is not, so restore it.
        {
            restoreChild(monitors[i], buffer, buffer_size, bloom_size, viruses);
            sigchld_counter--;
        }
    }
}

/**
 * Sends required information to the child Monitors.
 */
void sendMonitorData(MonitorInfo **monitors, unsigned int num_monitors, char *buffer, unsigned int buffer_size,
                     unsigned long int bloom_size)
{
    int ftok_id = 1;
    // Iterate over the Monitors
    for(unsigned int i = 0; i < num_monitors; i++)
    {
        // Send buffer size
        sendInt(monitors[i]->io_fd, buffer_size, buffer, buffer_size);

        monitors[i]->ftok_arg = ftok_id;
        // Send int to be given to ftok
        sendInt(monitors[i]->io_fd, ftok_id, buffer, buffer_size);
        // ftok_id + 1 will also be used be the child process
        ftok_id += 2;

        // Send bloom filter size
        sendLongInt(monitors[i]->io_fd, bloom_size, buffer, buffer_size);
        // Send number of directories for this monitor
        sendInt(monitors[i]->io_fd, monitors[i]->subdirs->getNumElements(), buffer, buffer_size);
        // Send the directory paths
        for (LinkedList::ListIterator itr = monitors[i]->subdirs->listHead(); !itr.isNull(); itr.forward())
        {
            sendString(monitors[i]->io_fd, static_cast<char*>(itr.getData()), buffer, buffer_size);
        }
    }
}

/**
 * Receives the Bloom Filters of all child Monitors.
 * The filters are stored in the given LinkedList of VirusFilter objects.
 */
void receiveMonitorFilters(MonitorInfo **monitors, unsigned int num_monitors, LinkedList *viruses,
                           char *buffer, unsigned int buffer_size, unsigned long int bloom_size)
{
    // Setting up structs required by select()
    fd_set fdset;
    struct timeval timeout;
    timeout.tv_sec = 15;
    timeout.tv_usec = 0;
    FD_ZERO(&fdset);

    unsigned int done_monitors = 0;
    int max_fd = -1;
    // Open read pipe for each child Monitor
    for (unsigned int i = 0; i < num_monitors; i++)
    {
        // Add each FD to the set and find max FD to pass to select()
        FD_SET(monitors[i]->io_fd, &fdset);
        if (max_fd < monitors[i]->io_fd)
        {
            max_fd = monitors[i]->io_fd;
        }
    }

    // Keep looping until Bloom Filters have been received for all children
    while (done_monitors != num_monitors)
    {
        int ready_fds;
        // Wait until data has been sent to any pipe
        ready_fds = select(max_fd + 1, &fdset, NULL, NULL, &timeout);
        if (ready_fds == -1)
        {
            perror("Error/Timeout after waiting to receive Bloom Filters.\n");
            exit(EXIT_FAILURE);
        }
        if (ready_fds != 0)
        {
            // Iterate over all Monitors
            for (unsigned int i = 0; i < num_monitors; i++)
            {
                if (FD_ISSET(monitors[i]->io_fd, &fdset))
                // The read FD of this monitor is ready
                {
                    // Receive the Bloom Filters sent by this Monitor
                    unsigned int num_filters;
                    // Receive number of sent Bloom Filters
                    receiveInt(monitors[i]->io_fd, num_filters, buffer, buffer_size);
                    for (unsigned int j = 0; j < num_filters; j++)
                    {
                        char *virus_name;
                        // Receive the name of the virus related with this Bloom Filter
                        receiveString(monitors[i]->io_fd, virus_name, buffer, buffer_size);
                        // Try to find the VirusFilter for this virus
                        VirusFilter *virus = static_cast<VirusFilter*>(viruses->getElement(virus_name, compareNameVirusFilter));
                        if (virus == NULL)
                        // No VirusFilter found for this virus, so create one now.
                        {
                            viruses->append(new VirusFilter(virus_name, bloom_size));
                            virus = static_cast<VirusFilter*>(viruses->getLast());
                        }
                        free(virus_name);
                        // Receive the BloomFilter and update the stored one.
                        updateBloomFilter(monitors[i]->io_fd, virus->filter, buffer, buffer_size);
                    }
                    done_monitors++;
                }
            }
        }
        // Clear the set and add all FD's again
        FD_ZERO(&fdset);
        for (unsigned int i = 0; i < num_monitors; i++)
        {
            FD_SET(monitors[i]->io_fd, &fdset);
        }
    }
}

/**
 * Executes the /addVaccinationRecords command, for the specified Country.
 */
void addVaccinationRecords(const char *country_name, CountryMonitor **countries, unsigned int num_countries,
                           LinkedList *viruses, char *buffer, unsigned int buffer_size, unsigned long int bloom_size)
{
    // Iterate over the countries
    for (unsigned int i = 0; i < num_countries; i++)
    {
        if (strcmp(country_name, countries[i]->country_name) == 0)
        // Found the country with this name
        {
            // Send SIGUSR1 to child
            kill(countries[i]->monitor->process_id, SIGUSR1);
            unsigned int num_filters;
            // Receive the number of bloom filters that were sent
            receiveInt(countries[i]->monitor->io_fd, num_filters, buffer, buffer_size);
            // Receive the bloom filters
            for (unsigned int j = 0; j < num_filters; j++)
            {
                char *virus_name;
                // Get the name of the virus related to this bloom filter
                receiveString(countries[i]->monitor->io_fd, virus_name, buffer, buffer_size);
                VirusFilter *virus = static_cast<VirusFilter*>(viruses->getElement(virus_name, compareNameVirusFilter));
                if (virus == NULL)
                // If a new virus was detected, store a new Bloom Filter for it
                {
                    viruses->append(new VirusFilter(virus_name, bloom_size));
                    virus = static_cast<VirusFilter*>(viruses->getLast());
                }
                free(virus_name);
                // Receive the new Bloom Filter and update the present one
                updateBloomFilter(countries[i]->monitor->io_fd, virus->filter, buffer, buffer_size);
            }
            // Done
            printf("Records updated successfully.\n");
            return;
        }
    }
    printf("The specified country was not found.\n");
}

/**
 * Executes the /searchVaccinationStatus command, for the specified citizen ID.
 */
void searchVaccinationStatus(unsigned int citizen_id, MonitorInfo **monitors, unsigned int active_monitors,
                             char *buffer, unsigned int buffer_size)
{
    // Setting up structs required by select()
    fd_set fdset;
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    FD_ZERO(&fdset);

    unsigned int done_monitors = 0;
    int max_fd = -1;
    // Open read pipe for each child Monitor
    for (unsigned int i = 0; i < active_monitors; i++)
    {
        // Send the request to all Monitors and notify with SIGUSR2
        sendMessageType(monitors[i]->io_fd, SEARCH_STATUS, buffer, buffer_size);
        sendInt(monitors[i]->io_fd, citizen_id, buffer, buffer_size);
        kill(monitors[i]->process_id, SIGUSR2);
        // Add each FD to the set and find max FD to pass to select()
        FD_SET(monitors[i]->io_fd, &fdset);
        if (max_fd < monitors[i]->io_fd)
        {
            max_fd = monitors[i]->io_fd;
        }
    }

    // Keep looping until all children have answered
    while (done_monitors != active_monitors)
    {
        int ready_fds;
        // Wait until data has been sent to any pipe
        ready_fds = select(max_fd + 1, &fdset, NULL, NULL, &timeout);
        if (ready_fds == -1)
        {
            perror("Error/Timeout after waiting to receive travel request answers.\n");
            exit(EXIT_FAILURE);
        }
        if (ready_fds != 0)
        {
            // Iterate over all Monitors
            for (unsigned int i = 0; i < active_monitors; i++)
            {
                if (FD_ISSET(monitors[i]->io_fd, &fdset))
                // The read FD of this monitor is ready
                {
                    // Receive answer type
                    char msg_type;
                    receiveMessageType(monitors[i]->io_fd, msg_type, buffer, buffer_size);
                    if (msg_type == CITIZEN_FOUND)
                    // Citizen found, so receive the answer string
                    {
                        char *answer;
                        receiveString(monitors[i]->io_fd, answer, buffer, buffer_size);
                        printf("%s", answer);
                        free(answer);
                    }
                    done_monitors++;
                }
            }
        }
        // Clear the set and add all FD's again
        FD_ZERO(&fdset);
        for (unsigned int i = 0; i < active_monitors; i++)
        {
            FD_SET(monitors[i]->io_fd, &fdset);
        }
    }
}

/**
 * Executes the /travelRequest command with the specified arguments.
 */
void travelRequest(unsigned int citizen_id, Date &date, char *country_from, char *country_to, char *virus_name,
                   LinkedList *viruses, MonitorInfo **monitors, unsigned int active_monitors,
                   CountryMonitor **countries, unsigned int num_countries,
                   char *buffer, unsigned int buffer_size, unsigned int &accepted_requests, unsigned int &rejected_requests)
{
    // Get the Bloom Filter for the specified virus
    VirusFilter *target_virus = static_cast<VirusFilter*>(viruses->getElement(virus_name, compareNameVirusFilter));
    if (target_virus == NULL)
    // Not found
    {
        printf("ERROR: The specified virus was not found.\n");
    }
    else
    {
        // Get the CountryMonitor structures for the specified countries
        CountryMonitor *target_country_from = NULL;
        CountryMonitor *target_country_to = NULL;
        for (unsigned int i = 0; i < num_countries; i++)
        {
            if (strcmp(country_from, countries[i]->country_name) == 0)
            {
                target_country_from = countries[i];
                if (target_country_to != NULL)
                {
                    break;
                }
            }
            if (strcmp(country_to, countries[i]->country_name) == 0)
            {
                target_country_to = countries[i];
                if (target_country_from != NULL)
                {
                    break;
                }
            }
        }
        // Error if one of the countries was not found
        if (target_country_from == NULL)
        {
            printf("ERROR: The specified origin country was not found.\n");
        }
        else if (target_country_to == NULL)
        {
            printf("ERROR: The specified destination country was not found.\n");
        }
        else
        {
            char char_id[MAX_ID_DIGITS + 1];    // max digits + \0
            sprintf(char_id, "%d", citizen_id);

            // This indicates whether the request is rejected or accepted
            bool accepted = false;
            if (target_virus->filter->isPresent(char_id))
            // The citizen ID is marked as "maybe present" in the Bloom Filter
            {
                char ans_type;
                char *answer;
                // Send Request type and required information
                sendMessageType(target_country_from->monitor->io_fd, TRAVEL_REQUEST, buffer, buffer_size);
                sendInt(target_country_from->monitor->io_fd, citizen_id, buffer, buffer_size);
                sendDate(target_country_from->monitor->io_fd, date, buffer, buffer_size);
                sendString(target_country_from->monitor->io_fd, virus_name, buffer, buffer_size);
                // Notify the Monitor process
                kill(target_country_from->monitor->process_id, SIGUSR2);
                
                // Receive Monitor answer
                receiveMessageType(target_country_from->monitor->io_fd, ans_type, buffer, buffer_size);
                receiveString(target_country_from->monitor->io_fd, answer, buffer, buffer_size);
                printf("%s", answer);
                free(answer);
                accepted = (ans_type == TRAVEL_REQUEST_ACCEPTED);
            }
            else
            {
                printf("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");
            }
            // Store the TravelRequest
            // Find the requests tree for the given virus
            VirusRequests *requests = static_cast<VirusRequests*>(target_country_to->virus_requests->getElement(virus_name, compareNameVirusRequests));
            if (requests == NULL)
            // No requests tree exists for this virus, so create one now
            {
                target_country_to->virus_requests->append(new VirusRequests(virus_name));
                requests = static_cast<VirusRequests*>(target_country_to->virus_requests->getLast());
            }
            // Insert the request in the tree
            requests->requests_tree->insert(new TravelRequest(date, accepted));
            // Increment the proper counter
            accepted ? accepted_requests++ : rejected_requests++;
        }
    }
}

/**
 * While traversing the subtree starting with root recursively, 
 * counts the number of accepted and rejected requests between the two Dates
 * and updates the counters properly.
 */
void getTravelStatsRec(RBTreeNode *root, Date &start, Date &end, unsigned int &accepted, unsigned int &rejected)
{
    if (root == NULL) { return; }
    // Getting the root Record
    TravelRequest *root_data = static_cast<TravelRequest*>(root->data);

    if (compareDates(&root_data->date, &start) >= 0)
    // root Date is greater than start
    {
        // so count stats in Left subtree
        getTravelStatsRec(root->left, start, end, accepted, rejected);
        if (compareDates(&root_data->date, &end) <= 0)
        // end is greater than root Date
        {
            // root is in range, so update the correct counter
            root_data->accepted ? accepted++ : rejected++;
            // and count stats in the right subtree
            getTravelStatsRec(root->right, start, end, accepted, rejected);
            return;
        }
    }
    // root is smaller than start, so just search to the right
    if (compareDates(&root_data->date, &end) <= 0)
    {
        getTravelStatsRec(root->right, start, end, accepted, rejected);
    }
}

/**
 * Executes the /travelStats command, with the specified arguments (no country argument).
 */
void travelStats(char *virus_name, Date &start, Date &end, CountryMonitor **countries, unsigned int num_countries)
{
    unsigned int accepted_requests = 0;
    unsigned int rejected_requests = 0;
    // Iterate over the countries
    for (int i = 0; i < num_countries; i++)
    {
        // Get the requests tree for the target Virus related to each Country
        VirusRequests *virus_requests = static_cast<VirusRequests*>(countries[i]->virus_requests->getElement(virus_name, compareNameVirusRequests));
        if (virus_requests != NULL)
        // Tree found, so count stats
        {
            getTravelStatsRec(virus_requests->requests_tree->root, start, end, accepted_requests, rejected_requests);
        }
    }
    // Display results
    printf("TOTAL REQUESTS %d\n", accepted_requests + rejected_requests);
    printf("ACCEPTED %d\n", accepted_requests);
    printf("REJECTED %d\n", rejected_requests);
}

/**
 * Executes the /travelStats command, with the specified arguments.
 */
void travelStats(char *virus_name, Date &start, Date &end, const char *country_name,
                 CountryMonitor **countries, unsigned int num_countries)
{
    if (country_name == NULL)
    // No country argument given, so call the no-country travelStats version
    {
        travelStats(virus_name, start, end, countries, num_countries);
        return;
    }
    unsigned int accepted_requests = 0;
    unsigned int rejected_requests = 0;
    int i;
    // Iterate over the countries until the Country with the given name is found
    for (i = 0; i < num_countries; i++)
    {
        if (strcmp(country_name, countries[i]->country_name) == 0)
        // Country found
        {
            // Get the requests tree for the target Virus related to this Country
            VirusRequests *virus_requests = static_cast<VirusRequests*>(countries[i]->virus_requests->getElement(virus_name, compareNameVirusRequests));
            if (virus_requests != NULL)
            // Tree found, so count stats
            {
                getTravelStatsRec(virus_requests->requests_tree->root, start, end, accepted_requests, rejected_requests);
            }
            break;
        }
    }
    if (i == num_countries)
    // Country not found
    {
        printf("ERROR: The specified country was not found.\n");
    }
    else
    // Display results
    {
        printf("TOTAL REQUESTS %d\n", accepted_requests + rejected_requests);
        printf("ACCEPTED %d\n", accepted_requests);
        printf("REJECTED %d\n", rejected_requests);
    }
}

/**
 * Terminates all Monitor processes and deletes the created pipes.
 */
void terminateChildren(MonitorInfo **monitors, unsigned int num_monitors, char *buffer, unsigned int buffer_size)
{
    for(unsigned int i = 0; i < num_monitors; i++)
    {
        //kill(monitors[i]->process_id, SIGINT);
        sendMessageType(monitors[i]->io_fd, MONITOR_EXIT, buffer, buffer_size);
        waitpid(monitors[i]->process_id, NULL, 0);
        monitors[i]->terminateConnection();
    }
}

/**
 * Deletes the structures/ADT's used by the Parent Monitor.
 */
void releaseResources(CountryMonitor **countries, MonitorInfo **monitors, unsigned int num_monitors,
                      struct dirent **directories, unsigned int num_dirs, LinkedList *viruses)
{
    unsigned int i;
    // Delete CountryMonitors
    for (i = 0; i < num_dirs - 2; i++)
    {
        delete countries[i];
    }
    delete[] countries;

    // Delete MonitorInfos
    for (i = 0; i < num_monitors; i++)
    {
        delete monitors[i];
    }
    delete[] monitors;

    // Delete Directories array
    for (i = 0; i < num_dirs; i++)
    {
        free(directories[i]);
    }
    free(directories);
    
    delete viruses;
}

/* Comparison functions used by ADT's */

/**
 * Compares the given name to the virus name of the specified VirusFilter object.
 * Returns 0 if they are equal, non-zero otherwise.
 */
int compareNameVirusFilter(void *name, void *filter)
{
    return strcmp(static_cast<char*>(name), static_cast<VirusFilter*>(filter)->virus_name);
}

/**
 * Compares the 2 TravelRequest objects based on their dates first.
 * Since 2 requests must always by considered different, if the dates are equal,
 * either -1 or 1 will be returned randomly.
 */
int compareTravelRequests(void *r1, void *r2)
{
    TravelRequest *req1 = static_cast<TravelRequest*>(r1);
    TravelRequest *req2 = static_cast<TravelRequest*>(r2);

    int comp = compareDates(req1->date, req2->date);
    if (comp != 0) { return comp; }

    // return either -1 or 1
    return -1 + (rand() % 2) * 2;
}

/**
 * Compares the given name to the virus name of the specified VirusRequests object.
 * Returns 0 if they are equal, non-zero otherwise.
 */
int compareNameVirusRequests(void *name, void *req)
{
    return strcmp(static_cast<char*>(name), static_cast<VirusRequests*>(req)->virus_name);
}
