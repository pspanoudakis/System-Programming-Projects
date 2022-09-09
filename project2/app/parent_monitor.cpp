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
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sstream>

#include "pipe_msg.hpp"
#include "../include/linked_list.hpp"
#include "../include/bloom_filter.hpp"
#include "../include/rb_tree.hpp"
#include "parent_monitor_utils.hpp"
#include "app_utils.hpp"
#include "parse_utils.hpp"

#define MAX_MONITORS 300
#define MAX_BUFFER_SIZE 1000000

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
 * Checks if the /addVaccinationRecords command was valid and
 * stores the country name in the given variable.
 */
bool addVaccinationRecordsParse(char *&country_name)
{
    char *arg;                  // strtok tokens are stored here
    country_name = NULL;          // Initializing pointer argument to NULL

    // Try to obtain the argument
    arg = strtok(NULL, " ");
    if (arg == NULL)
    // Could not obtain any arguments
    {
        printf("Expected a country name. Rejecting command.\n");
        return false;
    }
    // Stored the Virus Name
    country_name = new char[strlen(arg) + 1];
    strcpy(country_name, arg);

    // See if there are other given arguments
    arg = strtok(NULL, " ");
    if (arg != NULL)
    // There are, which is unexpected.
    {
        printf("More than expected arguments found. Rejecting command.\n");
        return false;
    }
    return true;
}

/**
 * Checks if the /searchVaccinationStatus command was valid and
 * stores the citizen ID in the given variable.
 */
bool searchVaccinationStatusParse(unsigned int &citizen_id)
{
    char *arg;
    // Try to obtain the argument
    arg = strtok(NULL, " ");
    if (arg == NULL)
    // Could not obtain any arguments
    {
        printf("Expected a citizen ID. Rejecting command.\n");
        return false;
    }
    // Stored the Virus Name
    if (!isPositiveNumber(arg) || strlen(arg) > MAX_ID_DIGITS)
    {
        printf("Invalid Citizen ID deteted. Make sure it is an up-to %d digits number.\n", MAX_ID_DIGITS);
        return false;
    }
    citizen_id = atoi(arg);
    // See if there are other given arguments
    arg = strtok(NULL, " ");
    if (arg != NULL)
    // There are, which is unexpected.
    {
        printf("More than expected arguments found. Rejecting command.\n");
        return false;
    }
    return true;
}

/**
 * Checks if the /travelRequest command was valid and
 * stores the user arguments in the given variables.
 */
bool travelRequestParse(unsigned int &citizen_id, Date &date, char *&countryFrom, char *&countryTo, char *&virus_name)
{
    short int curr_arg = 0;     // This indicates which argument is examined
    char *token;                // This is used to obtain the token returned by strtok
    
    // Initializing the given arguments to NULL
    countryFrom = NULL;
    countryTo = NULL;
    virus_name = NULL;
    date.set(0, 0, 0);

    // Loop until strtok no longer gives other tokens or more than expected arguments detected
    while ( (token = strtok(NULL, " "))!= NULL && curr_arg < 5 )
    {
        switch (curr_arg)
        {
            case 0:
                // token is citizen_id
                if (!isPositiveNumber(token) || strlen(token) > MAX_ID_DIGITS)
                {
                    printf("Invalid Citizen ID deteted. Make sure it is an up-to %d digits number.\n",
                            MAX_ID_DIGITS);
                    return false;
                }
                citizen_id = atoi(token);
                break;
            case 1:
                // token is date
                if (!parseDateString(token, date))
                // Could not parse Date token successfully 
                {
                    printf("Invalid date argument detected. Rejecting command.\n");
                    return false;
                }
                break;
            case 2:
                // token is countryFrom
                countryFrom = new char [strlen(token)+1];
                strcpy(countryFrom, token);
                break;
            case 3:
                // token is countryTo
                countryTo = new char [strlen(token)+1];
                strcpy(countryTo, token);
                break;
            case 4:
                // token is virus
                virus_name = new char [strlen(token)+1];
                strcpy(virus_name, token);
                break;
            default:
                break;
        }
        curr_arg++;
    }
    // Checking how many arguments have been read
    if (curr_arg == 5)
    {
        return true;
    }
    else if (curr_arg < 5)
    {
        printf("Less than expected arguments have been detected. Rejecting command.\n");
    }
    else
    {
        printf("More than expected arguments have been detected. Rejecting command.\n");
    }
    return false;
}

/**
 * Checks if the /travelStats command was valid and
 * stores the user arguments in the given variables.
 */
