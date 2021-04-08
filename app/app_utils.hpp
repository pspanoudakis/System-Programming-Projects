/**
 * File: app_utils.hpp
 * Contains classes used by the app, routine signatures used by main,
 * as well as several other function signatures used internally (but dependent on app classes)
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef APP_CLASSES_HPP
#define APP_CLASSES_HPP

#include "../include/utils.hpp"

#define MAX_ID_DIGITS 5     // Maximum number of digits allowed in Citizen ID's

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
        BloomFilter *filter;        // Bloom Filter for fast citizen lookup
    public:
        char *virus_name;           // The name of the Virus.
        VirusRecords(char *name, int skip_list_layers, unsigned long filter_bytes);
        ~VirusRecords();
        bool insertRecordOrShowExisted(VaccinationRecord *record, VaccinationRecord**present, bool &modified,
                                       FILE *fstream);
        bool checkBloomFilter(char *citizenID);
        void displayVaccinationStatus(int citizenID);
        void displayWhetherVaccinated(int citizenID);
        void displayNonVaccinated();
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
        int total_population;           // Number of Records associated with this Virus and Citizens of this Country
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
        void updateVirusStatusPopulation(CitizenRecord *citizen, char *virus_name);
        void displayTotalPopulationStatus(char *virus_name, Date start = Date(),  Date end = Date());
        void displayStatusByAge(char *virus_name, Date start,  Date end);
};

/**
 * Functions to be used internally by container structures ------------------------------ 
 */

char* copyString(const char *str);
int citizenHashObject(void *citizen);

// Comparison functions take void* arguments
// for internal compatibility purposes.

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
void vaccineStatus(int citizen_id, LinkedList *viruses);
void vaccineStatus(int citizen_id, LinkedList *viruses, char *virus_name);
void vaccineStatusBloom(int citizen_id, LinkedList *viruses, char *virus_name);
void listNonVaccinatedPersons(char *virus_name, LinkedList *viruses);
void populationStatus(char *virus_name, LinkedList *countries, Date start=Date(), Date end=Date());
void populationStatus(char *virus_name, char *country_name, LinkedList *countries, Date start=Date(), Date end=Date());
void popStatusByAge(char *virus_name, LinkedList *countries, Date start=Date(), Date end=Date());
void popStatusByAge(char *virus_name, char *country_name, LinkedList *countries, Date start=Date(), Date end=Date());

#endif
