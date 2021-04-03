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
#include <cstdarg>

#include "app_utils.hpp"
#include "../include/linked_list.hpp"
#include "../include/rb_tree.hpp"
#include "../include/skip_list.hpp"
#include "../include/bloom_filter.hpp"
#include "../include/hash_table.hpp"

/**
 * This is useful in cases where output is not desireable,
 * which can be indicated by passing a NULL file pointer.
 */
void displayMessage(FILE *fstream, const char *format, ...)
{
    if (fstream == NULL) { return; }
    va_list args;
    va_start(args, format);
    vfprintf(fstream, format, args);
    va_end(args);
}

/**
 * Creates a dynamic copy of the specified string and returns its address.
 */
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

/**
 * Creates a date with the specified informations.
 */
Date::Date(unsigned short int d, unsigned short int m, unsigned short int y):
day(d), month(m), year(y) { }

Date::Date(const Date &date):
day(date.day), month(date.month), year(date.year) { }

/**
 * Sets the date information according to the parameters.
 */
void Date::set(unsigned short int d, unsigned short int m, unsigned short int y)
{
    this->day = d;
    this->month = m;
    this->year = y;
}

/**
 * Checks if the Date is valid:
 * Day between 1-30, Month between 1-12, year between 1990-2100
 */
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

/**
 * Sets the Date to the today's date.
 */
void Date::setToCurrentDate()
{
    time_t timer = time(NULL);
    struct tm time_info;
	time_info = *localtime(&timer);
    this->set(time_info.tm_mday, time_info.tm_mon + 1, time_info.tm_year + 1900);
}

/**
 * Sets the Date information according to the other date.
 */
void Date::set(Date &other)
{
    this->day = other.day;
    this->month = other.day;
    this->year = other.year;
}

/**
 * Compares Dates a and b.
 * @returns > 0 if a > b, 0 if a == b and < 0 if a < b.
 */
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

/**
 * Citizen Record functions -----------------------------------------------------------------------
 */

/**
 * @brief Creates a Citizen with the specified information.
 * 
 * @param citizen_id The ID of the Citizen.
 * @param name The Citizen's name (a copy will be stored).
 * @param citizen_age The Citizen's age.
 * @param c The Citizen's country record.
 */
CitizenRecord::CitizenRecord(int citizen_id, char *name, int citizen_age, CountryStatus *c):
id(citizen_id), fullname(copyString(name)), age(citizen_age), country(c) { }

CitizenRecord::~CitizenRecord()
{
    delete [] fullname;
}

/**
 * Checks if the Citizen has the exact same stored information with the specified. 
 */
bool CitizenRecord::hasInfo(int citizen_id, char *citizen_name, int citizen_age, char *country_name)
{
    // Checking ID
    if (this->id != citizen_id)    
    {
        return false;
    }
    // Checking name
    if (strcmp(this->fullname, citizen_name) != 0)
    {
        return false;
    }
    // Checking age
    if (this->age != citizen_age)
    {
        return false;
    }
    // Checking country name
    if (strcmp(this->country->country_name, country_name) != 0)
    {
        return false;
    }
    // All checks were successful
    return true;
}

/**
 * Compares Citizens A and B.
 * @returns 0 if they are identical, > 0 if A is "greater" or < 0 if B is "greater"
 * (based on their information)
 */
int compareCitizens(void *a, void *b)
{
    CitizenRecord *c1 = static_cast<CitizenRecord*>(a);
    CitizenRecord *c2 = static_cast<CitizenRecord*>(b);

    if (c1->id != c2->id)
    // If the ID's are different, return (a->id - b->id).
    {
        return c1->id - c2->id;
    }
    int cmp = strcmp(c1->fullname, c2->fullname);
    if (cmp != 0)
    // Else if names are different, return > 0 if a->name is "greater", < 0 if b->name is "greater"
    {
        return cmp;
    }
    cmp = strcmp(c1->country->country_name, c2->country->country_name);
    if (cmp != 0)
    // Else if countries are different, return > 0 if a->country is "greater", < 0 if b->country is "greater"
    {
        return cmp;
    }
    if (c1->age != c2->age)
    // If the ages are different, return (a->age - b->age).
    {
        return c1->age - c2->age;
    }
    return 0;
}

