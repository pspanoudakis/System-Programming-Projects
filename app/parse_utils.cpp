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

bool isPositiveNumber(const char* str)
{
    char* endptr;    
    return (strtol(str, &endptr, 10) > 0) && (*endptr == '\0');
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

    while ( (token = strtok(NULL, " "))!= NULL && curr_arg <= 9 )
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
            delete[] country_name;
            delete[] citizen_fullname;
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
                    delete[] country_name;
                    delete[] citizen_fullname;
                    delete[] virus_name;
                    return false;
                }
                return true;
            }
            else
            {
                fprintf(fstream, "Invalid YES/NO argument detected. Rejecting command.\n");
                delete[] country_name;
                delete[] citizen_fullname;
                delete[] virus_name;
                return false;
            }
            break;
        case 8:
            // token is date
            // TODO: this is temporary
            sscanf(token, "%hu-%hu-%hu", &date.day, &date.month, &date.year);
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
    if (citizen_fullname != NULL)
    {
        delete[] citizen_fullname;
        if (country_name != NULL)
        {
            delete[] country_name;
            if (virus_name != NULL)
            {
                delete[] virus_name;
            }
        }
    }
    else
    {
        if (fname != NULL)
        {
            delete[] fname;
        }
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

    while ( (token = strtok(NULL, " "))!= NULL && curr_arg <=7 )
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
            delete[] country_name;
            delete[] citizen_fullname;
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
    if (citizen_fullname != NULL)
    {
        delete[] citizen_fullname;
        if (country_name != NULL)
        {
            delete[] country_name;
            if (virus_name != NULL)
            {
                delete[] virus_name;
            }
        }
    }
    else
    {
        if (fname != NULL)
        {
            delete[] fname;
        }
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
    if ( isPositiveNumber(arg1) && (citizen_id = atoi(arg1)) >= 0 && (citizen_id <= 120) )
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
    if ( isPositiveNumber(arg1) && (citizen_id = atoi(arg1)) >= 0 && (citizen_id <= 120) )
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
    short int curr_arg = 1;
    char *token;
    country_name = NULL;
    virus_name = NULL;

    while ( (token = strtok(NULL, " "))!= NULL && curr_arg <= 5 )
    {
        switch (curr_arg)
        {
        case 1:
            // token is country
            country_name = new char [strlen(token)+1];
            strcpy(country_name, token);
            break;
        case 2:
            // token is virus
            virus_name = new char [strlen(token)+1];
            strcpy(virus_name, token);
            break;
        case 3:
            // token is date
            // TODO: this is temporary
            sscanf(token, "%hu-%hu-%hu", &start.day, &start.month, &start.year);
            curr_arg++;
            if ( (token = strtok(NULL, " "))!= NULL )
            {
                sscanf(token, "%hu-%hu-%hu", &start.day, &start.month, &start.year);
                if (compareDates(&start, &end) > 0)
                {
                    printf("The first Date cannot be greater than the second one. Rejecting command.\n");
                    delete[] country_name;
                    delete[] virus_name;
                    return false;
                }
            }
            else
            {
                printf("Expected an upper limit Date. Rejecting command.\n");
                delete[] country_name;
                delete[] virus_name;
                return false;
            }
            break;
        default:
            break;
        }
        curr_arg++;
    }
    if (curr_arg == 3)
    {
        return true;
    }
    else if (curr_arg < 3)
    {
        printf("Less than expected arguments have been detected. Rejecting command.\n");
    }
    else
    {
        printf("More than expected arguments have been detected. Rejecting command.\n");
    }
    if (country_name != NULL)
    {
        delete[] country_name;
        if (virus_name != NULL)
        {
            delete[] virus_name;
        }
    }
    return false;
}