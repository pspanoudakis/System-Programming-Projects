#include <cstring>
#include "app_utils.hpp"
#include <cstdio>

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

bool Date::isValidDate()
{
    return ( (day >= 1) && (day <= 30) && 
             (month >= 1) && (month <= 12) &&
             (year >= 1990) && (year <= 2100));
}

bool Date::isNullDate()
{
    return (day == 0) && (month == 0) && (year == 0);
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

CitizenRecord::CitizenRecord(int citizen_id, char *name, int citizen_age, CountryStatus *c):
id(citizen_id), fullname(name), age(citizen_age), country(c) { }

CitizenRecord::~CitizenRecord()
{
    delete [] fullname;
}

/**
 * Compares the given ID with the citizen ID of the specified Vaccination Record.
 * @return 0 if the ID's are equal, a positive number if
 * ID > citizen ID, or a negative number if ID < citizen ID.
 */
int compareIdToVaccinationRecord(void *id, void *record)
{
    int target_id = *(int*)id;
    VaccinationRecord *vaccination_record = (VaccinationRecord*)record;
    return (target_id - vaccination_record->citizen->id);
}

int compareCitizens(void *a, void *b)
{
    CitizenRecord *c1 = (CitizenRecord*)a;
    CitizenRecord *c2 = (CitizenRecord*)b;

    if (c1->id != c2->id)
    {
        return c1->id - c2->id;
    }
    int cmp = strcmp(c1->fullname, c2->fullname);
    if (cmp != 0)
    {
        return cmp;
    }
    cmp = strcmp(c1->country->country_name, c2->country->country_name);
    if (cmp != 0)
    {
        return cmp;
    }
    if (c1->age != c2->age)
    {
        return c1->age - c2->age;
    }
    return 0;
}

int compareVaccinationRecords(void *a, void *b)
{
    return compareCitizens( ((VaccinationRecord*)a)->citizen, ((VaccinationRecord*)b)->citizen);
}

VaccinationRecord::VaccinationRecord(CitizenRecord *person, bool is_vaccinated, char *virus, Date d = Date()):
citizen(person), vaccinated(is_vaccinated), virus_name(virus), date(d) { }

VaccinationRecord::~VaccinationRecord() { }

/**
 * Displays the Citizen Information of the given Vaccination Record.
 * @param record A pointer to a VaccinationRecord (void* for internal compatibility purposes)
 */
void displayVaccinationCitizen(void *record)
{
    CitizenRecord *citizen = ((VaccinationRecord*)record)->citizen;
    printf("%d %s %s %d\n", citizen->id, citizen->fullname, citizen->country->country_name, citizen->age);
}

VirusCountryStatus::VirusCountryStatus(char *name, CompareFunc tree_func):
virus_name(name), record_tree(new RedBlackTree(tree_func)) { }

VirusCountryStatus::~VirusCountryStatus()
{
    delete record_tree;
}

/**
 * Virus Records Methods-Functions -----------------------------------------------------------------
 */

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

void VirusRecords::displayVaccinationStatus(int citizenID)
{
    VaccinationRecord *record = (VaccinationRecord*)this->vaccinated->find(&citizenID, compareIdToVaccinationRecord);
    if (record == NULL)
    {
        printf("%s NO\n", this->virus_name);
    }
    else
    {
        printf("%s YES %d-%d-%d\n", this->virus_name, record->date.day, record->date.month, record->date.year);   
    }
}

void VirusRecords::displayWhetherVaccinated(int citizenID)
{
    VaccinationRecord *record = (VaccinationRecord*)this->vaccinated->find(&citizenID, compareIdToVaccinationRecord);
    if (record == NULL)
    {
        printf("NOT VACCINATED\n");
    }
    else
    {
        printf("VACCINATED ON %d-%d-%d\n", record->date.day, record->date.month, record->date.year);   
    }    
}

void VirusRecords::displayNonVaccinated()
{
    this->non_vaccinated->displayElements(displayVaccinationCitizen);
}

bool VirusRecords::checkBloomFilter(char *citizen_id)
{
    return this->filter->isPresent(citizen_id);
}

void VirusRecords::insertRecordOrShowExisted(VaccinationRecord *record)
{
    VaccinationRecord *present;
    if (record->vaccinated)
    {
        if ( this->vaccinated->insert(record, (void**)&present, compareVaccinationRecords) )
        {
            printf("SUCCESSFULLY VACCINATED\n");
        }
        else
        {            
            printf("ERROR: CITIZEN %d ALREADY VACCINATED ON %d-%d-%d\n", 
                    present->citizen->id, present->date.day, present->date.month, present->date.year);
        }
    }
    else
    {
        if ( this->non_vaccinated->insert(record, (void**)present, compareVaccinationRecords) )
        {
            printf("SUCCESSFULLY MARKED AS NON VACCINATED\n");
        }
        else
        {
            printf("ERROR: ALREADY MARKED AS NON VACCINATED\n");
        }
    }
    
}

int compareVirusCountryStatus(void *a, void *b)
{

}

void destroyVirusCountryStatus(void *status)
{
    delete status;
}

CountryStatus::CountryStatus(char *name, CompareFunc comp, DestroyFunc dest):
country_name(name), total_population(0), population_20_40(0), population_40_60(0),
population_60_plus(0), virus_status(new LinkedList(comp, dest)) { }

CountryStatus::~CountryStatus()
{
    delete [] country_name;
    delete virus_status;
}
