#include <cstring>
#include "app_classes.hpp"

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

CitizenRecord::CitizenRecord(int citizenID, const char *name, int citizenAge, Country *citizenCountry):
id(citizenID), fullname(copyString(name)), age(citizenAge), country(citizenCountry) { }

CitizenRecord::~CitizenRecord()
{
    delete [] fullname;
}

VaccinationRecord::VaccinationRecord(CitizenRecord *person, bool is_vaccinated, Virus v, Date d = Date()):
citizen(person), vaccinated(is_vaccinated), virus(v), date(d) { }

VaccinationRecord::~VaccinationRecord() { }