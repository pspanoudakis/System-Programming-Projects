/**
 * File: parent_monitor_utils.hpp
 * Classes & Routines used by the Parent Monitor.
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef PARENT_MONITOR_UTILS_HPP
#define PARENT_MONITOR_UTILS_HPP

#include "app_utils.hpp"

/**
 * Stores information about a specific child Monitor process.
 */
class MonitorInfo
{
    public:
        int process_id;                 // The Monitor process ID.
        int read_fd;                    // A slot to store a read FD (used when using multiple Monitors).
        const char *write_pipe_path;    // The path of the fifo pipe used for sending data to the Monitor.
        const char *read_pipe_path;     // The path of the fifo pipe used for receiving data from the Monitor.
        LinkedList *subdirs;            // A list with the subdirectories assigned to this Monitor.
        MonitorInfo();
        ~MonitorInfo();
};

/**
 * Stores Travel Requests, sorted by date in a Red-Black Tree, about a specific virus.
 */
class VirusRequests
{
    public:
        const char *virus_name;
        RedBlackTree *requests_tree;
        VirusRequests(const char *name);
        ~VirusRequests();
};

/**
 * Stores the monitor that has access to a specific country directory,
 * as the VirusRequests objects related to this country.
 */
class CountryMonitor
{
    public:
        const char *country_name;
        MonitorInfo *monitor;
        LinkedList *virus_requests;
        CountryMonitor(const char *name, MonitorInfo *monitor_info);
        ~CountryMonitor();
};

/**
 * Represents a Travel Request in a specific date, which has been
 * either accepted or rejected.
 */
class TravelRequest
{
    public:
        Date date;
        const bool accepted;
        TravelRequest(Date &request_date, bool is_accepted);
        ~TravelRequest(); 
};

/**
 * Stores a Bloom Filter related to a specific virus.
 */
class VirusFilter
{
    public:
        const char *virus_name;
        BloomFilter *filter;
        VirusFilter(const char *name, unsigned long size);
        ~VirusFilter();
};

/* Parent Monitor routines --------------------------------------------------------------------- */

bool assignMonitorDirectories(char *path, CountryMonitor **&countries, MonitorInfo **&monitors, unsigned int num_monitors,
                              struct dirent **&files, unsigned int &num_files);

void createMonitors(MonitorInfo **monitors, unsigned int num_monitors, unsigned int &active_monitors);

void restoreChild(MonitorInfo *monitor, char *buffer, unsigned int buffer_size, unsigned long int bloom_size,
                  LinkedList *viruses);

void checkAndRestoreChildren(MonitorInfo **monitors, unsigned int num_monitors, char *buffer, unsigned int buffer_size,
                             unsigned long int bloom_size, LinkedList *viruses, int &sigchld_counter);

void sendMonitorData(MonitorInfo **monitors, unsigned int num_monitors, char *buffer, unsigned int buffer_size,
                     unsigned long int bloom_size);

void receiveMonitorFilters(MonitorInfo **monitors, unsigned int num_monitors, LinkedList *viruses,
                           char *buffer, unsigned int buffer_size, unsigned long int bloom_size);

void addVaccinationRecords(const char *country_name, CountryMonitor **countries, unsigned int num_countries,
                           LinkedList *viruses, char *buffer, unsigned int buffer_size, unsigned long int bloom_size);

void searchVaccinationStatus(unsigned int citizen_id, MonitorInfo **monitors, unsigned int active_monitors,
                             char *buffer, unsigned int buffer_size);

void travelRequest(unsigned int citizen_id, Date &date, char *country_from, char *country_to, char *virus_name,
                   LinkedList *viruses, MonitorInfo **monitors, unsigned int active_monitors,
                   CountryMonitor **countries, unsigned int num_countries,
                   char *buffer, unsigned int buffer_size, unsigned int &accepted_requests, unsigned int &rejected_requests); 

void getTravelStatsRec(RBTreeNode *root, Date &start, Date &end, unsigned int &accepted, unsigned int &rejected);

void travelStats(char *virus_name, Date &start, Date &end, CountryMonitor **countries, unsigned int num_countries);

void travelStats(char *virus_name, Date &start, Date &end, const char *country_name,
                 CountryMonitor **countries, unsigned int num_countries); 

void terminateChildren(MonitorInfo **monitors, unsigned int num_monitors);

void releaseResources(CountryMonitor **countries, MonitorInfo **monitors, unsigned int num_monitors,
                      struct dirent **directories, unsigned int num_dirs, LinkedList *viruses);

/* Comparison functions to be used by ADT's ----------------------------------------------------- */

int compareNameVirusFilter(void *name, void *filter);
int compareTravelRequests(void *r1, void *r2);
int compareNameVirusRequests(void *name, void *req);

#endif
