## Project 1 in UNIX System Programming Course
## Pavlos Spanoudakis (sdi1800184)
## Documentation & Additional comments
***

### Project Structure
- `app` directory: Source & Header files used by the Main app.
    - `main.cpp`: Main client program.
    - `app_utils.cpp` & `app_utils.hpp`: App Classes, several routines used for Command Execution.
    - `parse_utils.cpp` & `parse_utils.hpp`: Routines used for Input parsing.
- `include` directory: Header files for ADT's used by the app.
- `lib` directory: Source files for ADT's used by the app.
- `build` directory: Used during app build for objective files.
- `Makefile`
- `testFile.sh` script to generate input File.

### Compiling, Executing & Using the app
In the project root, run `make` and after the build is done,
run `./vaccineMonitor -c <citizenRecordsFile> -b <Bloom Size (in bytes)>`.

The app first starts inserting records from the input file. If a record
has a **syntax** error, a message will be displayed (the execution will continue).

When file processing is over, the app will inform the user, and will be ready to accept user commands.
After a command is given, the app will display an error message if the command was invalid,
or execute the command and print the requested information, or a successful execution message.

When done, run `make clean` to clean up objective files & executable.

### App Classes & Structures
- `CitizenRecord`: Contains Citizen information (ID, age, Name, Country)
- `VaccinationRecord`: Contains Vaccination Record information (Citizen, Virus, vaccinated/non-vaccinated, Date)
- `VirusRecords`: Contains Skip Lists & Bloom Filter for a specific Virus. 
                  The Skip Lists contain pointers to Vaccination Records.
- `VirusCountryStatus`: It is associated with a specific Country an a specific Virus.
                        It contains a Red-Black Tree with all the Vaccination Records marked with "YES"
                        (connected with Citizens of this Country and this Virus),
                         sorted by Date (to speed up Date-related queries).
- `CountryStatus`: Contains information about a Country (name, population counters), as well as
                   a List of `VirusCountryStatus` objects.

The app handles:
- A Hash Table containing `CitizenRecord` objects.
- A Linked List containing `VirusRecords` objects.
- A Linked List containing `CountryStatus` objects.

The `CountryStatus` and `VirusRecords` objects are each associated to a Country and Virus respectively,
so they are stored in Lists, because the number of Countries and of Viruses is
quite limited.

### ADT's used by the App
- Skip List:
- Bloom Filter: It is implemented using an array of `char` elements (since they have 1 byte size by standrard).
  The bits are modified and checked using proper bitwise shifts.
- Single-Linked List:
- Hash Table:

### Simplifications, Minor Design Choices & Details
- The app "treats" file records and records inserted by the user in the **exact same way**:

  If a "NO" record (about a Virus vaccination) has been inserted for a certain Citizen and
  a "YES" record for the same Citizen and Virus is inserted later, the existing record will be **modified**
  (removal from "non-vaccinated" Skip List, insertion to "vaccinated" Skip List & Bloom FIlter, etc.),
  whether it was inserted by the user or the input file. Of course the opposite is **not allowed**
  (a "YES" record being modified to a "NO" record). 
- Citizen, Country and Virus Names are **case sensitive**
  (e.g. "Greece", "greece" and "GREECE" are considered 3 different countries).
- In each Country structure, 5 population counters are stored:
    - Total Population
    - Population aged Under-20
    - Population aged 20-40
    - Population aged 40-60
    - Population aged 60+

    In `/populationStatus` and `/popStatusByAge` queries, the percentage denominator(s) is one of the above.
    Essentially, that means a Citizen without YES/NO record regarding a specific Virus, 
    is considered as **not vaccinated**.
- The Citizen ID's allowed by the app can have up to a certain number of digits,
  which is specified by the `MAX_ID_DIGITS` macro in `app/app_utils.hpp`. It is set to `5` by default.

### Performance & Resource Handling
- The app has been tested with various input file sizes (all generated using the shell script), up to 500K records,
and the maximum time needed for file processing was 90 secs.
- The app has been tested extensively with Valgrind and no leaks are reported in multiple scenarios.
  Note that that there is a reported "bug" in Valgrind used in certain DIT workstations, regarding C++ memory pooling,
  which causes some bytes to be reported as "still reachable" after the end of the execution of any C++ program.
  When testing with Valgrind in DIT Labs, apart from the above "leak", no other leaks were reported.
  You can read more here:
  - http://valgrind.org/docs/manual/faq.html#faq.reports
  - https://gcc.gnu.org/onlinedocs/libstdc++/faq.html#faq.memory_leaks

### Development & Testing
Developed & tested in WSL Ubuntu 20.04, using Visual Studio Code.
Successfully tested in DIT Lab Ubuntu 18.04 as well.