/**
 * Compares the specified ID to the ID of the specified citizen.
 * @returns > 0 if ID > citizen->ID, 0 if they are equal, < 0 otherwise.
 */
int compareIdToCitizen(void *id, void *citizen_record)
{
    return *(static_cast<int*>(id)) - (static_cast<CitizenRecord*>(citizen_record)->id);
}

/**
 * Displays the information of the given Citizen.
 */
void displayCitizen(void *record)
{
    CitizenRecord *citizen = static_cast<CitizenRecord*>(record);
    printf("%d %s %s %d\n", citizen->id, citizen->fullname, citizen->country->country_name, citizen->age);
}

/**
 * Returns the "object" of the citizen to be used for hashing,
 * which is just the ID of the citizen.
 */
int citizenHashObject(void *citizen)
{
    return static_cast<CitizenRecord*>(citizen)->id;
}

/**
 * Vaccination Record functions -------------------------------------------------------------------
 */

/**
 * @brief Creates a Vaccination Record with the specified information.
 * 
 * @param person The Citizen that the Record refers to.
 * @param is_vaccinated Shows whether the citizen is vaccinated or not vaccinated.
 * @param virus The name of the Virus. Note than this is the *exact same* character array address
 * stored in VirusRecords for this Virus.
 * @param d The Date of the vaccination. If the citizen was not vaccinated, a null Date can be stored.
 */
VaccinationRecord::VaccinationRecord(CitizenRecord *person, bool is_vaccinated, char *virus, Date d):
citizen(person), virus_name(virus), vaccinated(is_vaccinated), date(d) { }

VaccinationRecord::~VaccinationRecord() { } // Note that virus_name is NOT deleted. 
                                            // It will be deleted when VirusRecords destructor is called.

/**
 * Change the status of a "non-vaccinated" record to "vaccinated".
 * @param d The date of the vaccination.
 */
void VaccinationRecord::vaccinate(Date d)
{
    this->vaccinated = true;
    this->date.set(d.day, d.month, d.year);
}

/**
 * Compares Vaccination Records A and B by comparing the Citizens they refer to.
 */
int compareVaccinationRecordsByCitizen(void *a, void *b)
{
    CitizenRecord *citizen1 = static_cast<VaccinationRecord*>(a)->citizen;
    CitizenRecord *citizen2 = static_cast<VaccinationRecord*>(b)->citizen;
    return compareCitizens(citizen1, citizen2);
}

/**
 * Compares Vaccination Records a and b by comparing their Dates.
 * If the Dates are equal, the citizens they refer to will be compared.
 * 
 * @returns > 0 if Record A is "greater" (either by Date or by Citizen), 0 if they are equal, < 0 otherwise.
 */
