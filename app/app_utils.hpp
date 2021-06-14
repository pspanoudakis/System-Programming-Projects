/**
 * File: app_utils.hpp
 * Contains classes used by the app, routine signatures used by main,
 * as well as several other function signatures used internally (but dependent on app classes)
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef APP_CLASSES_HPP
#define APP_CLASSES_HPP

#include <string>
#include "../include/utils.hpp"

#define CHILD_EXEC_NAME "monitorServer"
#define CHILD_EXEC_PATH "./monitorServer"

//#define SHOW_CYCLIC_BUFFER_MSG          // Comment out this to disable messages regarding cyclic buffer.
#define MAX_BUFFER_SIZE 1000000         // Maximum size for buffers
#define MAX_MONITORS 250
#define MAX_THREADS 1000
#define MAX_ID_DIGITS 5                 // Maximum number of digits allowed in Citizen ID's
#define MAX_BLOOM_SIZE 1000000          // Maximum Bloom Filter size allowed

/* Defined Message Type codes. */
#define TRAVEL_REQUEST 1
#define SEARCH_STATUS 2
#define TRAVEL_REQUEST_ACCEPTED 3
#define TRAVEL_REQUEST_REJECTED 4
#define TRAVEL_REQUEST_INVALID 5
#define CITIZEN_FOUND 6
#define CITIZEN_NOT_FOUND 7
#define MONITOR_EXIT 8

class LinkedList;
class RedBlackTree;
class RBTreeNode;
class SkipList;
class BloomFilter;
class HashTable;

/**
 * Used for Date representation.
 */
class Date
{
    public:
        unsigned short int day;
        unsigned short int month;
        unsigned short int year;

        Date(unsigned short int d = 0, unsigned short int m = 0, unsigned short int y = 0);
        Date(const Date &date);
        void set(unsigned short int d, unsigned short int m, unsigned short int y);
        void set(Date &other);
        bool isNullDate();
        bool isValidDate();
        void setToCurrentDate();
        bool isBetween(const Date &a, const Date &b);
        void set6monthsPrior(const Date &other);
};

class CountryStatus;
/**
 * Used for Citizen Representation.
 */
class CitizenRecord
{
    public:        
        const unsigned int id;
        char *fullname;
        unsigned short int age;
        CountryStatus *country;     // The country of the citizen.
        CitizenRecord(unsigned int citizen_id, char *name, unsigned short int citizen_age, CountryStatus *c);
        ~CitizenRecord();
        bool hasInfo(unsigned int id, char *name, unsigned short int age, char *country_name);
        std::string toString();
};

/**
 * Used for a Vaccination Record represenation.
 */
class VaccinationRecord
{
    public:
        CitizenRecord *citizen;     // The citizen that the record refers to.
        char* virus_name;
        bool vaccinated;            // TRUE if the citizen has been vaccinated, FALSE otherwise
        Date date; 
        VaccinationRecord(CitizenRecord *person, bool is_vaccinated, char *virus, Date d = Date());
        ~VaccinationRecord();
        void vaccinate(Date d);
};

/**
 * Used for storing records for a specific Virus.
 */
class VirusRecords
{
    private:
        SkipList *vaccinated;       // Skip List for vaccinated persons
        SkipList *non_vaccinated;   // Skip List for not vaccinated persons        
    public:
        BloomFilter *filter;        // Bloom Filter for fast citizen lookup
        char *virus_name;           // The name of the Virus.
        VirusRecords(char *name, int skip_list_layers, unsigned long filter_bytes);
        ~VirusRecords();
        bool insertRecordOrShowExisted(VaccinationRecord *record, VaccinationRecord**present, bool &modified,
                                       FILE *fstream);
        bool checkBloomFilter(char *citizenID);
        void getVaccinationStatusString(int citizenID, std::string &msg_str);
        VaccinationRecord *getVaccinationRecord(int citizenID);
};

/**
 * Used for storing information about a specific Virus in a specific country.
 */
