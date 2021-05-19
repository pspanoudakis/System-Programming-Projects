## Project 2 in UNIX System Programming Course
## Pavlos Spanoudakis (sdi1800184)
## Documentation & Additional comments
***

### Project Structure
- `app` directory: Source & Header files used by the Main app.
    - `parent_monitor.cpp`: travelMonitor client program.
    - `parent_monitor_utils.cpp` & `parent_monitor_utils.hpp`: App Classes, several routines used for Command Execution.
    - `monitor.cpp`: monitor client program.
    - `app_utils.cpp` & `app_utils.hpp`: App Classes, several routines used for Command Execution.
    - `parse_utils.cpp` & `parse_utils.hpp`: Routines used for Input parsing.
    - `pipe_msg.cpp` & `pipe_msg.hpp`: Routines used for fifo pipe reading/writing.
- `include` directory: Header files for ADT's used by the app.
- `lib` directory: Source files for ADT's used by the app.
- `build` directory: Used during app build for objective files.
- `Makefile`
- `create_infiles.sh` script to generate Country directories.

### Compiling, Executing & Using the app
**make**, **g++** and **openssl** are required (all installed in DIT workstations)

In the project root, run `make` and after the build is done,
run `./travelMonitor -m <numMonitors> -b <bufferSize> -s <sizeOfBloom> -i <inputDirectory>`

When done, run `make clean` to clean up objective files & executable.

### App Classes & Structures
- **Parent Montor**:
    - `MonitorInfo`: Used by the Parent process to store information about each child Monitor (pipe paths, process id,
                     directory paths)
    - `VirusRequests`: Used to store Travel Requests, sorted by date in a Red-Black Tree, about a specific virus.
    - `CountryMonitor`: Stores the monitor that has access to a specific Country directory.
    - `TravelRequest`: Represents a Travel Request, which is either accepted or rejected and has a specific date.
    - `VirusFilter`: Used by the Parent Process to store the bloom filter regarding a specific virus.
- **Monitor**:
    - `CitizenRecord`: Contains Citizen information (ID, age, Name, Country)
    - `VaccinationRecord`: Contains Vaccination Record information (Citizen, Virus, vaccinated/non-vaccinated, Date)
    - `VirusRecords`: Contains Skip Lists & Bloom Filter for a specific Virus. 
                      The Skip Lists contain pointers to Vaccination Records.
    - `VirusCountryStatus`: It is associated with a specific Country and a specific Virus.
                            It contains a Red-Black Tree with all the Vaccination Records marked with "YES"
                            (connected with Citizens of this Country and this Virus),
                            sorted by Date (to speed up Date-related queries).
                            It also contains total population and age group counters for the Vaccination Records
                            (**both** YES/NO) associated with this Virus and Citizens of this Country.
    - `CountryStatus`: Contains the corresponding Country name, as well as
                       a List of `VirusCountryStatus` objects.
    - `DirectoryInfo`: Used by the child Monitors to handle Directories. Stores the corresponding country directory path,
                       as well as a list with the files in the directory.

The Monitor app handles:
- A Hash Table containing `CitizenRecord` objects.
- A Linked List containing `VirusRecords` objects.
- A Linked List containing `CountryStatus` objects.

The `CountryStatus` and `VirusRecords` objects are each associated to a Country and Virus respectively,
so they are stored in Lists, because the number of Countries and of Viruses is
quite limited.

### ADT's used by the App
- **Skip List**: It is implemented using an array of pointers to Skip List Nodes, which are the head nodes of each layer.
  Every Skip List Node contains an array of pointers to Skip List Nodes, 1 for each layer where the Node is present.
- **Bloom Filter**: It is implemented using an array of `char` elements (since they have 1 byte size by standrard).
  The bits are modified and checked using proper bitwise shifts. When data is about to be "inserted", or is to
  be checked whether it has potentially been inserted, the hash functions given by the instructors are used
  (see `lib/hash_functions.cpp`), to figure out which bits need to be set to 1.
- **Single-Linked List**: A simple single-linked List implementation, with the addition of an iterator class.
- **Hash Table**: Implemented with Seperate Chaining, using an fixed-size array of Linked Lists.
 `SHA1` from `openssl` is used for element hashing.
- **Red-Black Tree**: A typical Red-Black Tree implementation. The internal fields of Red-Black Tree and Node classes
  are public in order to be visible from App routines (used in Date-related queries).

The only ADT that includes element deletion is the Skip List, since deletion is not needed by the App for the other ADT's.

### Simplifications, Minor Design Choices & Details
- The app "treats" file records and records inserted by the user in the **exact same way**:

  If a "NO" record (about a Virus vaccination) has been inserted for a certain Citizen and
  a "YES" record for the same Citizen and Virus is inserted later, the existing record will be **modified**
  (removal from "non-vaccinated" Skip List, insertion to "vaccinated" Skip List & Bloom FIlter, etc.),
  whether it was inserted by the user or the input file. Of course the opposite is **not allowed**
  (a "YES" record being modified to a "NO" record). 
- Citizen, Country and Virus Names are **case sensitive**
  (e.g. "Greece", "greece" and "GREECE" are considered 3 different countries).
- In `/vaccineStatus` command, the app will search the specified Citizen in all "vaccinated" Skip Lists,
  and will print whether the citizen was found or not (if not found, the citizen will appear as non-vaccinated whether
  there is a Record in "non-vaccinated" Skip List or not).
- The Citizen ID's allowed by the app can have up to a certain number of digits,
  which is specified by the `MAX_ID_DIGITS` macro in `app/app_utils.hpp`. It is set to `5` by default.
- In `app/main.cpp`, macros `HASHTABLE_BUCKETS` (for the Citizen Records Hash Table) and `MAX_BLOOM_SIZE`
  (for maximum Bloom Filter size) are defined and can be modified. During testing, it was observed that
  for Bloom Filter sizes > 1000000, bad_alloc exception was thrown at some point (combined with large inputFile size).

### Performance & Resource Handling
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
