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

#define MAX_BLOOM_SIZE 1000000          // Maximum Bloom Filter size allowed
#define MAX_MONITORS 300
#define MAX_BUFFER_SIZE 1000000

int sigchld_received = 0;
bool terminate = false;

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

bool travelRequestParse(unsigned int &citizen_id, Date &date, char *&country_name, char *&virus_name)
{
    short int curr_arg = 0;     // This indicates which argument is examined
    char *token;                // This is used to obtain the token returned by strtok
    
    // Initializing the given arguments to NULL
    country_name = NULL;
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
                country_name = new char [strlen(token)+1];
                strcpy(country_name, token);
                break;
            case 3:
                // token is countryTo
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

void addVaccinationRecords(const char *country_name, CountryMonitor **countries, unsigned int num_countries,
                           LinkedList *viruses, char *buffer, unsigned int buffer_size, unsigned long int bloom_size)
{
    for (unsigned int i = 0; i < num_countries; i++)
    {
        if (strcmp(country_name, countries[i]->country_name) == 0)
        {
            // TODO: Check if monitor is alive?
            kill(countries[i]->monitor->process_id, SIGUSR1);
            unsigned int num_filters;
            countries[i]->monitor->read_fd = open(countries[i]->monitor->read_pipe_path, O_RDONLY);
            if (countries[i]->monitor->read_fd < 0)
            {

            }
            receiveInt(countries[i]->monitor->read_fd, num_filters, buffer, buffer_size);
            for (unsigned int j = 0; j < num_filters; j++)
            {
                char *virus_name;
                receiveString(countries[i]->monitor->read_fd, virus_name, buffer, buffer_size);
                VirusFilter *virus = static_cast<VirusFilter*>(viruses->getElement(virus_name, compareNameVirusFilter));
                if (virus == NULL)
                {
                    viruses->append(new VirusFilter(virus_name, bloom_size));
                    virus = static_cast<VirusFilter*>(viruses->getLast());
                }
                free(virus_name);
                updateBloomFilter(countries[i]->monitor->read_fd, virus->filter, buffer, buffer_size);
            }
            printf("Records updated successfully.\n");
            return;
        }
    }
    printf("The specified country was not found.\n");
}

void searchVaccinationStatus(unsigned int citizen_id, MonitorInfo **monitors, unsigned int active_monitors,
                             char *buffer, unsigned int buffer_size)
{
    fd_set fdset;
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    FD_ZERO(&fdset);

    unsigned int done_monitors = 0;
    int max_fd = -1;
    for (unsigned int i = 0; i < active_monitors; i++)
    {
        int read_fd = open(monitors[i]->read_pipe_path, O_RDONLY);
        if (read_fd < 0)
        {
            
        }
        int write_fd = open(monitors[i]->write_pipe_path, O_WRONLY);
        if (write_fd < 0)
        {

        }
        sendMessageType(write_fd, SEARCH_STATUS, buffer, buffer_size);
        sendInt(write_fd, citizen_id, buffer, buffer_size);
        kill(monitors[i]->process_id, SIGUSR2);
        close(write_fd);
        FD_SET(read_fd, &fdset);
        monitors[i]->read_fd = read_fd;
        if (max_fd < monitors[i]->read_fd)
        {
            max_fd = monitors[i]->read_fd;
        }
    }

    while (done_monitors != active_monitors)
    {
        int ready_fds;
        ready_fds = select(max_fd + 1, &fdset, NULL, NULL, &timeout);
        if (ready_fds == -1)
        {

        }
        if (ready_fds != 0)
        {
            for (unsigned int i = 0; i < active_monitors; i++)
            {
                if (FD_ISSET(monitors[i]->read_fd, &fdset))
                {
                    char msg_type;
                    receiveMessageType(monitors[i]->read_fd, msg_type, buffer, buffer_size);
                    if (msg_type == CITIZEN_FOUND)
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
        FD_ZERO(&fdset);
        for (unsigned int i = 0; i < active_monitors; i++)
        {
            FD_SET(monitors[i]->read_fd, &fdset);
        }
    }

    for (unsigned int i = 0; i < active_monitors; i++)
    {
        close(monitors[i]->read_fd);
    }
}

void travelRequest(unsigned int citizen_id, Date &date, char *country_name, char *virus_name,
                   LinkedList *viruses, MonitorInfo **monitors, unsigned int active_monitors,
                   CountryMonitor **countries, unsigned int num_countries,
                   char *buffer, unsigned int buffer_size, unsigned int &accepted_requests, unsigned int &rejected_requests)
{
    VirusFilter *target_virus = static_cast<VirusFilter*>(viruses->getElement(virus_name, compareNameVirusFilter));
    if (target_virus == NULL)
    {
        printf("ERROR: The specified virus was not found.\n");
    }
    else
    {
        CountryMonitor *target_country = NULL;
        for (unsigned int i = 0; i < num_countries; i++)
        {
            if (strcmp(country_name, countries[i]->country_name) == 0)
            {
                target_country = countries[i];
                break;
            }
        }
        if (target_country == NULL)
        {
            printf("ERROR: The specified country was not found.\n");
        }
        else
        {
            char char_id[MAX_ID_DIGITS + 1]; // max digits + \0
            sprintf(char_id, "%d", citizen_id);
            bool accepted = false;
            if (target_virus->filter->isPresent(char_id))
            {
                target_country->monitor->read_fd = open(target_country->monitor->read_pipe_path, O_RDONLY);
                if (target_country->monitor->read_fd < 0)
                {

                }
                int write_fd = open(target_country->monitor->write_pipe_path, O_WRONLY);
                if (write_fd < 0)
                {

                }
                char ans_type;
                char *answer;
                sendMessageType(write_fd, TRAVEL_REQUEST, buffer, buffer_size);
                sendInt(write_fd, citizen_id, buffer, buffer_size);
                sendDate(write_fd, date, buffer, buffer_size);
                sendString(write_fd, virus_name, buffer, buffer_size);
                kill(target_country->monitor->process_id, SIGUSR2);
                
                receiveMessageType(target_country->monitor->read_fd, ans_type, buffer, buffer_size);
                receiveString(target_country->monitor->read_fd, answer, buffer, buffer_size);
                printf("%s", answer);
                free(answer);
                accepted = (ans_type == TRAVEL_REQUEST_ACCEPTED);
                close(write_fd);
                close(target_country->monitor->read_fd);
            }
            else
            {
                printf("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");
            }
            VirusRequests *requests = static_cast<VirusRequests*>(target_country->virus_requests->getElement(virus_name, compareNameVirusRequests));
            if (requests == NULL)
            {
                target_country->virus_requests->append(new VirusRequests(virus_name));
                requests = static_cast<VirusRequests*>(target_country->virus_requests->getLast());
            }
            requests->requests_tree->insert(new TravelRequest(date, accepted));
            accepted ? accepted_requests++ : rejected_requests++;
        }
    }
}

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
            // root is in range, so update the counter
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

void getTravelStatsRec(RBTreeNode *root, unsigned int &accepted, unsigned int &rejected)
{
    if (root == NULL) { return; }

    getTravelStatsRec(root->left, accepted, rejected);

    // Getting the root Record
    TravelRequest *root_data = static_cast<TravelRequest*>(root->data);
    root_data->accepted ? accepted++ : rejected++;

    getTravelStatsRec(root->right, accepted, rejected);
}

void travelStats(char *virus_name, Date &start, Date &end, CountryMonitor **countries, unsigned int num_countries)
{
    unsigned int accepted_requests = 0;
    unsigned int rejected_requests = 0;
    for (int i = 0; i < num_countries; i++)
    {
        VirusRequests *virus_requests = static_cast<VirusRequests*>(countries[i]->virus_requests->getElement(virus_name, compareNameVirusRequests));
        if (virus_requests != NULL)
        {
            getTravelStatsRec(virus_requests->requests_tree->root, start, end, accepted_requests, rejected_requests);
        }
    }
    printf("TOTAL REQUESTS %d\n", accepted_requests + rejected_requests);
    printf("ACCEPTED %d\n", accepted_requests);
    printf("REJECTED %d\n", rejected_requests);
}

void travelStats(char *virus_name, Date &start, Date &end, const char *country_name,
                 CountryMonitor **countries, unsigned int num_countries)
{
    if (country_name == NULL)
    {
        travelStats(virus_name, start, end, countries, num_countries);
        return;
    }
    unsigned int accepted_requests = 0;
    unsigned int rejected_requests = 0;
    for (int i = 0; i < num_countries; i++)
    {
        if (strcmp(country_name, countries[i]->country_name) == 0)
        {
            VirusRequests *virus_requests = static_cast<VirusRequests*>(countries[i]->virus_requests->getElement(virus_name, compareNameVirusRequests));
            if (virus_requests != NULL)
            {
                getTravelStatsRec(virus_requests->requests_tree->root, start, end, accepted_requests, rejected_requests);
            }
            break;
        }
    }
    printf("TOTAL REQUESTS %d\n", accepted_requests + rejected_requests);
    printf("ACCEPTED %d\n", accepted_requests);
    printf("REJECTED %d\n", rejected_requests);
}

void parseExecuteCommand(char *command, unsigned long bloom_size, char *buffer, unsigned int buffer_size,
                         CountryMonitor **countries, LinkedList *viruses, MonitorInfo **monitors,
                         unsigned int num_countries, unsigned int active_monitors,
                         unsigned int &accepted, unsigned int &rejected)
{
    // Variables used for storing command parameters
    unsigned int citizen_id;
    char *country_name, *virus_name;
    Date date, start, end;

    char *token = strtok(command, " ");
    if (token != NULL)
    // Recognizing the main command
    {
        // Call the corresponding parse routine and execution routine, if there are no errors.
        // Release any temporarily allocated memory at the end.
        if (strcmp(token, "/travelRequest") == 0)
        {
            if (travelRequestParse(citizen_id, date, country_name, virus_name))
            {
                travelRequest(citizen_id, date, country_name, virus_name, viruses, monitors, active_monitors,
                              countries, num_countries, buffer, buffer_size, accepted, rejected);
            }
            delete[] country_name;
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

void createLogFile(CountryMonitor **countries, unsigned int num_countries,
                   unsigned int accepted_requests, unsigned int rejected_requests)
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
    for (unsigned int i = 0; i < num_countries; i++)
    {
        fprintf(logfile, "%s\n", countries[i]->country_name);
    }
    
    fprintf(logfile, "TOTAL TRAVEL REQUESTS %d\n", accepted_requests + rejected_requests);
    fprintf(logfile, "ACCEPTED %d\n", accepted_requests);
    fprintf(logfile, "REJECTED %d\n", rejected_requests);
    
    fclose(logfile);
}

//-------------------------------------------------------------------------------------------------
/**
 * Checks and stores the program agruments properly.
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
            if (temp > 0 && temp <= MAX_MONITORS)
            {
                buffer_size = temp;
                got_buffer_size = true;
            }
            else
            {
                fprintf(stderr, "Invalid numMonitors argument. Make sure it is a positive integer up to %d", MAX_BUFFER_SIZE);
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
                fprintf(stderr, "Invalid numMonitors argument. Make sure it is a positive integer up to %d", MAX_BLOOM_SIZE);
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
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);

    unsigned int num_monitors = 3, active_monitors;
    MonitorInfo **monitors;
    CountryMonitor **countries;
    LinkedList *viruses;
    char *directory_path;
    struct dirent **directories;
    unsigned int num_dirs, num_countries, buffer_size;
    unsigned long bloom_size;

    if (!checkParseArgs(argc, argv, directory_path, num_monitors, bloom_size, buffer_size))
    {
        delete[] directory_path;
        exit(EXIT_FAILURE);
    }
    if (!assignMonitorDirectories("./countries", countries, monitors, num_monitors, directories, num_dirs))
    {
        exit(EXIT_FAILURE);
    }
    num_countries = num_dirs - 2;   // num_dirs counts "." and ".." as well

    createMonitors(monitors, num_monitors, active_monitors);

    char *buffer = new char[buffer_size];
    sendMonitorData(monitors, active_monitors, buffer, buffer_size, bloom_size);

    viruses = new LinkedList(delete_object<VirusFilter>);
    receiveMonitorFilters(monitors, active_monitors, viruses, buffer, buffer_size, bloom_size);

    char *line_buf;
    unsigned int accepted_requests = 0;
    unsigned int rejected_requests = 0;
    while ( !terminate )
    {
        printf("------------------------------------------\n");
        line_buf = fgetline(stdin);
        if (line_buf == NULL)
        {
            continue;
        }
        if (sigchld_received > 0)
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

    terminateChildren(monitors, active_monitors);
    createLogFile(countries, num_countries, accepted_requests, rejected_requests);

    delete[] directory_path;
    delete[] buffer;
    releaseResources(countries, monitors, num_monitors, directories, num_dirs, viruses);
}
