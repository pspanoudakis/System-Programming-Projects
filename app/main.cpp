/**
 * File: main.cpp
 * Main client program
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "../include/linked_list.hpp"
#include "../include/hash_table.hpp"
#include "../include/utils.hpp"
#include "app_utils.hpp"
#include "parse_utils.hpp"

#define HASHTABLE_BUCKETS 10000         // Number of buckets for the Citizen Hash Table
#define MAX_BLOOM_SIZE 1000000          // Maximum Bloom Filter size allowed

/**
 * Parses and executes the specified command properly. Displays error messages if needed.
 */
void parseExecuteCommand(char *command, HashTable *citizens, LinkedList *countries, LinkedList *viruses, unsigned long &bloom_size)
{
    // Variables used for storing command parameters
    int citizen_id, age;
    char *citizen_name, *country_name, *virus_name;
    bool vaccinated;
    Date date, start, end;

    char *token = strtok(command, " ");
    if (token != NULL)
    // Recognizing the main command
    {
        // Call the corresponding parse routine and execution routine, if there are no errors.
        // Release any temporarily allocated memory at the end.
        if (strcmp(token, "/insertCitizenRecord") == 0)
        {
            if (insertCitizenRecordParse(citizen_id, citizen_name, country_name, age, virus_name,
                                                     vaccinated, date, stdout))
            {
                insertVaccinationRecord(citizen_id, citizen_name, country_name, age, virus_name, vaccinated, date,
                                    countries, viruses, citizens, bloom_size, stdout);
            }
            delete[] citizen_name;
            delete[] country_name;
            delete[] virus_name;
        }
        else if (strcmp(token, "/vaccinateNow") == 0)
        {
            if (vaccinateNowParse(citizen_id, citizen_name, country_name, age, virus_name))
            {
                date.setToCurrentDate();
                insertVaccinationRecord(citizen_id, citizen_name, country_name, age, virus_name, true, date,
                                    countries, viruses, citizens, bloom_size, stdout);
            }
            delete[] citizen_name;
            delete[] country_name;
            delete[] virus_name;
        }
        else if (strcmp(token, "/vaccineStatusBloom") == 0)
        {
            if (vaccineStatusBloomParse(citizen_id, virus_name))
            {
                vaccineStatusBloom(citizen_id, viruses, virus_name);
            }
            delete[] virus_name;
        }
        else if (strcmp(token, "/vaccineStatus") == 0)
        {
            if (vaccineStatusParse(citizen_id, virus_name))
            {
                if (virus_name == NULL)
                {
                    vaccineStatus(citizen_id, viruses);
                }
                else
                {
                    vaccineStatus(citizen_id, viruses, virus_name);
                }
            }
            delete[] virus_name;
        }
        else if (strcmp(token, "/populationStatus") == 0 )
        {
            if (populationStatusParse(country_name, virus_name, start, end))
            {
                if (country_name == NULL)
                {
                    populationStatus(virus_name, countries, start, end);
                }
                else
                {
                    populationStatus(virus_name, country_name, countries, start, end);
                }
            }
            delete[] country_name;
            delete[] virus_name;
        }
        else if (strcmp(token, "/popStatusByAge") == 0 )
        {
            if (populationStatusParse(country_name, virus_name, start, end))
            {
                if (country_name == NULL)
                {
                    popStatusByAge(virus_name, countries, start, end);
                }
                else
                {
                    popStatusByAge(virus_name, country_name, countries, start, end);
                }
            }
            delete[] country_name;
            delete[] virus_name;
        }
        else if (strcmp(token, "/list-nonVaccinated-Persons") == 0)
        {
            if (listNonVaccinatedParse(virus_name))
            {
                listNonVaccinatedPersons(virus_name, viruses);
            }
            delete[] virus_name;
        }
        else
        {
            printf("Unknown command.\n");
        }
    }
}

/**
 * Checks and stores the program agruments properly.
 * @returns TRUE if the arguments are valid, FALSE otherwise.
 */
