#ifndef PARENT_MONITOR_UTILS_HPP
#define PARENT_MONITOR_UTILS_HPP

class LinkedList;

class MonitorInfo
{
    private:
        /* data */
    public:
        int process_id;
        const char *write_pipe_path;
        const char *read_pipe_path;
        LinkedList *subdirs;
        MonitorInfo();
        ~MonitorInfo();
};

class CountryMonitor
{
    private:
        /* data */
    public:
        const char *country_name;
        MonitorInfo *monitor;
        CountryMonitor(const char *name, MonitorInfo *monitor_info);
        ~CountryMonitor();
};

void assignMonitorDirectories(const char *path, CountryMonitor **&countries, MonitorInfo **&monitors, unsigned int num_monitors,
                              struct dirent **&files, unsigned int &num_files);
void createMonitors(MonitorInfo **monitors, unsigned int &num_monitors);
void restoreChild(MonitorInfo *monitor);
void restoreChild(int pid, MonitorInfo **monitors, unsigned int num_monitors);
void checkAndRestoreChildren(MonitorInfo **monitors, unsigned int num_monitors);
void releaseResources(CountryMonitor **countries, MonitorInfo **monitors,
                      unsigned int num_monitors, struct dirent **directories, unsigned int num_dirs);

#endif
