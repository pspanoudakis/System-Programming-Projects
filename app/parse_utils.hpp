#ifndef PARSE_UTILS_HPP
#define PARSE_UTILS_HPP

bool isPositiveNumber(const char* str);
char* fgetline(FILE *stream);

/**
 * Routines used for command argument parsing-checking --------------------------------------------
 */

bool insertCitizenRecordParse(int &citizen_id, char *&citizen_fullname, char *&country_name,
                              int &citizen_age, char *&virus_name, bool &vaccinated, Date &date, FILE *fstream);
bool vaccinateNowParse(int &citizen_id, char *&citizen_fullname, char *&country_name,
                       int &citizen_age, char *&virus_name);
bool vaccineStatusBloomParse(int &citizen_id, char *&virus_name);

// 2 cases here! we may not get a virus name
bool vaccineStatusParse(int &citizen_id, char *&virus_name);

// Again, 2 cases! we may not get a country name.
bool populationStatusParse(char *&country_name, char *&virus_name, Date &start, Date &end);
// They can cover each over probably
//bool popStatusByAgeParse(char *&country_name, char *&virus_name, Date &start, Date &end);

bool listNonVaccinatedParse(char *&virus_name);

#endif
