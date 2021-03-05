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

CitizenRecord::CitizenRecord(int citizenID, const char *name, int citizenAge, Country *citizenCountry):
id(citizenID), fullname(copyString(name)), age(citizenAge), country(citizenCountry) { }

CitizenRecord::~CitizenRecord()
{
    delete [] fullname;
}

VaccinationRecord::VaccinationRecord(CitizenRecord *person, Virus v, Date d):
citizen(person), virus(v), date(d) { }

VaccinationRecord::~VaccinationRecord() { }