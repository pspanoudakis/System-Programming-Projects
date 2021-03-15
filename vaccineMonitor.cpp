#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "include/linked_list.hpp"
#include "include/hash_table.hpp"
#include "include/utils.hpp"
#include "app/app_utils.hpp"
#include "app/parse_utils.hpp"

#define HASHTABLE_BUCKETS 50
#define BLOOM_BYTES 100

void parseExecuteCommand(char *command, HashTable *citizens, LinkedList *countries, LinkedList *viruses)
{
    int citizen_id, age;
    char *citizen_name, *country_name, *virus_name;
    bool vaccinated;
    Date date, start, end;
    bool correct_parse = false;

    char *token = strtok(command, " ");
    if (token != NULL)
    {
        if (strcmp(token, "/insertCitizenRecord") == 0)
        {
            correct_parse = insertCitizenRecordParse(citizen_id, citizen_name, country_name, age, virus_name,
                                                     vaccinated, date, stdout);
            if (correct_parse)
            {
                insertVaccinationRecord(citizen_id, citizen_name, country_name, age, virus_name, vaccinated, date,
                                    countries, viruses, citizens, BLOOM_BYTES, stdout);
                delete[] citizen_name;
                delete[] country_name;
                delete[] virus_name;
            }
        }
        else if (strcmp(token, "/vaccinateNow") == 0)
        {
            correct_parse = vaccinateNowParse(citizen_id, citizen_name, country_name, age, virus_name);
            if (correct_parse)
            {
                delete[] citizen_name;
                delete[] country_name;
                delete[] virus_name;
            }
        }
        else if (strcmp(token, "/vaccineStatusBloom") == 0)
        {
            correct_parse = vaccineStatusBloomParse(citizen_id, virus_name);
            if (correct_parse)
            {
                delete[] virus_name;
            }
        }
        else if (strcmp(token, "/vaccineStatus") == 0)
        {
            correct_parse = vaccineStatusParse(citizen_id, virus_name);
            if (correct_parse)
            {
                delete[] virus_name;
            }
        }
        else if (strcmp(token, "/populationStatus") == 0 )
        {
            correct_parse = populationStatusParse(country_name, virus_name, start, end);
            if (correct_parse)
            {
                delete[] country_name;
                delete[] virus_name;
            }
        }
        else if (strcmp(token, "/popStatusByAge") == 0 )
        {
            correct_parse = populationStatusParse(country_name, virus_name, start, end);
            if (correct_parse)
            {
                delete[] country_name;
                delete[] virus_name;
            }
        }
        else if (strcmp(token, "/list-nonVaccinated-Persons") == 0)
        {
            correct_parse = listNonVaccinatedParse(virus_name);
            if (correct_parse)
            {
                delete[] virus_name;
            }
        }
    }
}

int main(void)
{
    char *line_buf, *buf_copy, *temp;
    HashTable *citizens = new HashTable(HASHTABLE_BUCKETS, delete_object<CitizenRecord>, citizenHashObject);
    LinkedList *countries = new LinkedList(delete_object<CountryStatus>);
    LinkedList *viruses = new LinkedList(delete_object<VirusRecords>);
    int citizen_id, age;
    char *citizen_name, *country_name, *virus_name;
    bool vaccinated;
    Date date;

    //FILE *input_file = fopen("", "r");
    FILE *input_file = stdin;
    //FILE *dump_output = fopen("dev/null", "w");
    FILE *dump_output = stdout;

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
                                    countries, viruses, citizens, BLOOM_BYTES, dump_output);
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
    //fclose(dump_output);
    //fclose(input_file);
    printf("Now in command line mode.\n");
    while( (line_buf = fgetline(stdin)) != NULL)
    {
        if (strcmp(line_buf, "/exit") == 0)
        {
            free(line_buf);
            break;
        }
        parseExecuteCommand(line_buf, citizens, countries, viruses);
        free(line_buf);
    }

    delete countries;
    delete viruses;
    delete citizens;
    return 0;
}
