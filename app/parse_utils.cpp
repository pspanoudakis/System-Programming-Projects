/**
 * File: parse_utils.cpp
 * Contains implementations for several routines used in input parsing
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "app_utils.hpp"
#include "parse_utils.hpp"

bool parseDateString(const char *string, Date &date)
{
    unsigned short int len = strlen(string);
    if (len < 8 || len > 10)
    {
        return false;
    }
    sscanf(string, "%hu-%hu-%hu", &date.day, &date.month, &date.year);
    return date.isValidDate();
}

bool isPositiveNumber(const char* str)
{
    char* endptr;    
    return (strtol(str, &endptr, 10) >= 0) && (*endptr == '\0');
}

/**
 * Reads a character sequence up to a newline character or EOF from stream,
 * and returns a character buffer which contains the sequence.
 * Note that the returned pointer has to be free-ed after use.
 */
char* fgetline(FILE *stream)
{
    int c;
    int count = 1;
    char *buffer = NULL;

    c = fgetc(stream);
    while ( (c != '\n') && (c != EOF) )
    {  
        buffer = (char*)realloc(buffer, count + 1);           // Increasing the space by 1 byte
        // TODO: maybe display something?
        assert(buffer != NULL);
        buffer[count - 1] = c;                               // Storing the new letter
        buffer[count] = '\0';
        count++;
        c = fgetc(stream);
    }

    return buffer;
}

/**
 * Routines used for command argument parsing-checking --------------------------------------------
 */

bool insertCitizenRecordParse(int &citizen_id, char *&citizen_fullname, char *&country_name,
                              int &citizen_age, char *&virus_name, bool &vaccinated, Date &date, FILE *fstream)
{
    short int curr_arg = 1;
    char *fname = NULL;
    char *token;
    citizen_fullname = NULL;
    country_name = NULL;
    virus_name = NULL;
    date.set(0, 0, 0);

    while ( (token = strtok(NULL, " "))!= NULL && curr_arg <= 8 )
    {
        switch (curr_arg)
        {
            case 1:
                // token is citizen_id
                if (!isPositiveNumber(token) || strlen(token) > 4)
                {
                    fprintf(fstream, "Invalid Citizen ID deteted. Make sure it is an up-to 4 digits number.\n");
                    return false;
                }
                citizen_id = atoi(token);
                break;
            case 2:
                // token is first name
                fname = new char [strlen(token)+1];
                strcpy(fname, token);
                break;
            case 3:
                // token is last name
                citizen_fullname = new char[strlen(fname)+strlen(token) + 2]; // +2 for space + null char
                sprintf(citizen_fullname, "%s %s", fname, token);
                delete [] fname;
                fname = NULL;
                break;
            case 4:
                // token is country
                country_name = new char [strlen(token)+1];
                strcpy(country_name, token);
                break;
            case 5:
                // token is age
                if (isPositiveNumber(token))
                {
                    citizen_age = atoi(token);
                    if (citizen_age <= 120 && citizen_age >= 0)
                    {
                        break;
                    }                
                }
                fprintf(fstream, "Invalid Citizen Age deteted. Make sure it is a number 0-120.\n");
                fprintf(fstream, "Rejecting command.\n");
                return false;
            case 6:
                // token is virus
                virus_name = new char [strlen(token)+1];
                strcpy(virus_name, token);
                break;
            case 7:
                // token is YES/NO
                if (strcmp(token, "YES") == 0)
                {
                    vaccinated = true;
                }
                else if (strcmp(token, "NO") == 0)
                {
                    vaccinated = false;
                    if ((token = strtok(NULL, " "))!= NULL)
                    {
                        fprintf(fstream, "More than expected arguments have been detected. Rejecting command.\n");
                        return false;
                    }
                    return true;
                }
                else
                {
                    fprintf(fstream, "Invalid YES/NO argument detected. Rejecting command.\n");
                    return false;
                }
                break;
            case 8:
                // token is date
                if (!parseDateString(token, date))
                {
                    fprintf(fstream, "Invalid date argument detected. Rejecting command.\n");
                    return false;
                }
                break;        
            default:
                break;
        }
        curr_arg++;
    }
    if (curr_arg == 9)
    {
        return true;
    }
    else if (curr_arg < 9)
    {
        fprintf(fstream, "Less than expected arguments have been detected. Rejecting command.\n");
    }
    else
    {
        fprintf(fstream, "More than expected arguments have been detected. Rejecting command.\n");
    }
    return false;
}

bool vaccinateNowParse(int &citizen_id, char *&citizen_fullname, char *&country_name,
                       int &citizen_age, char *&virus_name)
{
    short int curr_arg = 1;
    char *fname = NULL;
    char *token;
    citizen_fullname = NULL;
    country_name = NULL;
    virus_name = NULL;

    while ( (token = strtok(NULL, " "))!= NULL && curr_arg <= 6 )
    {
        switch (curr_arg)
        {
            case 1:
                // token is citizen_id
                if (!isPositiveNumber(token) || strlen(token) > 4)
                {
                    printf("Invalid Citizen ID deteted. Make sure it is an up-to 4 digits number.\n");
                    return false;
                }
                citizen_id = atoi(token);
                break;
            case 2:
                // token is first name
                fname = new char [strlen(token)+1];
                strcpy(fname, token);
                break;
            case 3:
                // token is last name
                citizen_fullname = new char[strlen(fname)+strlen(token) + 2]; // +2 for space + null char
                sprintf(citizen_fullname, "%s %s", fname, token);
                delete [] fname;
                fname = NULL;
                break;
            case 4:
                // token is country
                country_name = new char [strlen(token)+1];
                strcpy(country_name, token);
                break;
            case 5:
                // token is age
                if (isPositiveNumber(token))
                {
                    citizen_age = atoi(token);
                    if (citizen_age <= 120 && citizen_age >= 0)
                    {
                        break;
                    }                
                }
                printf("Invalid Citizen Age deteted. Make sure it is a number 0-120.\n");
                printf("Rejecting command.\n");
                return false;
            case 6:
                // token is virus
                virus_name = new char [strlen(token)+1];
                strcpy(virus_name, token);
                break;
            default:
                break;
        }
        curr_arg++;
    }
    if (curr_arg == 7)
    {
        return true;
    }
    else if (curr_arg < 7)
    {
        printf("Less than expected arguments have been detected. Rejecting command.\n");
    }
    else
    {
        printf("More than expected arguments have been detected. Rejecting command.\n");
    }
    return false;
}

