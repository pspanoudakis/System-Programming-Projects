#include <unistd.h>
#include <wait.h>
#include <dirent.h>
#include <cstring>

#include "parent_monitor_utils.hpp"
#include "include/linked_list.hpp"
#include "include/utils.hpp"
#include "app/app_utils.hpp"

MonitorInfo::MonitorInfo(): process_id(-1), subdirs(new LinkedList(delete_object_array<char>)) { }

MonitorInfo::~MonitorInfo()
{
    delete subdirs;
}

CountryMonitor::CountryMonitor(const char *name, MonitorInfo *monitor_info):
country_name(copyString(name)), monitor(monitor_info) { }

CountryMonitor::~CountryMonitor()
{
    delete country_name;
}

void assignMonitorDirectories(const char *path, CountryMonitor **&countries, MonitorInfo **&monitors,
                              unsigned int num_monitors, struct dirent **&directories, unsigned int &num_dirs)
{
    unsigned int i;
    num_dirs = scandir(path, &directories, NULL, alphasort);

    countries = new CountryMonitor *[num_dirs];
    for (i = 0; i < num_dirs; i++)
    {
        countries[i] = NULL;
    }

    monitors = new MonitorInfo*[num_monitors];
    for (unsigned int i = 0; i < num_monitors; i++)
    {
        monitors[i] = NULL;
    }

    i = 0;
    for (unsigned int j = 0; j < num_dirs; j++)
    {
        if (strcmp(directories[j]->d_name, ".") == 0 || strcmp(directories[j]->d_name, "..") == 0)
        {
            continue;
        }
        if (monitors[i] == NULL)
        {
            monitors[i] = new MonitorInfo();
        }
        monitors[i]->subdirs->append(copyString(directories[j]->d_name));
        countries[j] = new CountryMonitor(directories[j]->d_name, monitors[i]);
        i++;
        i = i % num_monitors;
    }
}

void createMonitors(MonitorInfo **monitors, unsigned int &num_monitors)
{
    int pid;
    unsigned int i;
    for(i = 0; (i < num_monitors && monitors[i] != NULL); i++)
    {
        pid = fork();
        switch (pid)
        {
            case -1:
                /* problem */
                break;
            case 0:
                /* child */
                execl("./monitor", "monitor", monitors[i]->write_pipe_path, monitors[i]->read_pipe_path, NULL);
            default:
                monitors[i]->process_id = pid;
                // send subdirs to child...
        }
        return;
    }
    if (i < num_monitors)
    {
        num_monitors = i;
    }
    else
    {
        num_monitors = i - 1;
    }
}

void restoreChild(MonitorInfo *monitor)
{
    int new_pid = fork();
    switch (new_pid)
    {
        case -1:
            /* problem */
            break;
        case 0:
            /* child */
            execl("./monitor", "monitor", monitor->write_pipe_path, monitor->read_pipe_path, NULL);
        default:
            monitor->process_id = new_pid;
            // send subdirs to child...
    }
    return;
}

void restoreChild(int pid, MonitorInfo **monitors, unsigned int num_monitors)
{
    for(unsigned int i = 0; i < num_monitors; i++)
    {
        if(monitors[i]->process_id == pid)
        {
            int new_pid = fork();
            switch (new_pid)
            {
                case -1:
                    /* problem */
                    break;
                case 0:
                    /* child */
                    execl("./monitor", "monitor", monitors[i]->write_pipe_path, monitors[i]->read_pipe_path, NULL);
                default:
                    monitors[i]->process_id = new_pid;
                    // send subdirs to child...
            }
            return;
        }
    }
}

void checkAndRestoreChildren(MonitorInfo **monitors, unsigned int num_monitors)
{
    int new_pid;
    int wait_pid;
    for(unsigned int i = 0; i < num_monitors; i++)
    {
        wait_pid = waitpid(monitors[i]->process_id, NULL, WNOHANG);
        if(wait_pid > 0)
        {
            new_pid = fork();
            switch (new_pid)
            {
                case -1:
                    /* problem */
                    break;
                case 0:
                    /* child */
                    execl("./monitor", "monitor", monitors[i]->write_pipe_path, monitors[i]->read_pipe_path, NULL);
                default:
                    monitors[i]->process_id = new_pid;
                    // send subdirs to child...
            }
            return;
        }
    }
}

void releaseResources(CountryMonitor **countries, MonitorInfo **monitors,
                      unsigned int num_monitors, struct dirent **directories, unsigned int num_dirs)
{
    for (unsigned int i = 0; i < num_dirs; i++)
    {
        delete countries[i];
    }
    delete[] countries;

    for (unsigned int i = 0; i < num_monitors; i++)
    {
        delete monitors[i];
    }
    delete[] monitors;

    for (unsigned int i = 0; i < num_dirs; i++)
    {
        free(directories[i]);
    }
    free(directories);
}
