#ifndef APP_CLASSES_HPP
#define APP_CLASSES_HPP

#include "../include/utils.hpp"

class LinkedList;
class RedBlackTree;
class RBTreeNode;
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
        bool insertRecordOrShowExisted(VaccinationRecord *record);
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
        void displayTotalPopulationStatus(char *virus_name, Date start,  Date end);
        void displayStatusByAge(char *virus_name, Date start,  Date end);
};

int compareDates(void *a, void *b);
int compareIdToVaccinationRecord(void *id, void *record);
int compareIdToCitizen(void *id, void *citizen_record);
int compareCitizens(void *a, void *b);
int compareNameVirusRecord(void *name, void *virus_record);
int compareVaccinationRecordsByCitizen(void *a, void *b);
int compareVaccinationsDateFirst(void *a, void *b);
int compareNameVirusCountryStatus(void *name, void *virus_status);
int compareVirusNames(void *a, void *b);
int compareNameCountryStatus(void *name, void *country_status);

//void destroyVaccinationRecord(void *record);
//void destroyVirusCountryStatus(void *status);

void displayVaccinationCitizen(void *record);

#endif
