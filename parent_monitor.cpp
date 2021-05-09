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

    pause();

    terminateChildren(monitors, active_monitors);

    releaseResources(countries, monitors, num_monitors, directories, num_dirs, viruses);
}
