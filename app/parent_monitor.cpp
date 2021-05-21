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

//-------------------------------------------------------------------------------------------------

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
            // Open the read pipe for this Monitor to receive data
            countries[i]->monitor->read_fd = open(countries[i]->monitor->read_pipe_path, O_RDONLY);
            if (countries[i]->monitor->read_fd < 0)
            {
                perror("Failed to open read pipe.\n");
                exit(EXIT_FAILURE);
            }
            // Receive the number of bloom filters that were sent
            receiveInt(countries[i]->monitor->read_fd, num_filters, buffer, buffer_size);
            // Receive the bloom filters
            for (unsigned int j = 0; j < num_filters; j++)
            {
                char *virus_name;
                // Get the name of the virus related to this bloom filter
                receiveString(countries[i]->monitor->read_fd, virus_name, buffer, buffer_size);
                VirusFilter *virus = static_cast<VirusFilter*>(viruses->getElement(virus_name, compareNameVirusFilter));
                if (virus == NULL)
                // If a new virus was detected, store a new Bloom Filter for it
                {
                    viruses->append(new VirusFilter(virus_name, bloom_size));
                    virus = static_cast<VirusFilter*>(viruses->getLast());
                }
                free(virus_name);
                // Receive the new Bloom Filter and update the present one
                updateBloomFilter(countries[i]->monitor->read_fd, virus->filter, buffer, buffer_size);
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
        // Open read and write pipes form this monitor
        int read_fd = open(monitors[i]->read_pipe_path, O_RDONLY);
        if (read_fd < 0)
        {
            fprintf(stderr, "Failed to open pipe: %s\n", monitors[i]->read_pipe_path);
            exit(EXIT_FAILURE);
        }
        int write_fd = open(monitors[i]->write_pipe_path, O_WRONLY);
        if (write_fd < 0)
        {
            fprintf(stderr, "Failed to open pipe: %s\n", monitors[i]->write_pipe_path);
            exit(EXIT_FAILURE);
        }
        // Send the request to all Monitors and notify with SIGUSR2
        sendMessageType(write_fd, SEARCH_STATUS, buffer, buffer_size);
        sendInt(write_fd, citizen_id, buffer, buffer_size);
        kill(monitors[i]->process_id, SIGUSR2);
        // No more writing needed
        close(write_fd);
        // Add each FD to the set and find max FD to pass to select()
        FD_SET(read_fd, &fdset);
        monitors[i]->read_fd = read_fd;
        if (max_fd < monitors[i]->read_fd)
        {
            max_fd = monitors[i]->read_fd;
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
                if (FD_ISSET(monitors[i]->read_fd, &fdset))
                // The read FD of this monitor is ready
                {
                    // Receive answer type
                    char msg_type;
                    receiveMessageType(monitors[i]->read_fd, msg_type, buffer, buffer_size);
                    if (msg_type == CITIZEN_FOUND)
                    // Citizen found, so receive the answer string
                    {
                        char *answer;
                        receiveString(monitors[i]->read_fd, answer, buffer, buffer_size);
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
            FD_SET(monitors[i]->read_fd, &fdset);
        }
    }
    // Close all the read FD's.
    for (unsigned int i = 0; i < active_monitors; i++)
    {
        close(monitors[i]->read_fd);
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
                // Open pipes for countryFrom Monitor
                target_country_from->monitor->read_fd = open(target_country_from->monitor->read_pipe_path, O_RDONLY);
                if (target_country_from->monitor->read_fd < 0)
                {
                    fprintf(stderr, "Failed to open pipe: %s\n", target_country_from->monitor->read_pipe_path);
                    exit(EXIT_FAILURE);
                }
                int write_fd = open(target_country_from->monitor->write_pipe_path, O_WRONLY);
                if (write_fd < 0)
                {
                    fprintf(stderr, "Failed to open pipe: %s\n", target_country_from->monitor->write_pipe_path);
                    exit(EXIT_FAILURE);
                }
                char ans_type;
                char *answer;
                // Send Request type and required information
                sendMessageType(write_fd, TRAVEL_REQUEST, buffer, buffer_size);
                sendInt(write_fd, citizen_id, buffer, buffer_size);
                sendDate(write_fd, date, buffer, buffer_size);
                sendString(write_fd, virus_name, buffer, buffer_size);
                // Notify the Monitor process
                kill(target_country_from->monitor->process_id, SIGUSR2);
                
                // Receive Monitor answer
                receiveMessageType(target_country_from->monitor->read_fd, ans_type, buffer, buffer_size);
                receiveString(target_country_from->monitor->read_fd, answer, buffer, buffer_size);
                printf("%s", answer);
                free(answer);
                accepted = (ans_type == TRAVEL_REQUEST_ACCEPTED);
                close(write_fd);
                close(target_country_from->monitor->read_fd);
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

//-------------------------------------------------------------------------------------------------

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
    if (!assignMonitorDirectories("./countries", countries, monitors, num_monitors, directories, num_dirs))
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
            checkAndRestoreChildren(monitors, active_monitors, buffer, buffer_size, bloom_size, viruses);
            sigchld_received--;
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
