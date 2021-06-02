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

bool parentCheckParseArgs(int argc, char const *argv[], char *&directory_path, unsigned int &num_monitors,
                    unsigned long &bloom_size, unsigned int &buffer_size, unsigned int &cyclic_buffer_size,
                    unsigned int &num_threads);

bool childCheckparseArgs(int argc, char const *argv[], uint16_t &port,
               unsigned int &num_threads, unsigned int &buffer_size, unsigned int &cyclic_buffer_size,
               DirectoryInfo **&directories, unsigned long &bloom_size, unsigned int &num_dirs);

bool addVaccinationRecordsParse(char *&country_name);

bool searchVaccinationStatusParse(unsigned int &citizen_id);

bool travelRequestParse(unsigned int &citizen_id, Date &date, char *&countryFrom, char *&countryTo, char *&virus_name);

bool travelStatsParse(char *&virus_name, Date &start, Date &end, char *&country_name);

#endif
