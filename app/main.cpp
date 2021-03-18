#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "../include/linked_list.hpp"
#include "../include/hash_table.hpp"
#include "../include/utils.hpp"
#include "app_utils.hpp"
#include "parse_utils.hpp"

#define HASHTABLE_BUCKETS 10000
#define MAX_BLOOM_SIZE 1000000

void parseExecuteCommand(char *command, HashTable *citizens, LinkedList *countries, LinkedList *viruses, unsigned long &bloom_size)
{
    int citizen_id, age;
    char *citizen_name, *country_name, *virus_name;
    bool vaccinated;
    Date date, start, end;

    char *token = strtok(command, " ");
    if (token != NULL)
    {
        if (strcmp(token, "/insertCitizenRecord") == 0)
        {
            if (insertCitizenRecordParse(citizen_id, citizen_name, country_name, age, virus_name,
                                                     vaccinated, date, stdout))
            {
                insertVaccinationRecord(citizen_id, citizen_name, country_name, age, virus_name, vaccinated, date,
                                    countries, viruses, citizens, bloom_size, stdout);
                delete[] citizen_name;
                delete[] country_name;
                delete[] virus_name;
            }
        }
        else if (strcmp(token, "/vaccinateNow") == 0)
        {
            if (vaccinateNowParse(citizen_id, citizen_name, country_name, age, virus_name))
            {
                date.setToCurrentDate();
                insertVaccinationRecord(citizen_id, citizen_name, country_name, age, virus_name, true, date,
                                    countries, viruses, citizens, bloom_size, stdout);
                delete[] citizen_name;
                delete[] country_name;
                delete[] virus_name;
            }
        }
        else if (strcmp(token, "/vaccineStatusBloom") == 0)
        {
            if (vaccineStatusBloomParse(citizen_id, virus_name))
            {
                vaccineStatusBloom(citizen_id, viruses, virus_name);
                delete[] virus_name;
            }
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
                
                delete[] virus_name;
            }
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
                delete[] country_name;
                delete[] virus_name;
            }
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
                delete[] country_name;
                delete[] virus_name;
            }
        }
        else if (strcmp(token, "/list-nonVaccinated-Persons") == 0)
        {
            if (listNonVaccinatedParse(virus_name))
            {
                listNonVaccinatedPersons(virus_name, viruses);
                delete[] virus_name;
            }
        }
        else
        {
            printf("Unknown command.\n");
        }
    }
}

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
    char *line_buf, *buf_copy, *temp;
    FILE *input_file;
    unsigned long bloom_size;

    if ( !checkParseArgs(argc, argv, input_file, bloom_size) )
    {
        printf("Execution format: ./vaccineMonitor -c <citizenRecordsFile> -b <bloomSize>\n");
        return 1;
    }
     
    if (input_file == NULL)
    {
        printf("Unable to open the specified input file.\n");
        return 1;
    }

    FILE *dump_output = fopen("/dev/null", "w");
    if (dump_output == NULL)
    {
        printf("Cannot open /dev/null.\n");
        return 1;
    }

    HashTable *citizens = new HashTable(HASHTABLE_BUCKETS, delete_object<CitizenRecord>, citizenHashObject);
    LinkedList *countries = new LinkedList(delete_object<CountryStatus>);
    LinkedList *viruses = new LinkedList(delete_object<VirusRecords>);
    int citizen_id, age;
    char *citizen_name, *country_name, *virus_name;
    bool vaccinated;
    Date date;

    srand(time(NULL));

    printf("Processing input from file...\n");
    while( (line_buf = fgetline(input_file)) != NULL)
    {
        buf_copy = new char[strlen(line_buf) + 1];
        temp = new char[strlen(line_buf) + 3];
        sprintf(temp, "~ %s", line_buf);
        strcpy(buf_copy, line_buf);
        
        strtok(temp, " ");       

        if (insertCitizenRecordParse(citizen_id, citizen_name, country_name, age, virus_name, vaccinated, date, dump_output))
        {
            insertVaccinationRecord(citizen_id, citizen_name, country_name, age, virus_name, vaccinated, date,
                                    countries, viruses, citizens, bloom_size, dump_output);
            delete[] citizen_name;
            delete[] country_name;
            delete[] virus_name;
        }
        else
        {
            printf("ERROR in record: %s\n", buf_copy);
        }
        delete[] temp;
        delete[] buf_copy;
        free(line_buf);
    }
    fclose(dump_output);
    fclose(input_file);
    printf("Now in command line mode.\n");
    bool terminate = false;
    while( !terminate )
    {
        printf("------------------------------------------\n");
        line_buf = fgetline(stdin);
        if (line_buf == NULL)
        {
            continue;
        }
        if (strcmp(line_buf, "/exit") == 0)
        {
            terminate = true;
            continue;
        }
        parseExecuteCommand(line_buf, citizens, countries, viruses, bloom_size);
        free(line_buf);
    }
    free(line_buf);

    delete countries;
    delete viruses;
    delete citizens;
    return 0;
}