class VirusCountryStatus
{
    private:
        void getTotalStatsRec(int &total, Date start,  Date end, RBTreeNode *root);
        void getAgeStatsRec(int &bellow_20, int &between20_40, int &between40_60,
                            int &plus60, Date start, Date end, RBTreeNode *root);
        void getAgeStatsRec(int &bellow_20, int &between20_40, int &between40_60,
                            int &plus60, RBTreeNode *root);
        void updateAgeCounter(int age, int &bellow_20, int &between20_40, 
                              int &between40_60, int &plus60);
        RedBlackTree *record_tree;      // A Red-Black Tree that contains all the Vaccination Records
                                        // of *vaccinated* persons in the country for this Virus.
    public:
        int total_population;           // Number of Records (both YES/NO) associated with this Virus and Citizens of this Country
        int population_bellow_20;       // Number of such Records for each Age Group
        int population_20_40;
        int population_40_60;
        int population_60_plus;
        char *virus_name;               // The name of the virus.
                                        // Note than this is the *exact same* character array address
                                        // stored in VirusRecords for this Virus.
        VirusCountryStatus(char *name, CompareFunc tree_func);
        ~VirusCountryStatus();
        void storeVaccinationRecord(VaccinationRecord *record);
        void updatePopulation(CitizenRecord *citizen);
        void getVaccinationStatsByAge(int &bellow_20, int &between20_40, int &between40_60,
                                      int &plus60, Date start, Date end);
        void getVaccinationStatsByAge(int &bellow_20, int &between20_40, int &between40_60,
                                      int &plus60);                                      
        void getTotalVaccinationStats(int &total, Date start,  Date end);
        void getTotalVaccinationStats(int &total);
};

/**
 * Used for storing information about a specific country.
 */
class CountryStatus
{
    private:
        LinkedList *virus_status;       // A List of statuses about every Virus for which
                                        // a citizen of this country has been vaccinated.
    public:
        char *country_name;             // The name of the country.
        CountryStatus(char *name);
        ~CountryStatus();
        void storeCitizenVaccinationRecord(VaccinationRecord *record);
        void storeNewVaccinationRecord(VaccinationRecord *record);
};

/**
 * Used by child Monitors to store information about assigned country directories.
 */
class DirectoryInfo
{
        unsigned short int numContents;     // Number of files in the directory
    public:
        const char *path;                   // The full path of the directory
        LinkedList *contents;               // The directory contents (a list of file names)
        DirectoryInfo(const char *path);
        ~DirectoryInfo();
        void addContents();
        void updateContents();
};

/**
 * Functions to be used internally by container structures ------------------------------ 
 */

char* copyString(const char *str);
int citizenHashObject(void *citizen);

// Comparison functions take void* arguments
// for internal compatibility purposes.

int compareStrings(void *a, void *b);
int compareDates(const Date &d1, const Date &d2);
int compareDates(void *a, void *b);
int compareIdToVaccinationRecord(void *id, void *record);
int compareIdToCitizen(void *id, void *citizen_record);
int compareCitizens(void *a, void *b);
int compareNameVirusRecord(void *name, void *virus_record);
int compareVaccinationRecordsByCitizen(void *a, void *b);
int compareVaccinationsDateFirst(void *a, void *b);
int compareNameVirusCountryStatus(void *name, void *virus_status);
int compareNameCountryStatus(void *name, void *country_status);

void displayVaccinationCitizen(void *record);

/**
 * Functions used directly by main ----------------------------------------------------------------
 */

void displayMessage(FILE *fstream, const char *format, ...);
void insertVaccinationRecord(unsigned int citizen_id, char *full_name, char *country_name, unsigned short int age,
                             char *virus_name, bool vaccinated, Date &date,
                             LinkedList *countries, LinkedList *viruses, HashTable *citizens,
                             unsigned long bloom_bytes, FILE *fstream);
void vaccineStatus(int citizen_id, LinkedList *viruses, std::string &msg_str);
void vaccineStatus(int citizen_id, LinkedList *viruses, char *virus_name);
void vaccineStatusBloom(int citizen_id, LinkedList *viruses, char *virus_name);

#endif
