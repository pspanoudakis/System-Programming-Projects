/**
 * File: app_utils.cpp
 * Contains implementations for class methods, routines to be called by main
 * and several other functions used internally (but dependent on app classes)
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstring>
#include <cstdio>
#include <ctime>
#include <cstdlib>

#include "app_utils.hpp"
#include "../include/linked_list.hpp"
#include "../include/rb_tree.hpp"
#include "../include/skip_list.hpp"
#include "../include/bloom_filter.hpp"
#include "../include/hash_table.hpp"

char* copyString(const char *str)
{
    int len = strlen(str);
    char *copy = new char[len + 1];
    for (int i = 0; i < len; i++)
    {
        copy[i] = str[i];
    }
    copy[len] = '\0';
    return copy;
}

/**
 * Date class functions ---------------------------------------------------------------------------
 */

Date::Date(unsigned short int d, unsigned short int m, unsigned short int y):
day(d), month(m), year(y) { }

Date::Date(const Date &date):
day(date.day), month(date.month), year(date.year) { }

void Date::set(unsigned short int d, unsigned short int m, unsigned short int y)
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

void Date::setToCurrentDate()
{
    Date current = currentDate();
    this->set(current.day, current.month, current.year);
}

void Date::set(Date &other)
{
    this->day = other.day;
    this->month = other.day;
    this->year = other.year;
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

Date currentDate()
{
    time_t timer = time(NULL);
    struct tm time_info;
	time_info = *localtime(&timer);
    return Date(time_info.tm_mday, time_info.tm_mon + 1, time_info.tm_year + 1900);
}

/**
 * Citizen Record functions -----------------------------------------------------------------------
 */

CitizenRecord::CitizenRecord(int citizen_id, char *name, int citizen_age, CountryStatus *c):
id(citizen_id), fullname(copyString(name)), age(citizen_age), country(c) { }

CitizenRecord::~CitizenRecord()
{
    delete [] fullname;
}

bool CitizenRecord::hasInfo(int citizen_id, char *citizen_name, int citizen_age, char *country_name)
{
    if (this->id != citizen_id)
    {
        return false;
    }
    if (strcmp(this->fullname, citizen_name) != 0)
    {
        return false;
    }
    if (this->age != citizen_age)
    {
        return false;
    }
    if (strcmp(this->country->country_name, country_name) != 0)
    {
        return false;
    }
    return true;
}

int compareCitizens(void *a, void *b)
{
    CitizenRecord *c1 = static_cast<CitizenRecord*>(a);
    CitizenRecord *c2 = static_cast<CitizenRecord*>(b);

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

int compareIdToCitizen(void *id, void *citizen_record)
{
    return *(static_cast<int*>(id)) - (static_cast<CitizenRecord*>(citizen_record)->id);
}

void displayCitizen(void *record)
{
    CitizenRecord *citizen = static_cast<CitizenRecord*>(record);
    printf("%d %s %s %d\n", citizen->id, citizen->fullname, citizen->country->country_name, citizen->age);
}

int citizenHashObject(void *citizen)
{
    return static_cast<CitizenRecord*>(citizen)->id;
}

/**
 * Vaccination Record functions -------------------------------------------------------------------
 */

VaccinationRecord::VaccinationRecord(CitizenRecord *person, bool is_vaccinated, char *virus, Date d):
citizen(person), virus_name(virus), vaccinated(is_vaccinated), date(d) { }

VaccinationRecord::~VaccinationRecord() { }

void VaccinationRecord::vaccinate(Date d)
{
    this->vaccinated = true;
    this->date.set(d.day, d.month, d.year);
}

int compareVaccinationRecordsByCitizen(void *a, void *b)
{
    CitizenRecord *citizen1 = static_cast<VaccinationRecord*>(a)->citizen;
    CitizenRecord *citizen2 = static_cast<VaccinationRecord*>(b)->citizen;
    return compareCitizens(citizen1, citizen2);
}

int compareVaccinationsDateFirst(void *a, void *b)
{
    VaccinationRecord *rec1 = static_cast<VaccinationRecord*>(a);
    VaccinationRecord *rec2 = static_cast<VaccinationRecord*>(b);

    int cmp = compareDates(&(rec1->date), &(rec2->date));

    if (cmp != 0)
    {
        return cmp;
    }

    cmp = strcmp(rec1->virus_name, rec2->virus_name);
    if (cmp != 0)
    {
        return cmp;
    }

    return compareCitizens(rec1->citizen, rec2->citizen);
}

/**
 * Compares the given ID with the citizen ID of the specified Vaccination Record.
 * @return 0 if the ID's are equal, a positive number if
 * ID > citizen ID, or a negative number if ID < citizen ID.
 */
int compareIdToVaccinationRecord(void *id, void *record)
{
    int target_id = *(int*)id;
    VaccinationRecord *vaccination_record = static_cast<VaccinationRecord*>(record);
    return (target_id - vaccination_record->citizen->id);
}

/*
void destroyVaccinationRecord(void *record)
{
    delete (VaccinationRecord*)record;
}
*/
/**
 * Displays the Citizen Information of the given Vaccination Record.
 * @param record A pointer to a VaccinationRecord (void* for internal compatibility purposes)
 */
void displayVaccinationCitizen(void *record)
{
    CitizenRecord *citizen = static_cast<VaccinationRecord*>(record)->citizen;
    printf("%d %s %s %d\n", citizen->id, citizen->fullname, citizen->country->country_name, citizen->age);
}

/**
 * Virus Records Methods-Functions ----------------------------------------------------------------
 */

VirusRecords::VirusRecords(char *name, int skip_list_layers, unsigned long filter_bits):
vaccinated(new SkipList(skip_list_layers, delete_object<VaccinationRecord>)),
non_vaccinated(new SkipList(skip_list_layers, delete_object<VaccinationRecord>)),
filter(new BloomFilter(filter_bits)), virus_name(copyString(name)) { }

VirusRecords::~VirusRecords()
{
    delete non_vaccinated;
    delete vaccinated;
    delete [] virus_name;
    delete filter;
}

void VirusRecords::displayVaccinationStatus(int citizenID)
{
    VaccinationRecord *record;
    record =  static_cast<VaccinationRecord*>(this->vaccinated->find(&citizenID, compareIdToVaccinationRecord));
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
    VaccinationRecord *record;
    record =  static_cast<VaccinationRecord*>(this->vaccinated->find(&citizenID, compareIdToVaccinationRecord));
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

bool VirusRecords::insertRecordOrShowExisted(VaccinationRecord *record, VaccinationRecord**present, bool &modified,
                                            FILE *fstream)
{
    VaccinationRecord *temp;
    char char_id[5]; // max 4 digits + \0
    if (record->vaccinated)
    {
        // If there is already a Non-Vaccinated record for this citizen,
        // remove it from the non-vaccinated skip list
        this->non_vaccinated->remove(record, (void**)present, compareVaccinationRecordsByCitizen);
        if (*present != NULL)
        {
            // Mark as vaccinated
            (*present)->vaccinate(record->date);
            // Insert to vaccinated skip list
            this->vaccinated->insert(*present, (void**)&temp, compareVaccinationRecordsByCitizen);
            // Inserting in bloom filter as well
            sprintf(char_id, "%d", (*present)->citizen->id);
            this->filter->markAsPresent(char_id);

            // TODO: fprintf (for dev/null) maybe?
            fprintf(fstream, "SUCCESSFULLY VACCINATED\n");
            // Indicate that the existing record was previously marked
            // as "non vaccinated" and has changed
            modified = true;
            // Return false to indicate that the given record must be deleted
            return false;
        }
        // Now try to insert to vaccinated skip list
        if ( this->vaccinated->insert(record, (void**)present, compareVaccinationRecordsByCitizen) )
        // The insertion was successful
        {
            // Inserting in bloom filter as well
            sprintf(char_id, "%d", record->citizen->id);
            this->filter->markAsPresent(char_id);

            // TODO: fprintf (for dev/null) maybe?
            fprintf(fstream, "SUCCESSFULLY VACCINATED\n");
            return true;
        }
        else
        // The insertion failed because the citizen has already been vaccinated
        {   
            // TODO: fprintf (for dev/null) maybe?         
            fprintf(fstream, "ERROR: CITIZEN %d ALREADY VACCINATED ON %d-%d-%d\n", 
                    (*present)->citizen->id, (*present)->date.day, (*present)->date.month, (*present)->date.year);
            return false;
        }
    }
    else
    {
        // Try to find if the citizen has actually been vaccinated
        *present = static_cast<VaccinationRecord*>(this->vaccinated->find(record, compareVaccinationRecordsByCitizen));
        if (*present != NULL)
        // If so, inform the user
        {
            // TODO: fprintf (for dev/null) maybe?
            fprintf(fstream, "ERROR: CITIZEN %d ALREADY VACCINATED ON %d-%d-%d\n", 
                    (*present)->citizen->id, (*present)->date.day, (*present)->date.month, (*present)->date.year);
            // Return false to indicate that the given record must be deleted
            return false;
        }
        // Now try to insert to non-vaccinated skip list
        if ( this->non_vaccinated->insert(record, (void**)present, compareVaccinationRecordsByCitizen) )
        {
            // TODO: fprintf (for dev/null) maybe?
            fprintf(fstream, "SUCCESSFULLY MARKED AS NON VACCINATED\n");
            return true;
        }
        else
        {
            // TODO: fprintf (for dev/null) maybe?
            fprintf(fstream, "ERROR: ALREADY MARKED AS NON VACCINATED\n");
            return false;
        }
    }    
}

int compareNameVirusRecord(void *name, void *virus_record)
{
    char *target_name = static_cast<char*>(name);
    VirusRecords *target_record = static_cast<VirusRecords*>(virus_record);
    return strcmp(target_name, target_record->virus_name);
}

/**
 * Virus Country Status functions -----------------------------------------------------------------
 */

VirusCountryStatus::VirusCountryStatus(char *name, CompareFunc tree_func):
record_tree(new RedBlackTree(tree_func)), virus_name(name) { }

VirusCountryStatus::~VirusCountryStatus()
{
    delete record_tree;
}

void VirusCountryStatus::storeVaccinationRecord(VaccinationRecord *record)
{
    this->record_tree->insert(record);
}

void VirusCountryStatus::getTotalStatsRec(int &total, Date start, Date end, RBTreeNode *root)
{
    if (root == NULL) { return; }
    VaccinationRecord *root_data = static_cast<VaccinationRecord*>(root->data);

    if (compareDates(&root_data->date, &start) >= 0)
    {
        getTotalStatsRec(total, start, end, root->left);
        if (compareDates(&root_data->date, &end) <= 0)
        {
            total++;    // root is in range
            getTotalStatsRec(total, start, end, root->right);
            return;
        }
    }
    // root is smaller than start, so just search to the right
    if (compareDates(&root_data->date, &end) <= 0)
    {
        getTotalStatsRec(total, start, end, root->right);
    }
}

void VirusCountryStatus::getAgeStatsRec(int &bellow_20, int &between20_40, int &between40_60,
                                        int &plus60, Date start, Date end, RBTreeNode *root)
{
    if (root == NULL) { return; }
    VaccinationRecord *root_data = static_cast<VaccinationRecord*>(root->data);

    if (compareDates(&root_data->date, &start) >= 0)
    {
        getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, start, end, root->left);
        if (compareDates(&root_data->date, &end) <= 0)
        {
            updateAgeCounter(root_data->citizen->age, bellow_20, between20_40, between40_60, plus60);
            getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, start, end, root->right);
            return;
        }
    }
    // root is smaller than start, so just search to the right
    getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, start, end, root->right);
}

