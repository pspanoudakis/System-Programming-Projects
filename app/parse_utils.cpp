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
        assert(buffer != NULL);
        buffer[count - 1] = c;                               // Storing the new letter
        buffer[count] = '\0';
        count++;
        c = fgetc(stream);
    }

    return buffer;
}

bool insertCitizenRecordParse(int &citizen_id, char *&citizen_fullname, char *&country_name,
                              int &citizen_age, char *&virus_name, bool &vaccinated, Date &date)
{
    short int curr_arg = 1;
    char *fname = NULL;
    char *token;
    citizen_fullname = NULL;
    country_name = NULL;
    virus_name = NULL;

    while ( (token = strtok(NULL, " "))!= NULL )
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
            // TODO: release sources maybe?
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
                    printf("More than expected arguments have been detected. Rejecting command.\n");
                    // TODO: release sources maybe?
                    delete[] country_name;
                    delete[] citizen_fullname;
                    delete[] virus_name;
                    return false;
                }
                return true;
            }
            else
            {
                printf("Invalid YES/NO argument detected. Rejecting command.\n");
                // TODO: release sources maybe?
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
            return false;
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
        printf("Less than expected arguments have been detected. Rejecting command.\n");
    }
    else
    {
        printf("More than expected arguments have been detected. Rejecting command.\n");
    }
    // TODO: release sources maybe?
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