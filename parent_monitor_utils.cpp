#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <sstream>

#include "parent_monitor_utils.hpp"
#include "include/linked_list.hpp"
#include "include/bloom_filter.hpp"
#include "include/rb_tree.hpp"
#include "include/utils.hpp"
#include "app/app_utils.hpp"
#include "include/utils.hpp"
#include "pipe_msg.hpp"

MonitorInfo::MonitorInfo(): process_id(-1), read_pipe_path(NULL), write_pipe_path(NULL),
subdirs(new LinkedList(delete_object_array<char>)) { }

MonitorInfo::~MonitorInfo()
{
    delete[] read_pipe_path;
    delete[] write_pipe_path;
    delete subdirs;
}

CountryMonitor::CountryMonitor(const char *name, MonitorInfo *monitor_info):
country_name(copyString(name)), monitor(monitor_info), requests_tree(new RedBlackTree(compareTravelRequests)) { }

CountryMonitor::~CountryMonitor()
{
    delete[] country_name;
    delete requests_tree;
}

TravelRequest::TravelRequest(Date &request_date, bool is_accepted):
date(request_date), accepted(is_accepted) { }

TravelRequest::~TravelRequest() { }

VirusFilter::VirusFilter(const char *name, unsigned long size): virus_name(copyString(name)), filter(new BloomFilter(size)) { }

VirusFilter::~VirusFilter()
{
    delete[] virus_name;
    delete filter;
}

bool assignMonitorDirectories(const char *path, CountryMonitor **&countries, MonitorInfo **&monitors,
                              unsigned int num_monitors, struct dirent **&directories, unsigned int &num_dirs)
{
    unsigned int i;
    num_dirs = scandir(path, &directories, NULL, alphasort);

    if (num_dirs == -1)
    {
        fprintf(stderr, "Failed to scan directory: %s\n", path);
        return false;
    }

    countries = new CountryMonitor*[num_dirs - 2];
    for (i = 0; i < num_dirs - 2; i++)
    {
        countries[i] = NULL;
    }

    monitors = new MonitorInfo*[num_monitors];
    for (i = 0; i < num_monitors; i++)
    {
        monitors[i] = NULL;
    }

    i = 0;
    for (unsigned int j = 2; j < num_dirs; j++)
    {
        if (strcmp(directories[j]->d_name, ".") == 0 || strcmp(directories[j]->d_name, "..") == 0)
        {
            continue;
        }
        if (monitors[i] == NULL)
        {
            std::stringstream read_name_stream;
            std::stringstream write_name_stream;
            monitors[i] = new MonitorInfo();
            read_name_stream << "./fifo_pipes/read" << i;
            write_name_stream << "./fifo_pipes/write" << i; 
            monitors[i]->read_pipe_path = copyString(read_name_stream.str().c_str());
            monitors[i]->write_pipe_path = copyString(write_name_stream.str().c_str());

            if (mkfifo(monitors[i]->write_pipe_path, 0666) < 0 )
            {
                fprintf(stderr, "Error creating fifo: %s\n", monitors[i]->write_pipe_path);
                return false;
            }
            if (mkfifo(monitors[i]->read_pipe_path, 0666) < 0 )
            {
                fprintf(stderr, "Error creating fifo: %s\n", monitors[i]->read_pipe_path);
                return false;
            }
        }
        std::string directory_path(path);
        directory_path.append("/");
        directory_path.append(directories[j]->d_name);
        monitors[i]->subdirs->append(copyString(directory_path.c_str()));
        countries[j - 2] = new CountryMonitor(directories[j]->d_name, monitors[i]);
        i++;
        i = i % num_monitors;
    }

    return true;
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
                perror("Failed to create child process\n");
                exit(EXIT_FAILURE);
                break;
            case 0:
                /* child */
                execl("./monitor", "monitor", monitors[i]->write_pipe_path, monitors[i]->read_pipe_path, NULL);
                _Exit(EXIT_FAILURE);
            default:
                monitors[i]->process_id = pid;
                // send subdirs to child...
        }
    }
}

void restoreChild(MonitorInfo *monitor)
{
    unlink(monitor->write_pipe_path);
    unlink(monitor->read_pipe_path);
    if (mkfifo(monitor->write_pipe_path, 0666) < 0 )
    {
        perror("Error creating fifo");
        return;
    }
    if (mkfifo(monitor->read_pipe_path, 0666) < 0 )
    {
        perror("Error creating fifo");
        return;
    }
    int new_pid = fork();
    switch (new_pid)
    {
        case -1:
            /* problem */
            break;
        case 0:
            /* child */
            execl("./monitor", "monitor", monitor->write_pipe_path, monitor->read_pipe_path, NULL);
            _Exit(EXIT_FAILURE);
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

void sendMonitorData(MonitorInfo **monitors, unsigned int &num_monitors, char *buffer, unsigned int buffer_size,
                     unsigned long int bloom_size)
{
    int fd;
    for(unsigned int i = 0; (i < num_monitors && monitors[i] != NULL); i++)
    {
        fd = open(monitors[i]->write_pipe_path, O_WRONLY);
        if (fd < 0)
        {
            fprintf(stderr, "Failed to open pipe: %s\n", monitors[i]->write_pipe_path);
        }
        sendInt(fd, buffer_size, buffer, buffer_size);
        sendLongInt(fd, bloom_size, buffer, buffer_size);
        sendInt(fd, monitors[i]->subdirs->getNumElements(), buffer, buffer_size);
        for (LinkedList::ListIterator itr = monitors[i]->subdirs->listHead(); !itr.isNull(); itr.forward())
        {
            sendString(fd, static_cast<char*>(itr.getData()), buffer, buffer_size);
        }
        close(fd);
    }
}

void receiveMonitorFilters(MonitorInfo **monitors, unsigned int &num_monitors, LinkedList *viruses)
{

}

void terminateChildren(MonitorInfo **monitors, unsigned int &num_monitors)
{
    for(unsigned int i = 0; (i < num_monitors && monitors[i] != NULL); i++)
    {
        kill(monitors[i]->process_id, SIGKILL);
        waitpid(monitors[i]->process_id, NULL, 0);
        unlink(monitors[i]->read_pipe_path);
        unlink(monitors[i]->write_pipe_path);
    }
}

void releaseResources(CountryMonitor **countries, MonitorInfo **monitors,
                      unsigned int num_monitors, struct dirent **directories, unsigned int num_dirs)
{
    unsigned int i;
    for (i = 0; i < num_dirs - 2; i++)
    {
        delete countries[i];
    }
    delete[] countries;

    for (i = 0; i < num_monitors; i++)
    {
        delete monitors[i];
    }
    delete[] monitors;

    for (i = 0; i < num_dirs; i++)
    {
        free(directories[i]);
    }
    free(directories);
}


int compareTravelRequests(void *r1, void *r2)
{
    TravelRequest *req1 = static_cast<TravelRequest*>(r1);
    TravelRequest *req2 = static_cast<TravelRequest*>(r2);

    int comp = compareDates(req1->date, req2->date);
    if (comp != 0) { return comp; }

    // return either -1 or 1
    return -1 + (rand() % 2) * 2;
}