int compareVaccinationsDateFirst(void *a, void *b)
{
    VaccinationRecord *rec1 = static_cast<VaccinationRecord*>(a);
    VaccinationRecord *rec2 = static_cast<VaccinationRecord*>(b);

    int cmp = compareDates(&(rec1->date), &(rec2->date));

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

/**
 * Displays the Citizen Information of the given Vaccination Record.
 */
void displayVaccinationCitizen(void *record)
{
    CitizenRecord *citizen = static_cast<VaccinationRecord*>(record)->citizen;
    printf("%d %s %s %d\n", citizen->id, citizen->fullname, citizen->country->country_name, citizen->age);
}

/**
 * Virus Records Methods-Functions ----------------------------------------------------------------
 */

/**
 * @brief Creates a Virus Records object with the specified information.
 * @param name The name of the Virus (a copy will be stored).
 * @param skip_list_layers The number of max layers for the Skip Lists.
 * @param filter_bits The number of bytes for the Bloom Filter.
 */
VirusRecords::VirusRecords(char *name, int skip_list_layers, unsigned long filter_bytes):
vaccinated(new SkipList(skip_list_layers, delete_object<VaccinationRecord>)),
non_vaccinated(new SkipList(skip_list_layers, delete_object<VaccinationRecord>)),
filter(new BloomFilter(filter_bytes)), virus_name(copyString(name)) { }

VirusRecords::~VirusRecords()
{
    delete non_vaccinated;
    delete vaccinated;
    delete [] virus_name;
    delete filter;
}

/**
 * Prints a message to inform whether the Citizen with the specified ID
 * has been Vaccinated or not.
 * It is identical to displayWhetherVaccinated, but prints slightly different messages.
 */
void VirusRecords::displayVaccinationStatus(int citizenID)
{
    VaccinationRecord *record;
    // Search for a Citizen Vaccination Record with this ID in the Vaccinated Skip List
    record =  static_cast<VaccinationRecord*>(this->vaccinated->find(&citizenID, compareIdToVaccinationRecord));
    // Print message to indicate if a Record was found.
    if (record == NULL)
    {
        printf("%s NO\n", this->virus_name);
    }
    else
    {
        printf("%s YES %d-%d-%d\n", this->virus_name, record->date.day, record->date.month, record->date.year);   
    }
}

/**
 * Prints a message to inform whether the Citizen with the specified ID
 * has been Vaccinated or not.
 * It is identical to displayVaccinationStatus, but prints slightly different messages.
 */
void VirusRecords::displayWhetherVaccinated(int citizenID)
{
    VaccinationRecord *record;
    // Search for a Citizen Vaccination Record with this ID in the Vaccinated Skip List
    record =  static_cast<VaccinationRecord*>(this->vaccinated->find(&citizenID, compareIdToVaccinationRecord));
    // Print message to indicate if a Record was found.
    if (record == NULL)
    {
        printf("NOT VACCINATED\n");
    }
    else
    {
        printf("VACCINATED ON %d-%d-%d\n", record->date.day, record->date.month, record->date.year);   
    }    
}

/**
 * Displays all the Citizens stored in the Non-Vaccinated Skip List.
 */
void VirusRecords::displayNonVaccinated()
{
    this->non_vaccinated->displayElements(displayVaccinationCitizen);
}

/**
 * Returns TRUE if the given citizen ID is "possibly present"
 * according to the Bloom Filter, FALSE otherwise.
 */
bool VirusRecords::checkBloomFilter(char *citizen_id)
{
    return this->filter->isPresent(citizen_id);
}

/**
 * @brief Attempts to insert the specified Vaccination Record in the Skip List & Bloom Filter.
 * 
 * @param record The Vaccination Record to insert.
 * @param present If a Vaccination Record for the same citizen is found, it will be stored here.
 * @param modified This is set to TRUE if the existing Vaccination Record was modified
 * (from non-vaccinated to vaccinated)
 * @param fstream The file stream to print messages to. If output is not desirable, NULL can be passed.
 * 
 * @returns TRUE if the *given* record was inserted, FALSE otherwise. If FALSE is returned, present
 * parameter will point to the existing Record.
 */
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

            displayMessage(fstream, "SUCCESSFULLY VACCINATED\n");
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

            displayMessage(fstream, "SUCCESSFULLY VACCINATED\n");
            return true;
        }
        else
        // The insertion failed because the citizen has already been vaccinated
        {
            displayMessage(fstream, "ERROR: CITIZEN %d ALREADY VACCINATED ON %d-%d-%d\n", 
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
            displayMessage(fstream, "ERROR: CITIZEN %d ALREADY VACCINATED ON %d-%d-%d\n", 
                    (*present)->citizen->id, (*present)->date.day, (*present)->date.month, (*present)->date.year);
            // Return false to indicate that the given record must be deleted
            return false;
        }
        // Now try to insert to non-vaccinated skip list
        if ( this->non_vaccinated->insert(record, (void**)present, compareVaccinationRecordsByCitizen) )
        {
            displayMessage(fstream, "SUCCESSFULLY MARKED AS NON VACCINATED\n");
            return true;
        }
        else
        {
            displayMessage(fstream, "ERROR: ALREADY MARKED AS NON VACCINATED\n");
            return false;
        }
    }    
}

/**
 * Compares the given name to the name of the specified Virus Record.
 * @returns 0 if they are equal, > 0 if name > virus->name, < 0 otherwise.
 */
int compareNameVirusRecord(void *name, void *virus_record)
{
    char *target_name = static_cast<char*>(name);
    VirusRecords *target_record = static_cast<VirusRecords*>(virus_record);
    return strcmp(target_name, target_record->virus_name);
}

