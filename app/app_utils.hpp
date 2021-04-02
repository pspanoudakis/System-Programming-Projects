/**
 * File: app_utils.hpp
 * Contains classes used by the app, routine signatures used by main,
 * as well as several other function signatures used internally (but dependent on app classes)
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef APP_CLASSES_HPP
#define APP_CLASSES_HPP

#include "../include/utils.hpp"

class LinkedList;
class RedBlackTree;
class RBTreeNode;
class SkipList;
class BloomFilter;
class HashTable;

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
class CitizenRecord
{
    public:        
        const int id;
        char *fullname;
        int age;
        CountryStatus *country;
        CitizenRecord(int citizen_id, char *name, int citizen_age, CountryStatus *c);
        ~CitizenRecord();
        bool hasInfo(int id, char *name, int age, char *country_name);
};

class VaccinationRecord
{
    public:
        CitizenRecord *citizen;
        char* virus_name;
        bool vaccinated;
        Date date; 
        VaccinationRecord(CitizenRecord *person, bool is_vaccinated, char *virus, Date d = Date());
        ~VaccinationRecord();
        void vaccinate(Date d);
};

class VirusRecords
{
    private:
        SkipList *vaccinated;
        SkipList *non_vaccinated;
        BloomFilter *filter;
    public:
        char *virus_name;        
        VirusRecords(char *name, int skip_list_layers, unsigned long filter_bits);
        ~VirusRecords();
        bool insertRecordOrShowExisted(VaccinationRecord *record, VaccinationRecord**present, bool &modified,
                                       FILE *fstream);
        bool checkBloomFilter(char *citizenID);
        void displayVaccinationStatus(int citizenID);
        void displayWhetherVaccinated(int citizenID);
        void displayNonVaccinated();
};

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
        RedBlackTree *record_tree;
    public:
        char *virus_name;        
        VirusCountryStatus(char *name, CompareFunc tree_func);
        ~VirusCountryStatus();
        void storeVaccinationRecord(VaccinationRecord *record);
        void getVaccinationStatsByAge(int &bellow_20, int &between20_40, int &between40_60,
                                      int &plus60, Date start, Date end);
        void getVaccinationStatsByAge(int &bellow_20, int &between20_40, int &between40_60,
                                      int &plus60);                                      
        void getTotalVaccinationStats(int &total, Date start,  Date end);
        void getTotalVaccinationStats(int &total);
};

class CountryStatus
{
    private:
        int total_population;
        int population_bellow_20;
        int population_20_40;
        int population_40_60;
        int population_60_plus;
        LinkedList *virus_status;
    public:
        char *country_name;
        CountryStatus(char *name);
        ~CountryStatus();
        void storeCitizenVaccinationRecord(VaccinationRecord *record);
        void updatePopulation(CitizenRecord *citizen);
        void displayTotalPopulationStatus(char *virus_name, Date start = Date(),  Date end = Date());
        void displayStatusByAge(char *virus_name, Date start,  Date end);
};

/**
 * "Generic" Functions to be used internally by container structures ------------------------------ 
 */

char* copyString(const char *str);
int citizenHashObject(void *citizen);
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
void insertVaccinationRecord(int citizen_id, char *full_name, char *country_name, int age,
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