void VirusCountryStatus::getAgeStatsRec(int &bellow_20, int &between20_40, int &between40_60,
                                        int &plus60, RBTreeNode *root)
{
    if (root == NULL) { return; }
    VaccinationRecord *root_data = static_cast<VaccinationRecord*>(root->data);

    updateAgeCounter(root_data->citizen->age, bellow_20, between20_40, between40_60, plus60);
    getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, root->left);
    getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, root->right);
}

void VirusCountryStatus::getTotalVaccinationStats(int &total, Date start, Date end)
{
    this->getTotalStatsRec(total, start, end, this->record_tree->root);
}

void VirusCountryStatus::getTotalVaccinationStats(int &total)
{
    total = this->record_tree->getNumElements();
}

void VirusCountryStatus::getVaccinationStatsByAge( int &bellow_20, int &between20_40, int &between40_60,
                                                   int &plus60, Date start, Date end)
{
    this->getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, start, end, this->record_tree->root);
}

void VirusCountryStatus::getVaccinationStatsByAge( int &bellow_20, int &between20_40, int &between40_60,
                                                   int &plus60)
{
    this->getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, this->record_tree->root);
}

void VirusCountryStatus::updateAgeCounter(int age, int &bellow_20, int &between20_40, 
                                          int &between40_60, int &plus60)
{
    if (age < 20)
    {
        bellow_20++;
    }
    else if (age < 40)
    {
        between20_40++;
    }
    else if (age < 60)
    {
        between40_60++;
    }
    else
    {
        plus60++;
    }
}

