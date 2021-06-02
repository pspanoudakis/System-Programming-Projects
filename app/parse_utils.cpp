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

bool childCheckparseArgs(int argc, char const *argv[], uint16_t &port,
               unsigned int &num_threads, unsigned int &buffer_size, unsigned int &cyclic_buffer_size,
               DirectoryInfo **&directories, unsigned long &bloom_size, unsigned int &num_dirs)
{
    if (argc < 11)
    {
        fprintf(stderr, "Invalid number of arguments given.\n");
        fprintf(stderr, "Usage: ./monitorServer -p port -t numThreads -b socketBufferSize \
        -c cyclicBufferSize -s sizeOfBloom <path1> ... <pathN>\n");
        return false;
    }

    // Flags to make sure no arguments are given more than once
    bool got_port = false, got_num_threads = false, got_socket_buf_size = false;
    bool got_cyclic_buf_size = false, got_bloom_size = false;
    for (int i = 1; i < 11; i+=2)
    {
        if ( strcmp(argv[i], "-p") == 0 )
        {
            if (got_port) {
                fprintf(stderr, "Duplicate port number argument detected.\n");
                return false; 
            }
            port = atoi(argv[i + 1]);
            got_port = true;
        }
        else if ( strcmp(argv[i], "-t") == 0 )
        {
            if (got_num_threads) {
                fprintf(stderr, "Duplicate numThreads argument detected.\n");
                return false;
            }
            num_threads = atoi(argv[i + 1]);
            got_num_threads = true;
        }
        else if ( strcmp(argv[i], "-b") == 0 )
        {
            if (got_socket_buf_size) { 
                fprintf(stderr, "Duplicate socketBufferSize argument detected.\n");
                return false; 
            }
            int temp = atoi(argv[i + 1]);
            if (temp > 0 && temp <= MAX_BUFFER_SIZE)
            {
                buffer_size = temp;
                got_socket_buf_size = true;
            }
            else
            {
                fprintf(stderr, "Invalid socketBufferSize argument. Make sure it is a positive integer up to %d", MAX_BUFFER_SIZE);
                return false;
            }
        }
        else if ( strcmp(argv[i], "-c") == 0 )
        {
            if (got_cyclic_buf_size) { 
                fprintf(stderr, "Duplicate cyclicBufferSize argument detected.\n");
                return false; 
            }
            int temp = atoi(argv[i + 1]);
            if (temp > 0 && temp <= MAX_BUFFER_SIZE)
            {
                cyclic_buffer_size = temp;
                got_cyclic_buf_size = true;
            }
            else
            {
                fprintf(stderr, "Invalid cyclicBufferSize argument. Make sure it is a positive integer up to %d", MAX_BUFFER_SIZE);
                return false;
            }
        }
        else if ( strcmp(argv[i], "-s") == 0 )
        {
            if (got_bloom_size) { 
                fprintf(stderr, "Duplicate sizeOfBloom argument detected.\n");
                return false;
            }
            long temp = atol(argv[i + 1]);
            if (temp > 0 && temp <= MAX_BLOOM_SIZE)
            {
                bloom_size = temp;
                got_bloom_size = true;
            }
            else
            {
                fprintf(stderr, "Invalid sizeOfBloom argument. Make sure it is a positive integer up to %d", MAX_BLOOM_SIZE);
                return false;
            }
        }
        else
        {
            fprintf(stderr, "Invalid argument detected.\n");
            return false;
        }
    }

    num_dirs = argc - 11;
    directories = new DirectoryInfo*[num_dirs];
    for (unsigned int i = 0; i < num_dirs; i++)
    {
        directories[i] = new DirectoryInfo(argv[11 + i]);
        directories[i]->addContents();
    }
    return true;
}

/**
 * Checks and stores the program arguments properly.
 * @returns TRUE if the arguments are valid, FALSE otherwise.
 */
