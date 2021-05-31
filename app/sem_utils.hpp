int sem_init(int semid, int semnum, int value);
int sem_up(int semid, int semnum);
int sem_down(int semid, int semnum);
int sem_delete(int sem_id);