bool vaccineStatusParse(int &citizen_id, char *&virus_name)
{
    char *arg1;
    char *arg2;
    virus_name = NULL;

    arg1 = strtok(NULL, " ");
    if (arg1 == NULL)
    {
        printf("Less than expected arguments found. Rejecting command.\n");
        return false;
    }
    if ( isPositiveNumber(arg1) && (citizen_id = atoi(arg1)) >= 0 && (citizen_id <= 9999) )
    {
        arg2 = strtok(NULL, " ");
        if (arg2 != NULL)
        {
            virus_name = new char[strlen(arg2) + 1];
            strcpy(virus_name, arg2);
        }        
        return true;
    }
    printf("Invalid Citizen ID deteted. Make sure it is an up-to 4 digits number.\n");
    return false; 
}

bool vaccineStatusBloomParse(int &citizen_id, char *&virus_name)
{
    char *arg1;
    char *arg2;
    virus_name = NULL;

    arg1 = strtok(NULL, " ");
    if (arg1 == NULL)
    {
        printf("Less than expected arguments found. Rejecting command.\n");
        return false;
    }
    if ( isPositiveNumber(arg1) && (citizen_id = atoi(arg1)) >= 0 && (citizen_id <= 9999) )
    {
        arg2 = strtok(NULL, " ");
        if (arg2 != NULL)
        {
            virus_name = new char[strlen(arg2) + 1];
            strcpy(virus_name, arg2);
            return true;
        }
        printf("Expected a virus name. Rejecting command.\n"); 
        return false;
    }
    printf("Invalid Citizen ID deteted. Make sure it is an up-to 4 digits number.\n");
    return false; 
}

bool listNonVaccinatedParse(char *&virus_name)
{
    char *arg;
    virus_name = NULL;
    arg = strtok(NULL, " ");
    if (arg == NULL)
    {
        printf("Expected a virus name. Rejecting command.\n");
        return false;
    }
    virus_name = new char[strlen(arg) + 1];
    strcpy(virus_name, arg);
    return true;
}

bool populationStatusParse(char *&country_name, char *&virus_name, Date &start, Date &end)
{
    start.set(0, 0, 0);
    end.set(0, 0, 0);
    char **args = new char*[4];

    short int curr_arg = 0;
    char *token;
    start.set(0, 0, 0);
    start.set(0, 0, 0);
    country_name = NULL;
    virus_name = NULL;
    while ( (token = strtok(NULL, " "))!= NULL && curr_arg < 4)
    {
        args[curr_arg] = token;
        curr_arg++;
    }
    if (token != NULL)
    {
        printf("More than expected arguments have been detected. Rejecting command.\n");
        delete[] args;
        return false;
    }
    else
    {
        if (curr_arg == 0)
        {
            printf("Less than expected arguments have been detected. Rejecting command.\n");
            delete[] args;
            return false;
        }
    }
    switch (curr_arg)
    {
        case 1:
            virus_name = new char[strlen(args[0])+1];
            strcpy(virus_name, args[0]);

            delete[] args;
            return true;
        case 2:
            // args are: country virus
            country_name = new char[strlen(args[0])+1];
            strcpy(country_name, args[0]);
            virus_name = new char[strlen(args[1])+1];
            strcpy(virus_name, args[1]);

            delete[] args;
            return true;
        case 3:
            // args are: virus date1 date2
            virus_name = new char[strlen(args[0])+1];
            strcpy(virus_name, args[0]);

            if ( !(parseDateString(args[1], start) && parseDateString(args[2], end)) )
            {
                delete[] args;
                printf("Invalid date argument detected. Rejecting command.\n");
                return false;
            }
            delete[] args;
            if (compareDates(&start, &end) > 0)
            {
                printf("The first Date cannot be greater than the second one. Rejecting command.\n");
                
                return false;
            }
            return true;
        case 4:
            // args are: country virus date1 date2
            country_name = new char[strlen(args[0])+1];
            strcpy(country_name, args[0]);
            virus_name = new char[strlen(args[1])+1];
            strcpy(virus_name, args[1]);

            if ( !(parseDateString(args[2], start) && parseDateString(args[3], end)) )
            {
                delete[] args;
                printf("Invalid date argument detected. Rejecting command.\n");
                return false;
            }
            delete[] args;
            if (compareDates(&start, &end) > 0)
            {
                printf("The first Date cannot be greater than the second one. Rejecting command.\n");
                return false;
            }
            return true;   
        default:
            // This point will never be reached but anyway...
            return false;
            break;
    }
}