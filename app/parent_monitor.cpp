/**
 * File: parent_monitor.cpp
 * Parent Monitor main app.
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <sstream>
#include <string>

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include "../include/messaging.hpp"
#include "../include/linked_list.hpp"
#include "../include/bloom_filter.hpp"
#include "../include/rb_tree.hpp"
#include "parent_monitor_utils.hpp"
#include "app_utils.hpp"
#include "parse_utils.hpp"

int sigchld_received = 0;               // If > 0, a child Monitor has died
bool terminate = false;                 // Will be set to true to indicate termination

void sigint_handler(int s)
{
    terminate = true;
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigint_handler);
}

void sigchld_handler(int s)
{
    sigchld_received++;
    signal(SIGCHLD, sigchld_handler);
}

/**
 * Parses and executes the specified command properly. Displays error messages if needed.
 */
void parseExecuteCommand(char *command, unsigned long bloom_size, char *buffer, unsigned int buffer_size,
                         CountryMonitor **countries, LinkedList *viruses, MonitorInfo **monitors,
                         unsigned int num_countries, unsigned int active_monitors,
                         unsigned int &accepted, unsigned int &rejected)
{
    // Variables used for storing command parameters
    unsigned int citizen_id;
    char *country_name, *country_from, *country_to, *virus_name;
    Date date, start, end;

    char *token = strtok(command, " ");
    if (token != NULL)
    {
        // Recognize the command type
        // Call the corresponding parse routine and execution routine, if there are no errors.
        // Release any temporarily allocated memory at the end.
        if (strcmp(token, "/travelRequest") == 0)
        {
            if (travelRequestParse(citizen_id, date, country_from, country_to, virus_name))
            {
                travelRequest(citizen_id, date, country_from, country_to, virus_name, viruses, monitors, active_monitors,
                              countries, num_countries, buffer, buffer_size, accepted, rejected);
            }
            delete[] country_from;
            delete[] country_to;
            delete[] virus_name;
        }
        else if (strcmp(token, "/travelStats") == 0)
        {
            if (travelStatsParse(virus_name, start, end, country_name))
            {
                travelStats(virus_name, start, end, country_name, countries, num_countries);
            }
            delete[] country_name;
            delete[] virus_name;
        }
        else if (strcmp(token, "/addVaccinationRecords") == 0)
        {
            if (addVaccinationRecordsParse(country_name))
            {
                addVaccinationRecords(country_name, countries, num_countries, viruses, buffer, buffer_size, bloom_size);
            }
            delete[] country_name;
        }
        else if (strcmp(token, "/searchVaccinationStatus") == 0)
        {
            if (searchVaccinationStatusParse(citizen_id))
            {
                searchVaccinationStatus(citizen_id, monitors, active_monitors, buffer, buffer_size);
            }
        }
        else
        {
            printf("Unknown command.\n");
        }
    }
}

/**
 * Create the log file of the Parent process
 */
void createLogFile(CountryMonitor **countries, unsigned int num_countries,
                   unsigned int accepted_requests, unsigned int rejected_requests)
{
    // Create logfile name
    std::stringstream logfile_name_stream;
    logfile_name_stream << "./log_files/log_file." << getpid();
    const char *logfile_name = copyString(logfile_name_stream.str().c_str());
    FILE *logfile;

    // Create logfile
    if ((logfile = fopen(logfile_name, "w")) == NULL)
    {
        fprintf(stderr, "Failed to create log file: %s\n", logfile_name);
        delete [] logfile_name;
        return;
    }
    
    delete [] logfile_name;
    // Print country names in the logfile
    for (unsigned int i = 0; i < num_countries; i++)
    {
        fprintf(logfile, "%s\n", countries[i]->country_name);
    }
    // Print counters in the logfile
    fprintf(logfile, "TOTAL TRAVEL REQUESTS %d\n", accepted_requests + rejected_requests);
    fprintf(logfile, "ACCEPTED %d\n", accepted_requests);
    fprintf(logfile, "REJECTED %d\n", rejected_requests);
    // Done
    fclose(logfile);
}

int main(int argc, char const *argv[])
{
    // Register signal handlers
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);

    unsigned int num_monitors, active_monitors, num_dirs, num_countries, buffer_size, cyclic_buffer_size, num_threads;
    unsigned long bloom_size;
    char *directory_path, **child_argv;
    MonitorInfo **monitors;
    CountryMonitor **countries;
    LinkedList *viruses;
    struct dirent **directories;
    
    // Check given arguments and store them
    if (!parentCheckParseArgs(argc, argv, directory_path, num_monitors, bloom_size, buffer_size, cyclic_buffer_size, num_threads))
    {
        delete[] directory_path;
        exit(EXIT_FAILURE);
    }
    // Assign country directories to child Monitors
    if (!assignMonitorDirectories(directory_path, countries, monitors, num_monitors, directories, num_dirs))
    {
        exit(EXIT_FAILURE);
    }
    num_countries = num_dirs - 2;   // num_dirs counts "." and ".." as well

    buildBasicArgv(child_argv, num_threads, buffer_size, cyclic_buffer_size, bloom_size);

    // Create child Monitors
    createMonitors(monitors, num_monitors, active_monitors, child_argv);

    char *buffer = new char[buffer_size];
    // Send required information to child Monitors
    sendMonitorData(monitors, active_monitors, buffer, buffer_size);

    viruses = new LinkedList(delete_object<VirusFilter>);
    // Receive and "merge" Monitor bloom filters
    receiveMonitorFilters(monitors, active_monitors, viruses, buffer, buffer_size, bloom_size);

    char *line_buf;
    unsigned int accepted_requests = 0;
    unsigned int rejected_requests = 0;
    // Loop until SIGINT/SIGQUIT received or users types "/exit"
    printf("Now in command line mode.\n");
    while ( !terminate )
    {
        printf("------------------------------------------\n");
        // Get user command
        line_buf = fgetline(stdin);
        if (line_buf == NULL)
        {
            continue;
        }
        if (sigchld_received > 0)
        // One or more child processes has died, so restore them
        {
            checkAndRestoreChildren(monitors, active_monitors, buffer, buffer_size, bloom_size, viruses,
                                    sigchld_received, child_argv);
        }
        if (strcmp(line_buf, "/exit") == 0)
        // Exit if asked by the user
        {
            terminate = true;
            free(line_buf);
            continue;
        }
        // Otherwise, try to parse the line into a command and execute it.
        parseExecuteCommand(line_buf, bloom_size, buffer, buffer_size, countries, viruses, monitors, num_countries, active_monitors,
                            accepted_requests, rejected_requests);
        free(line_buf);
    }

    // Kill child Monitors, create logfile and release resources

    terminateChildren(monitors, active_monitors, buffer, buffer_size);
    createLogFile(countries, num_countries, accepted_requests, rejected_requests);

    delete[] directory_path;
    delete[] buffer;
    releaseResources(countries, monitors, num_monitors, directories, num_dirs, viruses, child_argv);
}
