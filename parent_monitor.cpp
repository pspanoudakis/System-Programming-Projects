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

void parseExecuteCommand(char *command, unsigned long &bloom_size, char *buffer, unsigned int buffer_size,
                         CountryMonitor **countries, LinkedList *viruses, MonitorInfo **monitors,
                         unsigned int num_countries, unsigned int active_monitors)
{

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
