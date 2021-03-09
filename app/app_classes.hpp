#ifndef APP_CLASSES_HPP
#define APP_CLASSES_HPP

#include "../include/linked_list.hpp"
#include "../include/rb_tree.hpp"
#include "../include/skip_list.hpp"
#include "../include/bloom_filter.hpp"

struct Date
{
    int day;
    int month;
    int year;

    Date(int d = 0, int m = 0, int y = 0);
    void set(int d, int m, int y);
};

class VirusRecords
{
    public:
        char *virus_name;
        SkipList *vaccinated;
        SkipList *non_vaccinated;
        BloomFilter *filter;
        VirusRecords(const char *name, int skip_list_layers, DestroyFunc dest);
        ~VirusRecords();
};

class VirusCountryStatus
{
    public:
        char *virus_name;
        RedBlackTree *record_tree;
        VirusCountryStatus(char *name, CompareFunc tree_func);
        ~VirusCountryStatus();
};

class CountryStatus
{
    public:
        char *country_name;
        int population;
        LinkedList *virus_status;
        CountryStatus();
        ~CountryStatus();
};

class CitizenRecord
{
    private:
        const int id;
        char *fullname;
        int age;
        char *country;
    public:
        CitizenRecord(int citizen_id, const char *name, int citizen_age, char *country_name);
        ~CitizenRecord();
};

class VaccinationRecord
{
    private:
        CitizenRecord *citizen;
        char* virus_name;
        bool vaccinated;
        Date date;        
    public:
        VaccinationRecord(CitizenRecord *person, bool is_vaccinated, char *virus, Date d = Date());
        ~VaccinationRecord();
};

#endif