/**
 * Country Status functions -----------------------------------------------------------------------
 */

CountryStatus::CountryStatus(char *name):
total_population(0), population_bellow_20(0), population_20_40(0), population_40_60(0), population_60_plus(0),
virus_status(new LinkedList(delete_object<VirusCountryStatus>)),
country_name(copyString(name)) { }

CountryStatus::~CountryStatus()
{
    delete [] country_name;
    delete virus_status;
}

void CountryStatus::storeCitizenVaccinationRecord(VaccinationRecord *record)
{
    VirusCountryStatus *virus_tree;
    virus_tree = static_cast<VirusCountryStatus*>(this->virus_status->getElement(record->virus_name, compareNameVirusCountryStatus));
    if (virus_tree == NULL)
    {
        this->virus_status->append(new VirusCountryStatus(record->virus_name, compareVaccinationsDateFirst));
        virus_tree = static_cast<VirusCountryStatus*>(this->virus_status->getLast());
    }
    virus_tree->storeVaccinationRecord(record);   
}

void CountryStatus::updatePopulation(CitizenRecord *citizen)
{
    this->total_population++;
    if (citizen->age < 20)
    {
        this->population_bellow_20++;
    }
    else if(citizen->age < 40)
    {
        this->population_20_40++;
    }
    else if(citizen->age < 60)
    {
        this->population_40_60++;
    }
    else
    {
        this->population_60_plus++;
    }
}

