#ifndef APP_CLASSES_HPP
#define APP_CLASSES_HPP

typedef char *Country;
typedef char *Virus;

struct Date
{
    int day;
    int month;
    int year;

    Date(int d = 0, int m = 0, int y = 0);
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
        bool vaccinated;
        Date date;        
    public:
        VaccinationRecord(CitizenRecord *person, bool is_vaccinated, Virus v, Date d = Date());
        ~VaccinationRecord();
};

#endif