bool parentCheckParseArgs(int argc, char const *argv[], char *&directory_path, unsigned int &num_monitors,
                    unsigned long &bloom_size, unsigned int &buffer_size, unsigned int &cyclic_buffer_size,
                    unsigned int &num_threads)
{
    directory_path = NULL;
    if (argc != 9)
    {
        perror("Insufficient/Unexpected number of arguments given.\n");
        printf("Usage: ./travelMonitor -m numMonitors -b bufferSize -s sizeOfBloom -i input_dir\n");
        return false;
    }

    // Flags to make sure no arguments are given more than once
    bool got_num_monitors = false;
    bool got_buffer_size = false;
    bool got_cyclic_buffer_size = false;
    bool got_bloom_size = false;
    bool got_input_dir = false;
    bool got_num_threads = false;

    for (int i = 1; i < 8; i+=2)
    {
        if ( strcmp(argv[i], "-m") == 0 )
        {
            if (got_num_monitors) {
                fprintf(stderr, "Duplicate numMonitors argument detected.\n");
                return false; 
            }
            int temp = atoi(argv[i + 1]);
            if (temp > 0 && temp <= MAX_MONITORS)
            {
                num_monitors = temp;
                got_num_monitors = true;
            }
            else
            {
                fprintf(stderr, "Invalid numMonitors argument. Make sure it is a positive integer up to %d.\n", MAX_MONITORS);
                return false;
            }
        }
        else if ( strcmp(argv[i], "-b") == 0 )
        {
            if (got_buffer_size) { 
                fprintf(stderr, "Duplicate bufferSize argument detected.\n");
                return false; 
            }
            int temp = atoi(argv[i + 1]);
            if (temp > 0 && temp <= MAX_BUFFER_SIZE)
            {
                buffer_size = temp;
                got_buffer_size = true;
            }
            else
            {
                fprintf(stderr, "Invalid bufferSize argument. Make sure it is a positive integer up to %d", MAX_BUFFER_SIZE);
                return false;
            }
        }
        else if ( strcmp(argv[i], "-c") == 0 )
        {
            if (got_cyclic_buffer_size) { 
                fprintf(stderr, "Duplicate cyclicBufferSize argument detected.\n");
                return false; 
            }
            int temp = atoi(argv[i + 1]);
            if (temp > 0 && temp <= MAX_BUFFER_SIZE)
            {
                cyclic_buffer_size = temp;
                got_cyclic_buffer_size = true;
            }
            else
            {
                fprintf(stderr, "Invalid cyclicBufferSize argument. Make sure it is a positive integer up to %d", MAX_BUFFER_SIZE);
                return false;
            }
        }
        else if ( strcmp(argv[i], "-s") == 0 )
        {
            if (got_bloom_size) { 
                fprintf(stderr, "Duplicate sizeOfBloom argument detected.\n");
                return false;
            }
            long temp = atol(argv[i + 1]);
            if (temp > 0 && temp <= MAX_BLOOM_SIZE)
            {
                bloom_size = temp;
                got_bloom_size = true;
            }
            else
            {
                fprintf(stderr, "Invalid sizeOfBloom argument. Make sure it is a positive integer up to %d", MAX_BLOOM_SIZE);
                return false;
            }
        }
        else if ( strcmp(argv[i], "-i") == 0 )
        {
            if (got_input_dir) {
                fprintf(stderr, "Duplicate input_dir argument detected.\n");
                return false;
            }
            directory_path = copyString(argv[i + 1]);
            got_input_dir = true;
        }
        else if ( strcmp(argv[i], "-t") == 0 )
        {
            if (got_num_threads) { 
                fprintf(stderr, "Duplicate numThreads argument detected.\n");
                return false;
            }
            long temp = atol(argv[i + 1]);
            if (temp > 0 && temp <= MAX_THREADS)
            {
                num_threads = temp;
                got_num_threads = true;
            }
            else
            {
                fprintf(stderr, "Invalid numThreads argument. Make sure it is a positive integer up to %d", MAX_THREADS);
                return false;
            }
        }
        else
        {
            fprintf(stderr, "Invalid argument detected.\n");
            return false;
        }
    }
    return true;
}