void CountryStatus::displayStatusByAge(char *virus_name, Date start, Date end)
{
    int bellow_20 = 0;
    int between_20_40 = 0;
    int between_40_60 = 0;
    int plus_60 = 0;

    VirusCountryStatus *virus_tree;
    virus_tree = static_cast<VirusCountryStatus*>(this->virus_status->getElement(virus_name, compareNameVirusCountryStatus));

    if (virus_tree != NULL)
    {
        if (start.isNullDate())
        {
            virus_tree->getVaccinationStatsByAge(bellow_20, between_20_40, between_40_60, plus_60);
        }
        else
        {
            virus_tree->getVaccinationStatsByAge(bellow_20, between_20_40, between_40_60, plus_60, start, end);
        }        
    }

    printf("%s\n", this->country_name);
    if (this->population_bellow_20 == 0)
    {
        printf("0-20 %d %.2f%%\n", bellow_20, (float)0);
    }
    else
    {
        printf("0-20 %d %.2f%%\n", bellow_20, (float)bellow_20/(float)this->population_bellow_20*100);
    }
    if (this->population_20_40 == 0)
    {
        printf("20-40 %d %.2f%%\n", between_20_40, (float)0);
    }
    else
    {
        printf("20-40 %d %.2f%%\n", between_20_40, (float)between_20_40/(float)this->population_20_40*100);
    }
    if (this->population_40_60 == 0)
    {
        printf("40-60 %d %.2f%%\n", between_40_60, (float)0);
    }
    else
    {
        printf("40-60 %d %.2f%%\n", between_40_60, (float)between_40_60/(float)this->population_40_60*100);
    }
    if (this->population_60_plus == 0)
    {
        printf("60+ %d %.2f%%\n", plus_60, (float)0);
    }
    else
    {
        printf("60+ %d %.2f%%\n", plus_60, (float)plus_60/(float)this->population_60_plus*100);
    }
}

