/**
 * File: sem_utils.hpp
 * Wrapper routines for semop() and semctl() that are used by the app for semaphore operations.
 * Pavlos Spanoudakis (sdi1800184)
 */

int sem_init(int semid, int semnum, int value);
int sem_up(int semid, int semnum);
int sem_down(int semid, int semnum);
int sem_delete(int sem_id);