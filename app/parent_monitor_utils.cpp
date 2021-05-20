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

#include <string>
#include <sstream>

#include "../include/linked_list.hpp"
#include "../include/bloom_filter.hpp"
#include "../include/rb_tree.hpp"
#include "../include/utils.hpp"
#include "parent_monitor_utils.hpp"
#include "app_utils.hpp"
#include "pipe_msg.hpp"

MonitorInfo::MonitorInfo(): process_id(-1), read_fd(-1), write_pipe_path(NULL), read_pipe_path(NULL),
subdirs(new LinkedList(delete_object_array<char>)) { }

MonitorInfo::~MonitorInfo()
{
    delete[] read_pipe_path;
    delete[] write_pipe_path;
    delete subdirs;
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
bool assignMonitorDirectories(const char *path, CountryMonitor **&countries, MonitorInfo **&monitors,
                              unsigned int num_monitors, struct dirent **&directories, unsigned int &num_dirs)
{
    unsigned int i;
    // Scan the given directory path
    num_dirs = scandir(path, &directories, NULL, alphasort);
    if (num_dirs == -1)
    {
        fprintf(stderr, "Failed to scan directory: %s\n", path);
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
            // Create and store pipe paths
            std::stringstream read_name_stream;
            std::stringstream write_name_stream;
            monitors[i] = new MonitorInfo();
            read_name_stream << "./fifo_pipes/read" << i;
            write_name_stream << "./fifo_pipes/write" << i; 
            monitors[i]->read_pipe_path = copyString(read_name_stream.str().c_str());
            monitors[i]->write_pipe_path = copyString(write_name_stream.str().c_str());
            // Make the pipes
            if (mkfifo(monitors[i]->write_pipe_path, 0666) < 0 )
            {
                fprintf(stderr, "Error creating fifo: %s\n", monitors[i]->write_pipe_path);
                return false;
            }
            if (mkfifo(monitors[i]->read_pipe_path, 0666) < 0 )
            {
                fprintf(stderr, "Error creating fifo: %s\n", monitors[i]->read_pipe_path);
                return false;
            }
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
                execl("./Monitor", "Monitor", monitors[i]->write_pipe_path, monitors[i]->read_pipe_path, NULL);
                _Exit(EXIT_FAILURE);
            default:
                // Parent
                monitors[i]->process_id = pid;
                // send subdirs to child...
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
    // Delete the Monitor pipes and create them again.
    unlink(monitor->write_pipe_path);
    unlink(monitor->read_pipe_path);
    if (mkfifo(monitor->write_pipe_path, 0666) < 0 )
    {
        perror("Error creating fifo");
        exit(EXIT_FAILURE);
    }
    if (mkfifo(monitor->read_pipe_path, 0666) < 0 )
    {
        perror("Error creating fifo");
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
            execl("./Monitor", "Monitor", monitor->write_pipe_path, monitor->read_pipe_path, NULL);
            _Exit(EXIT_FAILURE);
        default:
            // The Parent will send the required information to the new process
            monitor->process_id = new_pid;
            int fd = open(monitor->write_pipe_path, O_WRONLY);
            if (fd < 0)
            {
                fprintf(stderr, "Failed to open pipe: %s\n", monitor->write_pipe_path);
            }
            // Send buffer size, bloom size and the Country directories assigned to this monitor
            sendInt(fd, buffer_size, buffer, buffer_size);
            sendLongInt(fd, bloom_size, buffer, buffer_size);
            sendInt(fd, monitor->subdirs->getNumElements(), buffer, buffer_size);
            for (LinkedList::ListIterator itr = monitor->subdirs->listHead(); !itr.isNull(); itr.forward())
            {
                sendString(fd, static_cast<char*>(itr.getData()), buffer, buffer_size);
            }
            close(fd);

            // Receive Bloom Filters
            unsigned int num_filters;
            monitor->read_fd = open(monitor->read_pipe_path, O_RDONLY);
            if (monitor->read_fd < 0)
            {
                fprintf(stderr, "Failed to open pipe: %s\n", monitor->write_pipe_path);
                exit(EXIT_FAILURE);
            }
            receiveInt(monitor->read_fd, num_filters, buffer, buffer_size);
            for (unsigned int j = 0; j < num_filters; j++)
            {
                char *virus_name;
                receiveString(monitor->read_fd, virus_name, buffer, buffer_size);
                VirusFilter *virus = static_cast<VirusFilter*>(viruses->getElement(virus_name, compareNameVirusFilter));
                if (virus == NULL)
                {
                    viruses->append(new VirusFilter(virus_name, bloom_size));
                    virus = static_cast<VirusFilter*>(viruses->getLast());
                }
                free(virus_name);
                updateBloomFilter(monitor->read_fd, virus->filter, buffer, buffer_size);
            }
            close(monitor->read_fd);
    }
}

/**
 * Restores any dead child Monitor processes.
 */
void checkAndRestoreChildren(MonitorInfo **monitors, unsigned int num_monitors, char *buffer, unsigned int buffer_size,
                             unsigned long int bloom_size, LinkedList *viruses)
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
        }
    }
}

