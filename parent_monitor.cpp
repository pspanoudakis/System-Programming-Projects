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

int main(int argc, char const *argv[])
{
    unsigned int num_monitors = 3;
    MonitorInfo **monitors;
    CountryMonitor **countries;
    struct dirent **directories;
    unsigned int num_files;

    assignMonitorDirectories("./countries", countries, monitors, num_monitors, directories, num_files);
    

    releaseResources(countries, monitors, num_monitors, directories, num_files);
}
