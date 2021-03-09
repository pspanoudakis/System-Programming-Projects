#include <cstring>
#include "app_utils.hpp"

char* copyString(const char *str)
{
    int len = strlen(str);
    char *copy = new char [len];
    for (int i = 0; i <= len; i++)
    {
        copy[i] = str[i];
    }
    return copy;
}

Date::Date(int d, int m, int y):
day(d), month(m), year(y) { }

void Date::set(int d, int m, int y)
{
    this->day = d;
    this->month = m;
    this->year = y;
}

int compareDates(void *a, void *b)
{
    Date *d1 = (Date*)a;
    Date *d2 = (Date*)b;

    if (d1->year != d2->year)
    {
        return d1->year - d2->year;
    }
    if (d1->month != d2->month)
    {
        return d1->month - d2->month;
    }
    if (d1->day != d2->day)
    {
        return d1->day - d2->day;
    }
    return 0;
}

VirusRecords::VirusRecords(char *name, int skip_list_layers, DestroyFunc dest):
virus_name(name),
vaccinated(new SkipList(skip_list_layers, dest)),
non_vaccinated(new SkipList(skip_list_layers, dest)) { }

VirusRecords::~VirusRecords()
{
    delete non_vaccinated;
    delete vaccinated;
    delete virus_name;
}

VirusCountryStatus::VirusCountryStatus(char *name, CompareFunc tree_func):
virus_name(name), record_tree(new RedBlackTree(tree_func)) { }

VirusCountryStatus::~VirusCountryStatus()
{
    delete record_tree;
}

int compareVirusCountryStatus(void *a, void *b)
{

}

void destroyVirusCountryStatus(void *status)
{
    delete status;
}

CountryStatus::CountryStatus(char *name, CompareFunc comp, DestroyFunc dest):
country_name(name), population(0), virus_status(new LinkedList(comp, dest)) { }

CountryStatus::~CountryStatus()
{
    delete [] country_name;
    delete virus_status;
}

CitizenRecord::CitizenRecord(int citizen_id, char *name, int citizen_age, char *country_name):
id(citizen_id), fullname(name), age(citizen_age), country(country_name) { }

CitizenRecord::~CitizenRecord()
{
    delete [] fullname;
}

VaccinationRecord::VaccinationRecord(CitizenRecord *person, bool is_vaccinated, char *virus, Date d = Date()):
citizen(person), vaccinated(is_vaccinated), virus_name(virus), date(d) { }

VaccinationRecord::~VaccinationRecord() { }