/**
 * Checks if the /addVaccinationRecords command was valid and
 * stores the country name in the given variable.
 */
bool addVaccinationRecordsParse(char *&country_name)
{
    char *arg;                  // strtok tokens are stored here
    country_name = NULL;          // Initializing pointer argument to NULL

    // Try to obtain the argument
    arg = strtok(NULL, " ");
    if (arg == NULL)
    // Could not obtain any arguments
    {
        printf("Expected a country name. Rejecting command.\n");
        return false;
    }
    // Stored the Virus Name
    country_name = new char[strlen(arg) + 1];
    strcpy(country_name, arg);

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
 * Checks if the /searchVaccinationStatus command was valid and
 * stores the citizen ID in the given variable.
 */
bool searchVaccinationStatusParse(unsigned int &citizen_id)
{
    char *arg;
    // Try to obtain the argument
    arg = strtok(NULL, " ");
    if (arg == NULL)
    // Could not obtain any arguments
    {
        printf("Expected a citizen ID. Rejecting command.\n");
        return false;
    }
    // Stored the Virus Name
    if (!isPositiveNumber(arg) || strlen(arg) > MAX_ID_DIGITS)
    {
        printf("Invalid Citizen ID deteted. Make sure it is an up-to %d digits number.\n", MAX_ID_DIGITS);
        return false;
    }
    citizen_id = atoi(arg);
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
 * Checks if the /travelRequest command was valid and
 * stores the user arguments in the given variables.
 */
bool travelRequestParse(unsigned int &citizen_id, Date &date, char *&countryFrom, char *&countryTo, char *&virus_name)
{
    short int curr_arg = 0;     // This indicates which argument is examined
    char *token;                // This is used to obtain the token returned by strtok
    
    // Initializing the given arguments to NULL
    countryFrom = NULL;
    countryTo = NULL;
    virus_name = NULL;
    date.set(0, 0, 0);

    // Loop until strtok no longer gives other tokens or more than expected arguments detected
    while ( (token = strtok(NULL, " "))!= NULL && curr_arg < 5 )
    {
        switch (curr_arg)
        {
            case 0:
                // token is citizen_id
                if (!isPositiveNumber(token) || strlen(token) > MAX_ID_DIGITS)
                {
                    printf("Invalid Citizen ID deteted. Make sure it is an up-to %d digits number.\n",
                            MAX_ID_DIGITS);
                    return false;
                }
                citizen_id = atoi(token);
                break;
            case 1:
                // token is date
                if (!parseDateString(token, date))
                // Could not parse Date token successfully 
                {
                    printf("Invalid date argument detected. Rejecting command.\n");
                    return false;
                }
                break;
            case 2:
                // token is countryFrom
                countryFrom = new char [strlen(token)+1];
                strcpy(countryFrom, token);
                break;
            case 3:
                // token is countryTo
                countryTo = new char [strlen(token)+1];
                strcpy(countryTo, token);
                break;
            case 4:
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
    if (curr_arg == 5)
    {
        return true;
    }
    else if (curr_arg < 5)
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
 * Checks if the /travelStats command was valid and
 * stores the user arguments in the given variables.
 */
bool travelStatsParse(char *&virus_name, Date &start, Date &end, char *&country_name)
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
        case 1:
        case 2:
            printf("Less than expected arguments have been detected. Rejecting command.\n");
            delete[] args;
            return false;
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
            if (compareDates(start, end) > 0)
            {
                printf("The first Date cannot be greater than the second one. Rejecting command.\n");
                return false;
            }
            return true;
        case 4:
            // args are: virus date1 date2 country
            virus_name = new char[strlen(args[0])+1];
            strcpy(virus_name, args[0]);
            country_name = new char[strlen(args[3])+1];
            strcpy(country_name, args[3]);

            if ( !(parseDateString(args[1], start) && parseDateString(args[2], end)) )
            {
                delete[] args;
                printf("Invalid date argument detected. Rejecting command.\n");
                return false;
            }
            delete[] args;
            if (compareDates(start, end) > 0)
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