/**
 * Virus Country Status functions -----------------------------------------------------------------
 */

/**
 * @brief Creates a Virus information structure for a specific Country.
 * @param name The name of the Virus. Note than this is the *exact same* character array address
 * stored in VirusRecords for this Virus.
 * @param tree_func The comparison function to use in the Vaccinations Tree.
 */
VirusCountryStatus::VirusCountryStatus(char *name, CompareFunc tree_func):
record_tree(new RedBlackTree(tree_func)), virus_name(name) { }

VirusCountryStatus::~VirusCountryStatus()
{
    delete record_tree;                     
    // Note that virus_name is NOT deleted. 
    // It will be deleted when VirusRecords destructor is called.
}

/**
 * Stores the specified Record in the Vaccinations Tree.
 */
void VirusCountryStatus::storeVaccinationRecord(VaccinationRecord *record)
{
    this->record_tree->insert(record);
}

/**
 * While traversing the subtree starting with root recursively, 
 * counts the number of vaccinated persons between the two Dates
 * and updates the population counter.
 */
void VirusCountryStatus::getTotalStatsRec(int &total, Date start, Date end, RBTreeNode *root)
{
    if (root == NULL) { return; }
    // Getting the root Record
    VaccinationRecord *root_data = static_cast<VaccinationRecord*>(root->data);

    if (compareDates(&root_data->date, &start) >= 0)
    // root Date is greater than start
    {
        // so count stats in Left subtree
        getTotalStatsRec(total, start, end, root->left);
        if (compareDates(&root_data->date, &end) <= 0)
        // end is greater than root Date
        {
            // root is in range, so update the counter
            total++;
            // and count stats in the right subtree
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

/**
 * While traversing the subtree starting with root recursively, 
 * counts the number of vaccinated persons between the two Dates for each age group
 * and updates the age group counters.
 */
void VirusCountryStatus::getAgeStatsRec(int &bellow_20, int &between20_40, int &between40_60,
                                        int &plus60, Date start, Date end, RBTreeNode *root)
{
    if (root == NULL) { return; }
    // Getting the root Record
    VaccinationRecord *root_data = static_cast<VaccinationRecord*>(root->data);

    if (compareDates(&root_data->date, &start) >= 0)
    // root Date is greater than start
    {
        // so count stats in Left subtree
        getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, start, end, root->left);
        if (compareDates(&root_data->date, &end) <= 0)
        // end is greater than root Date
        {
            // root Date is between the limits, so update the correct counter
            updateAgeCounter(root_data->citizen->age, bellow_20, between20_40, between40_60, plus60);
            // and count stats in the right subtree
            getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, start, end, root->right);
            return;
        }
    }
    // root is smaller than start, so just search to the right
    getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, start, end, root->right);
}

/**
 * While traversing the subtree starting with root recursively, 
 * counts the number of vaccinated persons for each age group
 * and updates the age group counters.
 */
void VirusCountryStatus::getAgeStatsRec(int &bellow_20, int &between20_40, int &between40_60,
                                        int &plus60, RBTreeNode *root)
{
    if (root == NULL) { return; }
    // Getting the root Record
    VaccinationRecord *root_data = static_cast<VaccinationRecord*>(root->data);

    // Update the correct counter
    updateAgeCounter(root_data->citizen->age, bellow_20, between20_40, between40_60, plus60);
    // And count stats from left and right subtree
    getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, root->left);
    getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, root->right);
}

/**
 * Stores the number of vaccinated persons between the two Dates in the counter.
 */
void VirusCountryStatus::getTotalVaccinationStats(int &total, Date start, Date end)
{
    this->getTotalStatsRec(total, start, end, this->record_tree->root);
}

/**
 * Stores the total number of vaccinated persons in the counter.
 */
void VirusCountryStatus::getTotalVaccinationStats(int &total)
{
    total = this->record_tree->getNumElements();
}

/**
 * Stores the number of vaccinated persons between the two Dates for each age group in the proper counter.
 */
