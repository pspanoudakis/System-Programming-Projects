#ifndef PARSE_UTILS_HPP
#define PARSE_UTILS_HPP

bool isPositiveNumber(const char* str);
char* fgetline(FILE *stream);
bool insertCitizenRecordParse(int &citizen_id, char *&citizen_fullname, char *&country_name,
                              int &citizen_age, char *&virus_name, bool &vaccinated, Date &date);

#endif