void CountryStatus::displayTotalPopulationStatus(char *virus_name, Date start, Date end)
{
    int vaccinated_citizens = 0;
    VirusCountryStatus *virus_tree;

    if (this->total_population == 0)
    {
        printf("%s %d %.2f%%\n", this->country_name, vaccinated_citizens, (float)0);
        return;
    }
    virus_tree = static_cast<VirusCountryStatus*>(this->virus_status->getElement(virus_name, compareNameVirusCountryStatus));
    if (virus_tree != NULL)
    {
        if (start.isNullDate())
        {
            virus_tree->getTotalVaccinationStats(vaccinated_citizens);
        }
        else
        {
            virus_tree->getTotalVaccinationStats(vaccinated_citizens, start, end);
        }        
    }
    printf("%s %d %.2f%%\n", this->country_name, vaccinated_citizens, ((float)vaccinated_citizens/(float)total_population)*100);
}

/**
 * Compares the given virus name to the virus name of the
 * specified VirusCountryStatus.
 * @param name A char* string with a virus name
 * @param virus_status A pointer to a VirusCountryStatus
 * @return 0 if names are equal, otherwise returns a non-zero number.
 */
int compareNameVirusCountryStatus(void *name, void *virus_status)
{
    return strcmp((char*)name, (static_cast<VirusCountryStatus*>(virus_status))->virus_name);
}

int compareNameCountryStatus(void *name, void *country_status)
{
    return strcmp((char*)name, static_cast<CountryStatus*>(country_status)->country_name);
}

/*
void destroyVirusCountryStatus(void *status)
{
    delete (VirusCountryStatus*)status;
}
*/

/**
 * Functions to be used by main -------------------------------------------------------------------
 */

void insertVaccinationRecord(int citizen_id, char *full_name, char *country_name, int age,
                             char *virus_name, bool vaccinated, Date date,
                             LinkedList *countries, LinkedList *viruses, HashTable *citizens,
                             unsigned long bloom_bytes, FILE *fstream)
{
    CitizenRecord *target_citizen;
    CitizenRecord *present = static_cast<CitizenRecord*>(citizens->getElement(&citizen_id, compareIdToCitizen));

    if (present != NULL)
    // There is already a citizen with the specified ID
    {
        if ( !present->hasInfo(citizen_id, full_name, age, country_name) )
        // The provided info is not valid, so the request is rejected
        {
            fprintf(fstream, "ERROR: A citizen with the same ID, but different info already exists:\n");
            displayCitizen(present);
            return;
        }
        // The provided info is valid, so the existed citizen record will be used
        target_citizen = present;
    }
    else
    // There is no citizen with the specified ID, so a new one will be made
    {
        CountryStatus *target_country;
        target_country =  static_cast<CountryStatus*>(countries->getElement(country_name, compareNameCountryStatus));
        if (target_country == NULL)
        {
            target_country = new CountryStatus(country_name);
            countries->append(target_country);
        }
        target_citizen = new CitizenRecord(citizen_id, full_name, age, target_country);
        target_country->updatePopulation(target_citizen);
        citizens->insert(target_citizen);
    }
    VirusRecords *target_virus = (VirusRecords*)viruses->getElement(virus_name, compareNameVirusRecord);
    if (target_virus == NULL)
    {
        target_virus = new VirusRecords(virus_name, SKIP_LIST_MAX_LAYERS, bloom_bytes);
        viruses->append(target_virus);
    }
    
    VaccinationRecord *new_record;
    VaccinationRecord *existing;
    bool status_changed = false;
    if (vaccinated)
    {
        new_record = new VaccinationRecord(target_citizen, vaccinated, target_virus->virus_name, date);
    }
    else
    {
        new_record = new VaccinationRecord(target_citizen, vaccinated, target_virus->virus_name);
    }
    if (target_virus->insertRecordOrShowExisted(new_record, &existing, status_changed, fstream))
    // The vaccination record was successfully stored
    {
        if (new_record->vaccinated)
        // The citizen has been vaccinated, so store this record
        {
            new_record->citizen->country->storeCitizenVaccinationRecord(new_record);
        }
    }
    else
    // The record was not inserted (was already present)
    {
        delete new_record;
        if (status_changed)
        // If the status of the existing record changed from "non vaccinated"
        // to "vaccinated", it must be stored now
        {
            existing->citizen->country->storeCitizenVaccinationRecord(existing);
        }
    }    
}