/**
 * Sends required information to the child Monitors.
 */
void sendMonitorData(MonitorInfo **monitors, unsigned int num_monitors, char *buffer, unsigned int buffer_size,
                     unsigned long int bloom_size)
{
    int fd;
    // Iterate over the Monitors
    for(unsigned int i = 0; i < num_monitors; i++)
    {
        fd = open(monitors[i]->write_pipe_path, O_WRONLY);
        if (fd < 0)
        {
            fprintf(stderr, "Failed to open pipe: %s\n", monitors[i]->write_pipe_path);
            exit(EXIT_FAILURE);
        }
        // Send buffer size
        sendInt(fd, buffer_size, buffer, buffer_size);
        // Send bloom filter size
        sendLongInt(fd, bloom_size, buffer, buffer_size);
        // Send number of directories for this monitor
        sendInt(fd, monitors[i]->subdirs->getNumElements(), buffer, buffer_size);
        // Send the directory paths
        for (LinkedList::ListIterator itr = monitors[i]->subdirs->listHead(); !itr.isNull(); itr.forward())
        {
            sendString(fd, static_cast<char*>(itr.getData()), buffer, buffer_size);
        }
        close(fd);
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
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    FD_ZERO(&fdset);

    unsigned int done_monitors = 0;
    int max_fd = -1;
    // Open read pipe for each child Monitor
    for (unsigned int i = 0; i < num_monitors; i++)
    {
        int fd = open(monitors[i]->read_pipe_path, O_RDONLY);
        if (fd < 0)
        {
            fprintf(stderr, "Failed to open pipe: %s\n", monitors[i]->write_pipe_path);
            exit(EXIT_FAILURE);
        }
        // Add each FD to the set and find max FD to pass to select()
        FD_SET(fd, &fdset);
        monitors[i]->read_fd = fd;
        if (max_fd < monitors[i]->read_fd)
        {
            max_fd = monitors[i]->read_fd;
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
                if (FD_ISSET(monitors[i]->read_fd, &fdset))
                // The read FD of this monitor is ready
                {
                    // Receive the Bloom Filters sent by this Monitor
                    unsigned int num_filters;
                    // Receive number of sent Bloom Filters
                    receiveInt(monitors[i]->read_fd, num_filters, buffer, buffer_size);
                    for (unsigned int j = 0; j < num_filters; j++)
                    {
                        char *virus_name;
                        // Receive the name of the virus related with this Bloom Filter
                        receiveString(monitors[i]->read_fd, virus_name, buffer, buffer_size);
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
                        updateBloomFilter(monitors[i]->read_fd, virus->filter, buffer, buffer_size);
                    }
                    done_monitors++;
                }
            }
        }
        // Clear the set and add all FD's again
        FD_ZERO(&fdset);
        for (unsigned int i = 0; i < num_monitors; i++)
        {
            FD_SET(monitors[i]->read_fd, &fdset);
        }
    }

    // Close all the read FD's.
    for (unsigned int i = 0; i < num_monitors; i++)
    {
        close(monitors[i]->read_fd);
    }
}

/**
 * Terminates all Monitor processes and deletes the created pipes.
 */
void terminateChildren(MonitorInfo **monitors, unsigned int num_monitors)
{
    for(unsigned int i = 0; i < num_monitors; i++)
    {
        kill(monitors[i]->process_id, SIGKILL);
        waitpid(monitors[i]->process_id, NULL, 0);
        unlink(monitors[i]->read_pipe_path);
        unlink(monitors[i]->write_pipe_path);
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