bool checkParseArgs(int argc, char const *argv[], FILE *&input_file, unsigned long &bloom_size)
{
    if (argc != 5)
    {
        perror("Insufficient number of arguments given.\n");
        return false;
    }
    if ( strcmp(argv[1], "-c") != 0 )
    {
        if ( strcmp(argv[1], "-b") != 0 )
        {
            perror("Invalid argument detected.\n");
            return false;
        }
        // argv1 is -b
        if ( strcmp(argv[3], "-c") != 0 )
        {
            perror("Invalid argument detected.\n");
            return false;
        }
        // argv3 is -c
        if ( !isPositiveNumber(argv[2]) )
        {
            perror("Bloom Size must be a positive integer.\n");
            return false;
        }
        bloom_size = atol(argv[2]);
        if (bloom_size > MAX_BLOOM_SIZE)
        {
            printf("Bloom size too large. Using max size %d.\n", MAX_BLOOM_SIZE);
            bloom_size = MAX_BLOOM_SIZE;
        }
        input_file = fopen(argv[4], "r");
    }
    else
    // argv1 is -c
    {
        if ( strcmp(argv[3], "-b") != 0 )
        {
            perror("Invalid argument detected.\n");
            return false;
        }
        // argv3 is -b
        if ( !isPositiveNumber(argv[4]) )
        {
            perror("Bloom Size must be a positive integer.\n");
            return false;
        }
        bloom_size = atol(argv[4]);
        if (bloom_size > MAX_BLOOM_SIZE)
        {
            printf("Bloom size too large. Using max size %d.\n", MAX_BLOOM_SIZE);
            bloom_size = MAX_BLOOM_SIZE;
        }
        input_file = fopen(argv[2], "r");
    }    
    return true;
}

int main(int argc, char const *argv[])
{
    char *line_buf, *buf_copy, *temp;       // Used for storing line strings from input file
    FILE *input_file;
    unsigned long bloom_size;

    // Checking arguments and storing them.
    if ( !checkParseArgs(argc, argv, input_file, bloom_size) )
    // Exit if given arguments are invalid
    {
        printf("Execution format: ./vaccineMonitor -c <citizenRecordsFile> -b <bloomSize>\n");
        return 1;
    }
     
    if (input_file == NULL)
    // Exit if opening the input file failed.
    {
        printf("Unable to open the specified input file.\n");
        return 1;
    }

    // Structures used for information storing.
    HashTable *citizens = new HashTable(HASHTABLE_BUCKETS, delete_object<CitizenRecord>, citizenHashObject);
    LinkedList *countries = new LinkedList(delete_object<CountryStatus>);
    LinkedList *viruses = new LinkedList(delete_object<VirusRecords>);
    // Used for command argument storing.
    int citizen_id, age;
    char *citizen_name, *country_name, *virus_name;
    bool vaccinated;
    Date date;

    srand(time(NULL));

    // Starting input file processing
    printf("Processing input from file...\n");
    // Read the file line by line
    while( (line_buf = fgetline(input_file)) != NULL)
    {
        buf_copy = new char[strlen(line_buf) + 1];
        temp = new char[strlen(line_buf) + 3];
        sprintf(temp, "~ %s", line_buf);            // just a "hack" so that the parsing function
                                                    // can get all the line tokens using strtok.
        strcpy(buf_copy, line_buf);
        
        strtok(temp, " ");       

        // Parse the line
        if (insertCitizenRecordParse(citizen_id, citizen_name, country_name, age, virus_name, vaccinated, date, NULL))
        // If parsing was successful, try to insert the Record.
        {
            insertVaccinationRecord(citizen_id, citizen_name, country_name, age, virus_name, vaccinated, date,
                                    countries, viruses, citizens, bloom_size, NULL);
            delete[] citizen_name;
            delete[] country_name;
            delete[] virus_name;
        }
        else
        // Parsing error
        {
            printf("ERROR in record: %s\n", buf_copy);
        }
        delete[] temp;
        delete[] buf_copy;
        free(line_buf);
    }
    // Done with file at this point.
    fclose(input_file);

    // Entering command line mode
    printf("Now in command line mode.\n");
    bool terminate = false;
    while( !terminate )
    {
        printf("------------------------------------------\n");
        // Read line from stdin
        line_buf = fgetline(stdin);
        if (line_buf == NULL)
        {
            continue;
        }
        if (strcmp(line_buf, "/exit") == 0)
        // Exit if asked by the user
        {
            terminate = true;
            continue;
        }
        // Otherwise, try to parse the line into a command and execute it.
        parseExecuteCommand(line_buf, citizens, countries, viruses, bloom_size);
        free(line_buf);
    }
    free(line_buf);

    // Release resources and exit.
    delete countries;
    delete viruses;
    delete citizens;
    return 0;
}
