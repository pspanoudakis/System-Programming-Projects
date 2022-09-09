## Project 2 in UNIX System Programming Course
## Pavlos Spanoudakis (sdi1800184)
## Documentation & Additional comments
***

### Project Structure
- `app` directory: Source & Header files used by the Main app.
    - `parent_monitor.cpp`: travelMonitor client program.
    - `parent_monitor_utils.cpp` & `parent_monitor_utils.hpp`: App Classes, several routines used the parent monitor.
    - `monitor.cpp`: child monitor client program.
    - `app_utils.cpp` & `app_utils.hpp`: App Classes, several routines used by the child monitors.
    - `parse_utils.cpp` & `parse_utils.hpp`: Routines used for Input parsing.
    - `pipe_msg.cpp` & `pipe_msg.hpp`: Routines used for fifo pipe reading/writing.
- `include` directory: Header files for ADT's used by the app.
- `lib` directory: Source files for ADT's used by the app.
- `build` directory: Used during app build for objective files.
- `log_files` directory: Used for storing fifo pipes during the execution. **It must not be removed**.
- `Makefile`
- `create_infiles.sh` script to generate Country directories.

### Compiling, Executing & Using the app
**make**, **g++** and **openssl** are required (all installed in DIT workstations)

In the project root, run `make` and after the build is done,
run `./travelMonitor -m <numMonitors> -b <bufferSize> -s <sizeOfBloom> -i <inputDirectory>`

When done, run `make clean` to clean up objective files & executables.

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

The `Monitor` app handles:
- A Hash Table containing `CitizenRecord` objects.
- A Linked List containing `VirusRecords` objects.
- A Linked List containing `CountryStatus` objects.
- An Array of `DirectoryInfo` objects to handled assigned country directories.

The `CountryStatus` and `VirusRecords` objects are each associated to a Country and Virus respectively,
so they are stored in Lists, because the number of Countries and of Viruses is
quite limited.

The `parentMonitor` app handles:
- An Array of `MonitorInfo` objects (one for each child Monitor).
- An Array of `CountryMonitor` objects (one for each Country subdirectory).
- The `struct dirent` Array returned by `scandir`.
- A Linked List containing `VirusFilter` objects.

The the number of Countries and of Viruses is quite limited, so the use of an Array and a Linked List respectively
does not have a big impact in the app performance.

### `travelMonitor` and `Monitor` execution flow
The parent process scans the input directory specified by the user for Country directories, and assigns the directories to the
child Monitors (Round-Robin). If the number of Monitors specified by the user is greater than the number of found directories,
the parent process will create the same number of children as the countries.

The parent sends all required information to the created child processes (buffer size, bloom filter size, as well as the
country directories assign to each child), and then waits to receive each child Bloom Filters (and to "merge" the Bloom Filters related
to the same Virus).

Each child process receives the required information by the parent and scans all the files in the assigned directories
for Vaccination Records, which are processed and stored. When done with file processing, each child sends the Bloom Filters of each
virus to the parent process, and then enters a "listening" loop to accept and process requests/signals sent by the parent.

After the parent has obtained all the Bloom Filters, it enters "command line" mode and expects user commands from `stdin`.

### Pipe I/O & Process communication
Each process uses a buffer (with the buffer size the user has selected) to read/write data from/to a pipe.
The buffer size can be as small as 1 byte.

When the parent process wants to request certain information from a child Monitor,
it writes any required arguments in the Monitor write pipe, and notifies the Monitor by sending SIGUSR2.
Then the parent monitor opens the read pipe for this Monitor to receive the Monitor answer.
At the same time, the child Monitor receives the sent information, executes the command and sends the answer to the parent.

The only exception is in the `/addVaccinationRecords` command, where the parent process just sends a SIGUSR1
to the Monitor that handles the specified country, and then opens the read pipe to receive the updated Bloom Filters
from the Monitor.

In `/searchVaccinationStatus` (as well as in the beginning when receiving all the child Monitor Bloom Filters), the parent
process uses `select()` to choose the Monitor to receive data from. In this way, a slower Monitor will not prevent the parent
from receiving the data of other, faster Monitors.

### Pipe Messaging "protocol"
`app/pipe_msg` files provide routines for sending and receivng data through fifo pipes.
There are routines for sending and receiving `unsigned [long]/[short] int` variables,
 `BloomFilter` objects (the byte arrays are sent/received), `Date` objects (the day/month/year fields),
 `char*` arrays as well as `char` variables (used for sending/receiving message types).

- When sending/receiving `int` or `char`, just the variable bytes are sent/received wihtout any special operation taking place.
- When sending a `char*`, the **length** of the string is sent first, followed by the actual string bytes. Symmetrically,
the receiver first asks for the string length and then receives the string bytes (so as to know when to stop receiving).
- To send a `BloomFilter`, `sendBloomFilter` just writes the byte array of the filter to the pipe. On the other hand,
`updateBloomFilter` takes an existing `BloomFilter` as argument, receives the sent filter byte array through the pipe
and updates the existing one using bitwise-OR.

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
- Regarding Vaccination Record insertion from files:
  If a "NO" record (about a Virus vaccination) has been inserted for a certain Citizen and
  a "YES" record for the same Citizen and Virus is inserted later, the existing record will be **modified**
  (removal from "non-vaccinated" Skip List, insertion to "vaccinated" Skip List & Bloom Filter, etc.).
  Of course the opposite is **not allowed** (a "YES" record being modified to a "NO" record). 
- Citizen, Country and Virus Names are **case sensitive**
  (e.g. "Greece", "greece" and "GREECE" are considered 3 different countries).
- The Citizen ID's allowed by the app can have up to a certain number of digits,
  which is specified by the `MAX_ID_DIGITS` macro in `app/app_utils.hpp`. It is set to `5` by default.
- Macros used by Monitor/travelMonitor:
  - `app/monitor.cpp`: `HASHTABLE_BUCKETS` to set the number of buckets in Citizen HashTables.
  - `app/app_utils.hpp` : `MAX_BLOOM_SIZE` and `MAX_ID_DIGITS` for maximum `BloomFilter` size and
  maximum number of digits allowed in Citizen ID's respectively.
  - `app/parent_monitor.cpp`: `MAX_MONITORS` and `MAX_BUFFER_SIZE` as upper limits in number of child Monitors
  and buffer size respectively.
- As told by the instructors, it is assumed that "third-party" signals (`SIGINT/SIGQUIT` in child Monitors)
can be sent only when things are "idle/stable". The parent app will block if e.g. it is waiting for
a child to send some data and such signal is sent to the child before the data has been sent.

### Resource Handling
The app has been tested with Valgrind and no leaks are reported in multiple scenarios.
Note that that there is a reported "bug" in Valgrind used in certain DIT workstations, regarding C++ memory pooling,
which causes some bytes to be reported as "still reachable" after the end of the execution of any C++ program.
When testing with Valgrind in DIT Labs, apart from the above "leak", no other leaks were reported.
You can read more here:
- http://valgrind.org/docs/manual/faq.html#faq.reports
- https://gcc.gnu.org/onlinedocs/libstdc++/faq.html#faq.memory_leaks

### Development & Testing
Developed & tested in WSL Ubuntu 20.04, using Visual Studio Code.
Successfully tested in DIT Lab Ubuntu 18.04 as well.
