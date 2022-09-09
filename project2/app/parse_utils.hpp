/**
 * File: parse_utils.hpp
 * Singnatures for several routines used in input parsing
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef PARSE_UTILS_HPP
#define PARSE_UTILS_HPP

class Date;

char* fgetline(FILE *stream);

/**
 * Routines used for argument parsing-checking --------------------------------------------
 */

bool isPositiveNumber(const char* str);
bool parseDateString(const char *string, Date &date);

/**
 * Routines used for command parsing-checking --------------------------------------------
 */

bool insertCitizenRecordParse(int &citizen_id, char *&citizen_fullname, char *&country_name,
                              int &citizen_age, char *&virus_name, bool &vaccinated, Date &date, FILE *fstream);
bool vaccinateNowParse(int &citizen_id, char *&citizen_fullname, char *&country_name,
                       int &citizen_age, char *&virus_name);
bool vaccineStatusBloomParse(int &citizen_id, char *&virus_name);

bool vaccineStatusParse(int &citizen_id, char *&virus_name);

bool populationStatusParse(char *&country_name, char *&virus_name, Date &start, Date &end);

bool listNonVaccinatedParse(char *&virus_name);

#endif
