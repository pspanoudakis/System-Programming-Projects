/**
 * File: parse_utils.cpp
 * Contains implementations for several routines used in input parsing.
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "app_utils.hpp"
#include "parse_utils.hpp"

/**
 * @brief Parses the specified string into the given date.
 * 
 * @param string The string to extract the date from.
 * @param date The date object to store the information in.
 * 
 * @returns TRUE if successful, FALSE otherwise.
 */
bool parseDateString(const char *string, Date &date)
{
    unsigned short int len = strlen(string);
    if (len < 8 || len > 10)
    // A valid date string has 8-10 characters
    {
        return false;
    }
    // Just extract info using sscanf
    sscanf(string, "%hu-%hu-%hu", &date.day, &date.month, &date.year);
    // Check if the Date is valid
    return date.isValidDate();
}

/**
 * Checks whether the given string represents a positive integer.
 */
bool isPositiveNumber(const char* str)
{
    char* endptr;    
    return (strtol(str, &endptr, 10) >= 0) && (*endptr == '\0');
}

/**
 * Reads a character sequence up to a newline character or EOF from stream, 
 * and returns a character buffer which contains the sequence.
 * 
 * @returns A pointer to the character buffer containing the line.
 * If a line was not read, NULL will be returned.
 * Note that the returned pointer has to be free-d after use.
 */
char* fgetline(FILE *stream)
{
    int c;
    int count = 1;
    char *buffer = NULL;                                    // The line will be stored here

    // Reading from stream character by character
    c = fgetc(stream);
    // Until EOF or \n found
    while ( (c != '\n') && (c != EOF) )
    {  
        buffer = (char*)realloc(buffer, count + 1);         // Increasing the line buffer by 1 byte
        assert(buffer != NULL);
        buffer[count - 1] = c;                              // Storing the new letter in buffer
        buffer[count] = '\0';
        count++;
        c = fgetc(stream);                                  // Read next character
    }

    return buffer;                                          // Return when done.
}

/**
 * Routines used for command argument parsing-checking --------------------------------------------
 */

/**
 * @brief Parses the string stored in strtok buffer into /insertCitizenRecord
 * command arguments, and stores the argument values in the specified variables.
 * 
 * @param citizen_id The Citizen ID will be stored here.
 * @param citizen_fullname The Citizen Full Name will be stored here.
 * @param country_name The citizen Country name will be stored here.
 * @param citizen_age The Citizen Age will be stored here.
 * @param virus_name The Virus Name will be stored here.
 * @param vaccinated This will be either TRUE/FALSE if YES/NO argument was given respectively.
 * @param date The Date given by the user (if given) will be stored here. If a Date was not given,
 * a null Date will be stored.
 * @param fstream The file stream to print output messages. If output is not desirable,
 * NULL can be passed.
 * 
 * @returns TRUE if parsing was successful, FALSE if an error/bad argument was detected.
 */
bool insertCitizenRecordParse(int &citizen_id, char *&citizen_fullname, char *&country_name,
                              int &citizen_age, char *&virus_name, bool &vaccinated, Date &date, FILE *fstream)
{
    short int curr_arg = 1;     // This indicates which argument is examined
    char *fname = NULL;         // A temporary buffer for the Citizen First name
    char *token;                // This is used to obtain the token returned by strtok
    
    // Initializing the given arguments to NULL
    citizen_fullname = NULL;
    country_name = NULL;
    virus_name = NULL;
    date.set(0, 0, 0);

    // Loop until strtok no longer gives other tokens or more than expected arguments detected
    while ( (token = strtok(NULL, " "))!= NULL && curr_arg <= 9 )
    {
        switch (curr_arg)
        {
            case 1:
                // token is citizen_id
                if (!isPositiveNumber(token) || strlen(token) > MAX_ID_DIGITS)
                {
                    displayMessage(fstream, "Invalid Citizen ID deteted. Make sure it is an up-to %d digits number.\n",
                                    MAX_ID_DIGITS);
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
                // Invalid Age argument
                displayMessage(fstream, "Invalid Citizen Age deteted. Make sure it is a number 0-120.\nRejecting command.\n");
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
                    // argument was "NO" but there are still more arguments, which is unexpected.
                    {
                        displayMessage(fstream, "More than expected arguments have been detected. Rejecting command.\n");
                        return false;
                    }
                    return true;
                }
                else
                // The argument was not YES/NO so it is not valid
                {
                    displayMessage(fstream, "Invalid YES/NO argument detected. Rejecting command.\n");
                    return false;
                }
                break;
            case 8:
                // token is date
                if (!parseDateString(token, date))
                // Could not parse Date token successfully 
                {
                    displayMessage(fstream, "Invalid date argument detected. Rejecting command.\n");
                    return false;
                }
                break;        
            default:
                break;
        }
        curr_arg++;
    }
    // Checking how many arguments have been read
    if (curr_arg == 9)
    {
        return true;
    }
    else if (curr_arg < 9)
    {
        displayMessage(fstream, "Less than expected arguments have been detected. Rejecting command.\n");
    }
    else
    {
        displayMessage(fstream, "More than expected arguments have been detected. Rejecting command.\n");
    }
    return false;
}