void vaccineStatus(int citizen_id, LinkedList *viruses)
{
    LinkedList::ListIterator itr = viruses->listHead();

    while ( !itr.isNull() )
    {
        static_cast<VirusRecords*>(itr.getData())->displayVaccinationStatus(citizen_id);
        itr.forward();
    }
}

void vaccineStatus(int citizen_id, LinkedList *viruses, char *virus_name)
{
    VirusRecords *target_virus = static_cast<VirusRecords*>(viruses->getElement(virus_name, compareNameVirusRecord));
    if (target_virus != NULL)
    {
        target_virus->displayWhetherVaccinated(citizen_id);
    }
    else
    {
        printf("ERROR: The specified virus was not found.\n");
    }    
}

void vaccineStatusBloom(int citizen_id, LinkedList *viruses, char *virus_name)
{
    VirusRecords *target_virus = static_cast<VirusRecords*>(viruses->getElement(virus_name, compareNameVirusRecord));
    
    char char_id[5]; // max 4 digits + \0
    sprintf(char_id, "%d", citizen_id);
    if (target_virus != NULL)
    {
        if (target_virus->checkBloomFilter(char_id))
        {
            printf("MAYBE\n");
        }
        else
        {
            printf("NOT VACCINATED\n");
        }
    }
    else
    {
        printf("ERROR: The specified virus was not found.\n");
    }
}

void listNonVaccinatedPersons(char *virus_name, LinkedList *viruses)
{
    VirusRecords *target_virus = static_cast<VirusRecords*>(viruses->getElement(virus_name, compareNameVirusRecord));
    if (target_virus != NULL)
    {
        target_virus->displayNonVaccinated();
    }
    else
    {
        printf("ERROR: The specified virus was not found.\n");
    }
}

void populationStatus(char *virus_name, LinkedList *countries, Date start, Date end)
{
    LinkedList::ListIterator itr = countries->listHead();

    while ( !itr.isNull() )
    {
        static_cast<CountryStatus*>(itr.getData())->displayTotalPopulationStatus(virus_name, start, end);
        itr.forward();
    }
}

void populationStatus(char *virus_name, char *country_name, LinkedList *countries, Date start, Date end)
{
    CountryStatus* target_country;
    target_country = static_cast<CountryStatus*>(countries->getElement(country_name, compareNameCountryStatus));

    if (target_country != NULL)
    {
        target_country->displayTotalPopulationStatus(virus_name, start, end);
    }
    else
    {
        printf("ERROR: The specified country was not found.\n");
    }
}

void popStatusByAge(char *virus_name, LinkedList *countries, Date start, Date end)
{
    LinkedList::ListIterator itr = countries->listHead();

    while ( !itr.isNull() )
    {
        static_cast<CountryStatus*>(itr.getData())->displayStatusByAge(virus_name, start, end);
        itr.forward();
    }    
}

void popStatusByAge(char *virus_name, char *country_name, LinkedList *countries, Date start, Date end)
{
    CountryStatus* target_country;
    target_country = static_cast<CountryStatus*>(countries->getElement(country_name, compareNameCountryStatus));

    if (target_country != NULL)
    {
        target_country->displayStatusByAge(virus_name, start, end);
    }
    else
    {
        printf("ERROR: The specified country was not found.\n");
    }
}