bool travelStatsParse(char *&virus_name, Date &start, Date &end, char *&country_name)
{
    char **args = new char*[4];     // The string arguments will be stored here initially
    short int curr_arg = 0;         // This indicates which argument is examined
    char *token;                    // This is used to obtain the token returned by strtok

    // Initializing given arguments to NULL
    start.set(0, 0, 0);
    end.set(0, 0, 0);
    country_name = NULL;
    virus_name = NULL;

    // Get all the user arguments
    while ( (token = strtok(NULL, " "))!= NULL && curr_arg < 4)
    {
        args[curr_arg] = token;
        curr_arg++;
    }
    if (token != NULL)
    // Reject command if arguments were more than expected
    {
        printf("More than expected arguments have been detected. Rejecting command.\n");
        delete[] args;
        return false;
    }
    switch (curr_arg)
    // Exam the number of given arguments, and parse accordingly
    {
        case 0:
        case 1:
        case 2:
            printf("Less than expected arguments have been detected. Rejecting command.\n");
            delete[] args;
            return false;
        case 3:
            // args are: virus date1 date2
            virus_name = new char[strlen(args[0])+1];
            strcpy(virus_name, args[0]);

            if ( !(parseDateString(args[1], start) && parseDateString(args[2], end)) )
            {
                delete[] args;
                printf("Invalid date argument detected. Rejecting command.\n");
                return false;
            }
            delete[] args;
            if (compareDates(start, end) > 0)
            {
                printf("The first Date cannot be greater than the second one. Rejecting command.\n");
                return false;
            }
            return true;
        case 4:
            // args are: virus date1 date2 country
            virus_name = new char[strlen(args[0])+1];
            strcpy(virus_name, args[0]);
            country_name = new char[strlen(args[3])+1];
            strcpy(country_name, args[3]);

            if ( !(parseDateString(args[1], start) && parseDateString(args[2], end)) )
            {
                delete[] args;
                printf("Invalid date argument detected. Rejecting command.\n");
                return false;
            }
            delete[] args;
            if (compareDates(start, end) > 0)
            {
                printf("The first Date cannot be greater than the second one. Rejecting command.\n");
                return false;
            }
            return true;   
        default:
            // This point should never be reached, but
            // if it does, something really bad has happened :(
            delete[] args;
            return false;
            break;
    }
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
    logfile_name_stream << "log_files/log_file." << getpid();
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

/**
 * Checks and stores the program arguments properly.
 * @returns TRUE if the arguments are valid, FALSE otherwise.
 */
bool checkParseArgs(int argc, char const *argv[], char *&directory_path, unsigned int &num_monitors,
                    unsigned long &bloom_size, unsigned int &buffer_size)
{
    directory_path = NULL;
    if (argc != 9)
    {
        perror("Insufficient/Unexpected number of arguments given.\n");
        printf("Usage: ./travelMonitor -m numMonitors -b bufferSize -s sizeOfBloom -i input_dir\n");
        return false;
    }

    // Flags to make sure no arguments are given more than once
    bool got_num_monitors = false;
    bool got_buffer_size = false;
    bool got_bloom_size = false;
    bool got_input_dir = false;

    for (int i = 1; i < 8; i+=2)
    {
        if ( strcmp(argv[i], "-m") == 0 )
        {
            if (got_num_monitors) {
                perror("Duplicate numMonitors argument detected.\n");
                return false; 
            }
            int temp = atoi(argv[i + 1]);
            if (temp > 0 && temp <= MAX_MONITORS)
            {
                num_monitors = temp;
                got_num_monitors = true;
            }
            else
            {
                fprintf(stderr, "Invalid numMonitors argument. Make sure it is a positive integer up to %d.\n", MAX_MONITORS);
                return false;
            }
        }
        else if ( strcmp(argv[i], "-b") == 0 )
        {
            if (got_buffer_size) { 
                perror("Duplicate bufferSize argument detected.\n");
                return false; 
            }
            int temp = atoi(argv[i + 1]);
            if (temp > 0 && temp <= MAX_BUFFER_SIZE)
            {
                buffer_size = temp;
                got_buffer_size = true;
            }
            else
            {
                fprintf(stderr, "Invalid bufferSize argument. Make sure it is a positive integer up to %d", MAX_BUFFER_SIZE);
                return false;
            }
        }
        else if ( strcmp(argv[i], "-s") == 0 )
        {
            if (got_bloom_size) { 
                perror("Duplicate sizeOfBloom argument detected.\n");
                return false;
            }
            long temp = atol(argv[i + 1]);
            if (temp > 0 && temp <= MAX_BLOOM_SIZE)
            {
                bloom_size = temp;
                got_bloom_size = true;
            }
            else
            {
                fprintf(stderr, "Invalid sizeOfBloom argument. Make sure it is a positive integer up to %d", MAX_BLOOM_SIZE);
                return false;
            }
        }
        else if ( strcmp(argv[i], "-i") == 0 )
        {
            if (got_input_dir) {
                perror("Duplicate input_dir argument detected.\n");
                return false;
            }
            directory_path = copyString(argv[i + 1]);
            got_input_dir = true;
        }
        else
        {
            perror("Invalid argument detected.\n");
            return false;
        }
    }
    return true;
}

int main(int argc, char const *argv[])
{
    // Register signal handlers
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);

    unsigned int num_monitors, active_monitors, num_dirs, num_countries, buffer_size;
    unsigned long bloom_size;
    char *directory_path;
    MonitorInfo **monitors;
    CountryMonitor **countries;
    LinkedList *viruses;
    struct dirent **directories;
    
    // Check given arguments and store them
    if (!checkParseArgs(argc, argv, directory_path, num_monitors, bloom_size, buffer_size))
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

    // Create child Monitors
    createMonitors(monitors, num_monitors, active_monitors);

    char *buffer = new char[buffer_size];
    // Send required information to child Monitors
    sendMonitorData(monitors, active_monitors, buffer, buffer_size, bloom_size);

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
            checkAndRestoreChildren(monitors, active_monitors, buffer, buffer_size, bloom_size, viruses, sigchld_received);
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

    terminateChildren(monitors, active_monitors);
    createLogFile(countries, num_countries, accepted_requests, rejected_requests);

    delete[] directory_path;
    delete[] buffer;
    releaseResources(countries, monitors, num_monitors, directories, num_dirs, viruses);
}