/**
 * @brief Parses the string stored in strtok buffer into /vaccinateNow
 * command arguments, and stores the argument values in the specified variables.
 * 
 * @param citizen_id The Citizen ID will be stored here.
 * @param citizen_fullname The Citizen Full Name will be stored here.
 * @param country_name The citizen Country name will be stored here.
 * @param citizen_age The Citizen Age will be stored here.
 * @param virus_name The Virus Name will be stored here.
 * @param vaccinated This will be either TRUE/FALSE if YES/NO argument was given respectively.
 * 
 * @returns TRUE if parsing was successful, FALSE if an error/bad argument was detected.
 */
bool vaccinateNowParse(int &citizen_id, char *&citizen_fullname, char *&country_name,
                       int &citizen_age, char *&virus_name)
{
    short int curr_arg = 1;         // This indicates which argument is examined
    char *fname = NULL;             // A temporary buffer for the Citizen First name
    char *token;                    // This is used to obtain the token returned by strtok

    // Initializing the given arguments to NULL
    citizen_fullname = NULL;
    country_name = NULL;
    virus_name = NULL;

    // Loop until strtok no longer gives other tokens or more than expected arguments detected
    while ( (token = strtok(NULL, " "))!= NULL && curr_arg <= 7 )
    {
        switch (curr_arg)
        {
            case 1:
                // token is citizen_id
                if (!isPositiveNumber(token) || strlen(token) > MAX_ID_DIGITS)
                {
                    printf("Invalid Citizen ID deteted. Make sure it is an up-to %d digits number.\n",
                                    MAX_ID_DIGITS);
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
                // Invalid Age argument
                printf("Invalid Citizen Age deteted. Make sure it is a number 0-120.\nRejecting command.");
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
    // Checking how many arguments have been read
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

/**
 * @brief Parses the string stored in strtok buffer into /vaccineStatus
 * command arguments, and stores the argument values in the specified variables.
 * 
 * @param citizen_id The Citizen ID will be stored here.
 * @param virus_name The Virus Name will be stored here.
 * 
 * @returns TRUE if parsing was successful, FALSE if an error/bad argument was detected.
 */
bool vaccineStatusParse(int &citizen_id, char *&virus_name)
{
    char *arg;              // This is used to obtain the token returned by strtok
    virus_name = NULL;      // Initializing pointer argument to NULL

    // Try to obtain first argument
    arg = strtok(NULL, " ");
    if (arg == NULL)
    {
        printf("Less than expected arguments found. Rejecting command.\n");
        return false;
    }
    if ( isPositiveNumber(arg) && (strlen(arg) <= MAX_ID_DIGITS) && (citizen_id = atoi(arg)) >= 0 )
    // The argument is a valid ID
    {
        // Try to obtain a second argument
        arg = strtok(NULL, " ");
        if (arg != NULL)
        // There is a second argument, which is a Virus Name
        {
            virus_name = new char[strlen(arg) + 1];
            strcpy(virus_name, arg);
            // See if there are other arguments
            arg = strtok(NULL, " ");
            if (arg != NULL)
            // There are, which is unexpected
            {
                printf("More than expected arguments found. Rejecting command.\n");
                return false;
            }
        }  
        return true;
    }
    printf("Invalid Citizen ID deteted. Make sure it is an up-to 4 digits number.\n");
    return false; 
}

/**
 * @brief Parses the string stored in strtok buffer into /vaccineStatusBloom
 * command arguments, and stores the argument values in the specified variables.
 * 
 * @param citizen_id The Citizen ID will be stored here.
 * @param virus_name The Virus Name will be stored here.
 * 
 * @returns TRUE if parsing was successful, FALSE if an error/bad argument was detected.
 */
bool vaccineStatusBloomParse(int &citizen_id, char *&virus_name)
{
    char *arg;                  // This is used to obtain the token returned by strtok
    virus_name = NULL;          // Initializing pointer argument to NULL

    // Try to obtain first argument
    arg = strtok(NULL, " ");
    if (arg == NULL)
    {
        printf("Less than expected arguments found. Rejecting command.\n");
        return false;
    }
    if ( isPositiveNumber(arg) && (strlen(arg) <= MAX_ID_DIGITS) && (citizen_id = atoi(arg)) >= 0 )
    // The argument is a valid ID
    {
        // Try to obtain a second argument
        arg = strtok(NULL, " ");
        if (arg != NULL)
        {
            // Obtained, so store it
            virus_name = new char[strlen(arg) + 1];
            strcpy(virus_name, arg);
            // See if there are other arguments
            arg = strtok(NULL, " ");
            if (arg != NULL)
            // There are, which is unexpected
            {
                printf("More than expected arguments found. Rejecting command.\n");
                return false;
            }
            return true;
        }
        // Second argument not found, which is not acceptable.
        printf("Expected a virus name. Rejecting command.\n"); 
        return false;
    }
    printf("Invalid Citizen ID deteted. Make sure it is an up-to 4 digits number.\n");
    return false; 
}

/**
 * @brief Parses the string stored in strtok buffer into /listNonVaccinated
 * command arguments, and stores the argument values in the specified variables.
 * 
 * @param virus_name The Virus Name will be stored here.
 * 
 * @returns TRUE if parsing was successful, FALSE if an error/bad argument was detected.
 */
bool listNonVaccinatedParse(char *&virus_name)
{
    char *arg;                  // strtok tokens are stored here
    virus_name = NULL;          // Initializing pointer argument to NULL

    // Try to obtain the argument
    arg = strtok(NULL, " ");
    if (arg == NULL)
    // Could not obtain any arguments
    {
        printf("Expected a virus name. Rejecting command.\n");
        return false;
    }
    // Stored the Virus Name
    virus_name = new char[strlen(arg) + 1];
    strcpy(virus_name, arg);

    // See if there are other given arguments
    arg = strtok(NULL, " ");
    if (arg != NULL)
    // There are, which is unexpected.
    {
        printf("More than expected arguments found. Rejecting command.\n");
        return false;
    }
    return true;
}

/**
 * @brief Parses the string stored in strtok buffer into /populationStatus
 * command arguments, and stores the argument values in the specified variables.
 * 
 * @param country_name The citizen Country name will be stored here.
 * @param virus_name The Virus Name will be stored here.
 * @param start The First Date given by the user (if given) will be stored here. If a Date was not given,
 * a null Date will be stored.
 * @param end The Second Date given by the user (if given) will be stored here. If a Date was not given,
 * a null Date will be stored.
 * 
 * @returns TRUE if parsing was successful, FALSE if an error/bad argument was detected.
 */
bool populationStatusParse(char *&country_name, char *&virus_name, Date &start, Date &end)
{
    char **args = new char*[4];     // The string arguments will be stored here initially

    short int curr_arg = 0;         // This indicates which argument is examined
    char *token;                    // This is used to obtain the token returned by strtok

    // Initializing given arguments to NULL
    start.set(0, 0, 0);
    end.set(0, 0, 0);
    country_name = NULL;
    virus_name = NULL;

    // Get all the user arguments
    while ( (token = strtok(NULL, " "))!= NULL && curr_arg < 4)
    {
        args[curr_arg] = token;
        curr_arg++;
    }
    if (token != NULL)
    // Reject command if arguments were more than expected
    {
        printf("More than expected arguments have been detected. Rejecting command.\n");
        delete[] args;
        return false;
    }
    switch (curr_arg)
    // Exam the number of given arguments, and parse accordingly
    {
        case 0:
            // No arguments were given
            printf("Less than expected arguments have been detected. Rejecting command.\n");
            delete[] args;
            return false;
        case 1:
            // 1 argument (Virus Name) was given
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
            // This point should never be reached, but
            // if it does, something really bad has happened :(
            delete[] args;
            return false;
            break;
    }
}
