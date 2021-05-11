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

#include "pipe_msg.hpp"
#include "include/linked_list.hpp"
#include "parent_monitor_utils.hpp"
#include "app/app_utils.hpp"
#include "app/parse_utils.hpp"

bool sigchld_received = false;
bool terminate = false;

void sigint_handler(int s)
{
    terminate = true;
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigint_handler);
}

void sigchld_handler(int s)
{
    sigchld_received = true;
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

bool searchVaccinationStatusParse(int &citizen_id)
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

bool travelRequestParse()
{

}

bool travelStatsParse()
{

}

void travelRequest(unsigned int citizen_id, Date &date, const char *country_name, const char *virus_name,
                   LinkedList *viruses, MonitorInfo **monitors, unsigned int active_monitors,
                   CountryMonitor **countries, unsigned int num_countries)
{

}

void travelStats(const char *virus_name, Date &start, Date &end, const char *country_name,
                 CountryMonitor **countries, unsigned int num_countries)
{
    
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
                updateBloomFilter(countries[i]->monitor->read_fd, *(virus->filter), buffer, buffer_size);
            }
            printf("Records updated successfully.\n");
            return;
        }
    }
    printf("The specified country was not found.\n");
}

void parseExecuteCommand(char *command, unsigned long bloom_size, char *buffer, unsigned int buffer_size,
                         CountryMonitor **countries, LinkedList *viruses, MonitorInfo **monitors,
                         unsigned int num_countries, unsigned int active_monitors)
{
    // Variables used for storing command parameters
    int citizen_id;
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

        }
        else if (strcmp(token, "/travelStats") == 0)
        {
            
        }
        else if (strcmp(token, "/addVaccinationRecords") == 0)
        {
            if (addVaccinationRecordsParse(country_name))
            {
                addVaccinationRecords(country_name, countries, num_countries, viruses, buffer, buffer_size, bloom_size);
            }
            delete[] country_name;
        }
        else if (strcmp(token, "/searchVaccinationRecords") == 0)
        {
            
        }
        else
        {
            printf("Unknown command.\n");
        }
    }
}

//-------------------------------------------------------------------------------------------------

int main(int argc, char const *argv[])
{
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);

    unsigned int num_monitors = 3;
    unsigned int active_monitors;
    MonitorInfo **monitors;
    CountryMonitor **countries;
    LinkedList *viruses;
    struct dirent **directories;
    unsigned int num_dirs;
    unsigned int num_countries;

    if (!assignMonitorDirectories("./countries", countries, monitors, num_monitors, directories, num_dirs))
    {
        exit(EXIT_FAILURE);
    }
    num_countries = num_dirs - 2;

    createMonitors(monitors, num_monitors, active_monitors);

    char buffer[4];
    unsigned int buffer_size = 4;
    unsigned long int bloom_size = 100;
    sendMonitorData(monitors, active_monitors, buffer, buffer_size, bloom_size);

    viruses = new LinkedList(delete_object<VirusFilter>);

    receiveMonitorFilters(monitors, active_monitors, viruses, buffer, buffer_size, bloom_size);

    char *line_buf;
    while ( !terminate )
    {
        pause();
        line_buf = fgetline(stdin);
        if (line_buf == NULL)
        {
            continue;
        }
        if (strcmp(line_buf, "/exit") == 0)
        // Exit if asked by the user
        {
            terminate = true;
            continue;
        }
        // Otherwise, try to parse the line into a command and execute it.
        parseExecuteCommand(line_buf, bloom_size, buffer, buffer_size, countries, viruses, monitors, num_countries, active_monitors);
        free(line_buf);
    }

    terminateChildren(monitors, active_monitors);
    releaseResources(countries, monitors, num_monitors, directories, num_dirs, viruses);
}