void VirusCountryStatus::getVaccinationStatsByAge( int &bellow_20, int &between20_40, int &between40_60,
                                                   int &plus60, Date start, Date end)
{
    this->getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, start, end, this->record_tree->root);
}

/**
 * Stores the number of vaccinated persons for each age group in the proper counter.
 */
void VirusCountryStatus::getVaccinationStatsByAge( int &bellow_20, int &between20_40, int &between40_60,
                                                   int &plus60)
{
    this->getAgeStatsRec(bellow_20, between20_40, between40_60, plus60, this->record_tree->root);
}

/**
 * Updates the correct age counter according to the given age.
 * @param age The given Age.
 * @param bellow_20 The counter for age group [0-19]
 * @param between20_40 The counter for age group [20-39]
 * @param between40_60 The counter for age group [40-59]
 * @param plus60 The counter for age group 60+
 */
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

/**
 * Creates a Country structure with the specified name. 
 * @param name The name of the Country.
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

/**
 * Stores the specified Vaccination Record in the proper Virus information structure of this Country.
 */
void CountryStatus::storeCitizenVaccinationRecord(VaccinationRecord *record)
{
    // The Country information about the specified Virus will be stored here
    VirusCountryStatus *virus_tree;
    // Get this Country information about the specified Virus
    virus_tree = static_cast<VirusCountryStatus*>(this->virus_status->getElement(record->virus_name, compareNameVirusCountryStatus));
    if (virus_tree == NULL)
    // Did not find information about this Virus
    {
        // So create it now
        this->virus_status->append(new VirusCountryStatus(record->virus_name, compareVaccinationsDateFirst));
        virus_tree = static_cast<VirusCountryStatus*>(this->virus_status->getLast());
    }
    // Store the Record in the Virus structure.
    virus_tree->storeVaccinationRecord(record);   
}

/**
 * Updates the Country total population and age group population
 * based on the specified citizen.
 */
