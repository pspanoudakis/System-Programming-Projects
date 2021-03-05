#ifndef APP_CLASSES_HPP
#define APP_CLASSES_HPP

typedef char *Country;
typedef char *Virus;

struct Date
{
    int day;
    int month;
    int year;
};

/*
struct Country
{
    char *name;
};

struct Virus
{
    char *name;
};
*/

class CitizenRecord
{
    private:
        const int id;
        char *fullname;
        int age;
        Country *country;
    public:
        CitizenRecord(int citizenID, const char *name, int citizenAge, Country *citizenCountry);
        ~CitizenRecord();
};

class VaccinationRecord
{
    private:
        CitizenRecord *citizen;
        Virus virus;
        Date date;        
    public:
        VaccinationRecord(CitizenRecord *person, Virus v, Date d);
        ~VaccinationRecord();
};

#endif
