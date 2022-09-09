/**
 * File: sem_utils.cpp
 * Wrapper routines for semop() and semctl() that are used by the app for semaphore operations.
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <sys/sem.h>
#include <sys/types.h>

#include "sem_utils.hpp"

union semun
{
    int val;
    struct semid_ds *buff;
    unsigned short *array;
};

/**
 * Initiallizes the semaphore the semaphore with index == semnum in
 * the semaphore array with ID == semid, to the specified value. 
 */
int sem_init(int semid, int semnum, int value)
{
    union semun args;
    args.val = value;
    return semctl(semid, semnum, SETVAL, args);
}

/**
 * Unlocks/Increments the semaphore with index == semnum in
 * the semaphore array with ID == semid.
 */
int sem_up(int semid, int semnum)
{
    struct sembuf semops;
    semops.sem_flg = 0;                    // setting up sembuf properly
    semops.sem_num = semnum;
    semops.sem_op = 1;

    return semop(semid, &semops, 1);         // semaphore up
}

/** 
 * Locks/Decrements the semaphore with index == semnum in
 * the semaphore array with ID == semid.
*/
int sem_down(int semid, int semnum)
{
    struct sembuf semops;
    semops.sem_flg = 0;                    // setting up sembuf properly
    semops.sem_num = semnum;
    semops.sem_op = -1;
    
    return semop(semid, &semops, 1);         // semaphore down
}

/**
 * Removes the semaphore array with the specified ID.
 */
int sem_delete(int sem_id)
{
    return semctl(sem_id, 0, IPC_RMID, 0);
}

