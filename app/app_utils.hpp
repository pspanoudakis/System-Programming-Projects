#ifndef APP_CLASSES_HPP
#define APP_CLASSES_HPP

#include "../include/utils.hpp"

class LinkedList;
class RedBlackTree;
class SkipList;
class BloomFilter;

struct Date
{
    int day;
    int month;
    int year;

    Date(int d = 0, int m = 0, int y = 0);
    void set(int d, int m, int y);
    bool isNullDate();
    bool isValidDate();
};

class CitizenRecord
{
    public:
        const int id;
        char *fullname;
        int age;
        CountryStatus *country;
        CitizenRecord(int citizen_id, char *name, int citizen_age, CountryStatus *c);
        ~CitizenRecord();
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
};

class VirusRecords
{
    public:
        char *virus_name;
        SkipList *vaccinated;
        SkipList *non_vaccinated;
        BloomFilter *filter;
        VirusRecords(char *name, int skip_list_layers, DestroyFunc dest);
        ~VirusRecords();
        void insertRecordOrShowExisted(VaccinationRecord *record);
        bool checkBloomFilter(char *citizenID);
        void displayVaccinationStatus(int citizenID);
        void displayWhetherVaccinated(int citizenID);
        void displayNonVaccinated();
};

class VirusCountryStatus
{
    public:
        char *virus_name;
        RedBlackTree *record_tree;
        VirusCountryStatus(char *name, CompareFunc tree_func);
        ~VirusCountryStatus();
        void storeVaccinationRecord(VaccinationRecord *record);
        void getVaccinationStatsByAge(int &bellow_20, int &between20_40, int &between40_60,
                                      int plus60, Date start, Date end);
        void getVaccinationStatsByAge(int &bellow_20, int &between20_40, int &between40_60,
                                      int &plus60);                                      
        void getTotalVaccinationStats(int &total, Date start,  Date end);
        void getTotalVaccinationStats(int &total);
};

class CountryStatus
{
    public:
        char *country_name;
        int total_population;
        int population_bellow_20;
        int population_20_40;
        int population_40_60;
        int population_60_plus;

        LinkedList *virus_status;
        CountryStatus(char *name, CompareFunc comp, DestroyFunc dest);
        ~CountryStatus();
        void storeCitizenVaccinationRecord(VaccinationRecord *record);
        void updatePopulation(VaccinationRecord *record);
        void displayTotalPopulationStatus(char *virus_name, Date start,  Date end);
        void displayStatusByAge(char *virus_name, Date start,  Date end);
};

#endif