void CountryStatus::updatePopulation(CitizenRecord *citizen)
{
    // Increase total population counter
    this->total_population++;
    // Then increase population counter for the citizen age group
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

/**
 * Displays the age groups statuses in this Country regarding the specified Virus.
 * If the given Dates are valid, the results will be filtered accordingly.
 */
void CountryStatus::displayStatusByAge(char *virus_name, Date start, Date end)
{
    // Number of vaccinated Citizens for each age group
    int bellow_20 = 0;
    int between_20_40 = 0;
    int between_40_60 = 0;
    int plus_60 = 0;

    VirusCountryStatus *virus_tree;         // The Country information about the specified Virus will be stored here

    // Get this Country information about the specified Virus
    virus_tree = static_cast<VirusCountryStatus*>(this->virus_status->getElement(virus_name, compareNameVirusCountryStatus));

    if (virus_tree != NULL)
    // Found information about this Virus
    {
        if (start.isNullDate())
        // If Dates are null, calculate total stats
        {
            virus_tree->getVaccinationStatsByAge(bellow_20, between_20_40, between_40_60, plus_60);
        }
        else
        // Otherwise calculate stats between the given Dates
        {
            virus_tree->getVaccinationStatsByAge(bellow_20, between_20_40, between_40_60, plus_60, start, end);
        }        
    }

    // Print the results
    // If the country has no population in an age group just print 0 percentage    
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

/**
 * Displays the Population Status in this Country regarding the specified virus.
 * If the given Dates are valid, the results will be filtered accordingly.
 */
void CountryStatus::displayTotalPopulationStatus(char *virus_name, Date start, Date end)
{
    int vaccinated_citizens = 0;            // Total number of vaccinated Citizens
    VirusCountryStatus *virus_tree;         // The Country information about the specified Virus will be stored here

    if (this->total_population == 0)
    // If the country has no population just print 0 percentage (although this won't happen normally)
    {
        printf("%s %d %.2f%%\n", this->country_name, vaccinated_citizens, (float)0);
        return;
    }
    // Get this Country information about the specified Virus
    virus_tree = static_cast<VirusCountryStatus*>(this->virus_status->getElement(virus_name, compareNameVirusCountryStatus));
    if (virus_tree != NULL)
    // Found information about this Virus
    {
        if (start.isNullDate())
        // If Dates are null, calculate total stats
        {
            virus_tree->getTotalVaccinationStats(vaccinated_citizens);
        }
        else
        // Otherwise calculate stats between the given Dates
        {
            virus_tree->getTotalVaccinationStats(vaccinated_citizens, start, end);
        }        
    }
    // Print the results
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

/**
 * Compares the given Name to the name of the specified Country.
 * @returns > 0 if name > country->name, 0 if they are equal, < 0 otherwise.
 */
int compareNameCountryStatus(void *name, void *country_status)
{
    return strcmp((char*)name, static_cast<CountryStatus*>(country_status)->country_name);
}

/**
 * Functions to be used by main in order to execute user commands ---------------------------------
 */

/**
 * @brief Execute the /insertVaccinationRecord command with the specified arguments
 * (a Citizen ID, a Citizen name, a Country name, an Age, a Virus name, a boolean (YES/NO) variable,
 * and possibly a Vaccination Date.).
 * @param countries A Linked List with the Country Statuses.
 * @param viruses A Linked List with the Virus Records.
 * @param citizens A HashTable with all the stored Citizens.
 * @param bloom_bytes The number of bytes in Bloom Filters.
 * @param fstream The file stream to print output messages. If output is not desirable, NULL can be passed.
 */
void insertVaccinationRecord(int citizen_id, char *full_name, char *country_name, int age,
                             char *virus_name, bool vaccinated, Date &date,
                             LinkedList *countries, LinkedList *viruses, HashTable *citizens,
                             unsigned long bloom_bytes, FILE *fstream)
{
    // First try to find or create the citizen with the specified info.
    CitizenRecord *target_citizen;

    // Trying to find if this citizen is already stored.
    CitizenRecord *present = static_cast<CitizenRecord*>(citizens->getElement(&citizen_id, compareIdToCitizen));

    if (present != NULL)
    // There is already a citizen with the specified ID
    {
        if ( !present->hasInfo(citizen_id, full_name, age, country_name) )
        // The provided info is not valid, so the request is rejected
        {
            displayMessage(fstream, "ERROR: A citizen with the same ID, but different info already exists:\n");
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
    // Trying to find a Virus with that name
    VirusRecords *target_virus = (VirusRecords*)viruses->getElement(virus_name, compareNameVirusRecord);
    if (target_virus == NULL)
    // There is no Virus with the specified name, so a new one will be made
    {
        target_virus = new VirusRecords(virus_name, SKIP_LIST_MAX_LAYERS, bloom_bytes);
        viruses->append(target_virus);
    }
    // The new Vaccination Record will be stored here
    VaccinationRecord *new_record;
    // If there is an existing Record for this Citizen and this Virus, store it here
    VaccinationRecord *existing;
    // This will indicate if the existing Record changed (from non-vaccinated to vaccinated)
    bool status_changed = false;
    // Creating a new Vaccination Record
    if (vaccinated)
    {
        new_record = new VaccinationRecord(target_citizen, vaccinated, target_virus->virus_name, date);
    }
    else
    {
        new_record = new VaccinationRecord(target_citizen, vaccinated, target_virus->virus_name);
    }
    // Try to store the record or record an existing one for this Citizen and this Virus
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
        // to "vaccinated", it must be stored in the Country structure now.
        {
            existing->citizen->country->storeCitizenVaccinationRecord(existing);
        }
    }    
}

/**
 * Executes the /vaccineStatus command with a citizen ID as the only argument.
 * 
 * @param viruses A Linked List with the Virus Records.
 */
void vaccineStatus(int citizen_id, LinkedList *viruses)
{
    LinkedList::ListIterator itr = viruses->listHead();

    while ( !itr.isNull() )
    // Iterate over all viruses
    {
        // Display Status for this citizen
        static_cast<VirusRecords*>(itr.getData())->displayVaccinationStatus(citizen_id);
        itr.forward();
    }
}

/**
 * Executes the /vaccineStatus command with a citizen ID and a Virus name as arguments.
 * 
 * @param viruses A Linked List with the Virus Records.
 */
void vaccineStatus(int citizen_id, LinkedList *viruses, char *virus_name)
{
    // Get the Records for the specified virus
    VirusRecords *target_virus = static_cast<VirusRecords*>(viruses->getElement(virus_name, compareNameVirusRecord));
    if (target_virus != NULL)
    // Records found, so display whether the citizen is vaccinated
    {
        target_virus->displayWhetherVaccinated(citizen_id);
    }
    else
    // Records not found for this Virus
    {
        printf("ERROR: The specified virus was not found.\n");
    }    
}

/**
 * Executes the /vaccineStatusBloom command with the specified arguments.
 *
 * @param viruses A Linked List with the Virus Records.
 */
void vaccineStatusBloom(int citizen_id, LinkedList *viruses, char *virus_name)
{
    // Get Records for this Virus
    VirusRecords *target_virus = static_cast<VirusRecords*>(viruses->getElement(virus_name, compareNameVirusRecord));
    
    // Storing citizen ID in a string
    char char_id[5]; // max 4 digits + \0
    sprintf(char_id, "%d", citizen_id);
    if (target_virus != NULL)
    // Records found for this Virus
    {
        // So check bloom filter
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
    // Records not found for this Virus
    {
        printf("ERROR: The specified virus was not found.\n");
    }
}

/**
 * Executes the /listNonVaccinatedPersons command with the specified Virus name as argument.
 * 
 * @param viruses A Linked List with the Virus Records.
 */
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

/**
 * Executes the /populationStatus command with a Virus name and (possibly) 2 Dates as arguments.
 * If no Date arguments were given by the user, 2 null Dates can be passed in order to be ignored.
 * 
 * @param countries A Linked List with the Country Statuses.
 */
void populationStatus(char *virus_name, LinkedList *countries, Date start, Date end)
{
    LinkedList::ListIterator itr = countries->listHead();

    while ( !itr.isNull() )
    // Iterate over the Countries
    {
        // Display population status for each country
        static_cast<CountryStatus*>(itr.getData())->displayTotalPopulationStatus(virus_name, start, end);
        itr.forward();
    }
}

/**
 * Executes the /populationStatus command with a Virus name, a Country name and (possibly) 2 Dates as arguments.
 * If no Date arguments were given by the user, 2 null Dates can be passed in order to be ignored.
 * 
 * @param countries A Linked List with the Country Statuses.
 */
void populationStatus(char *virus_name, char *country_name, LinkedList *countries, Date start, Date end)
{
    CountryStatus* target_country;
    // Find this specific Country
    target_country = static_cast<CountryStatus*>(countries->getElement(country_name, compareNameCountryStatus));

    if (target_country != NULL)
    // Country found
    {
        // Display population status for this country
        target_country->displayTotalPopulationStatus(virus_name, start, end);
    }
    else
    // Country not found
    {
        printf("ERROR: The specified country was not found.\n");
    }
}

/**
 * Executes the /popStatusByAge command with a Virus name and (possibly) 2 Dates as arguments.
 * If no Date arguments were given by the user, 2 null Dates can be passed in order to be ignored.
 * 
 * @param countries A Linked List with the Country Statuses.
 */
void popStatusByAge(char *virus_name, LinkedList *countries, Date start, Date end)
{
    LinkedList::ListIterator itr = countries->listHead();

    while ( !itr.isNull() )
    // Iterate over the Countries
    {
        // Display population status for each country
        static_cast<CountryStatus*>(itr.getData())->displayStatusByAge(virus_name, start, end);
        itr.forward();
    }    
}

/**
 * Executes the /popStatusByAge command with a Virus name, a Country name and (possibly) 2 Dates as arguments.
 * If no Date arguments were given by the user, 2 null Dates can be passed in order to be ignored.
 * 
 * @param countries A Linked List with the Country Statuses.
 */
void popStatusByAge(char *virus_name, char *country_name, LinkedList *countries, Date start, Date end)
{
    CountryStatus* target_country;
    // Find this specific Country
    target_country = static_cast<CountryStatus*>(countries->getElement(country_name, compareNameCountryStatus));

    if (target_country != NULL)
    // Country found
    {
        // Display each age group status for this country
        target_country->displayStatusByAge(virus_name, start, end);
    }
    else
    // Country not found
    {
        printf("ERROR: The specified country was not found.\n");
    }
}
