# System Programming Projects

This is a series of projects for the Spring 2021 System Programming course on [DIT@UoA](https://www.di.uoa.gr/en).

## General Task
All projects focus on building an app to store & monitor **Vaccination Records** for citizens from different countries.
The app uses "records" (reading them from files) with the following format:
    
    <citizenId> <name> <surname> <countryName> <age> <virusName> <isVaccinated(YES/NO)> <optionalVaccinationDate>

e.g.

    889 John Papadopoulos Greece 52 COVID-19 YES 27-12-2020
    776 Maria Tortellini Italy 36 SARS-1 NO

The app can then respond to queries regarding the imported records,
to display the vaccination status of a citizen,
stats on a specific virus or country, insert new vaccination records etc.

Each project expands on this task, as explained below.

## Project Tasks & Goals

- **Project 1**:\
Development of the **base app**, which operates as explained above.
The project also includes the development of two other data structures:
    - [**Bloom Filter**](https://en.wikipedia.org/wiki/Bloom_filter),
    a probabilistic structure to determine whether a record does not exist or **possibly exists**.
    - [**Skip List**](https://en.wikipedia.org/wiki/Skip_list),
    to efficiently store and retrieve vaccination records regarding a specific virus.

    Both structures are also explained in `project1/task.pdf`.
    Finally, the project includes generating citizen records
    (by using existing country & viruses names) using **Bash Scripting**.
- **Project 2**:\
Introduction of multiprocessing & interprocess communication via FIFO  pipes:
A parent monitor process creates a number of child monitor processes,
which expand the functionality of the project 1 application:
    - The child monitors communicate with the parent monitor using one-way FIFO pipes.
    Each child is assigned two such pipes, one for reading and one for writing.
    All processes are obliged to use a buffer with **limited** size to read/write
    and communicate using a defined **communication protocol**.
    A process notifies another for sent data using defined **signals**.
    - Each child monitor is assigned a set of directories to read record files from,
    reads and stores the records similarily to project 1.
    - All child monitors send their Bloom Filters to the parent process, which **unifies** them.
    - The parent process keeps track of the directories assigned to each monitor.
    Each directory contains record files regarding a **specific country**.
    - The parent application can then serve user queries,
    by invoking the related monitor, or multiple monitors, if needed.
    - The user can add new files to a specific country directory,
    and ask for an update using a specific query.
    The parent monitor will notify the related child monitor,
    which scans the directory and updates the records using the new files.
    - The parent monitor also handles the case of a child monitor **suddenly stopping**,
    by starting a new child monitor to **replace it**.
    - When the user exits, the parent application notifies all child monitors to stop accordingly.

    Finally, the project includes the creation of a **Bash Script**,
    which reads a record file (with vaccination records from different countries),
    creates separate directories (one for each country found in the file)
    and places each country's records into files in the corresponding directory.
- **Project 3**:
    - Replacing FIFO pipes with network **Sockets**,
    adding more complexity in I/O (using `htonl`, `ntohl` and similar routines when required).
    - Using **threads** in each child monitor to read files from the assigned directories:
    The child monitor provides the threads with the vaccination records containers
    and a **shared buffer** (both accessed properly by using **semaphores**)
    which contains the paths to the record files in the directories assigned to the child monitor.
    The monitor acts as a **producer** (placing new file paths in the buffer)
    and each thread acts as a **consumer** (removing a file path from the buffer, 
    reading and storing the vaccination records in the file).

All projects are built in **C++**. 
Other goals which apply to all project parts were:
- Building (and building on) core data structures and containers 
(Linked List, Hash Table and Red-Black Tree) **from scratch** 
(the usage of STL was extremely restricted, essentially allowing only
 `std:string`, `std:stringstream` and other similar namespaces).
- Creating, managing and deleting generalized containers with unknown content type.
- Eliminating data duplication by properly using pointers.
- Correct usage and release of dynamically allocated memory.
- Familiarizing with low-level I/O, sending/reading data in parts & reconstructing it,
 introducing communication protocols, sending & handling signals across processes.
- Dividing problems in subproblems and choosing the most efficient solution for each one.
- Detecting edge cases, stretch testing & targetly debugging the application.

All projects include:
- A `task.pdf` file describing the corresponding project tasks in detail (in Greek).
- A `README.md` file with a brief explanation & comments on the implementation,
 as well as a description of the project structure, execution steps etc.
