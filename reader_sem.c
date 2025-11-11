#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define BUF_SIZE 256

#define SEM_FULL 0
#define SEM_EMPTY 1

struct shmseg {
    char data[BUF_SIZE];
};

void sem_wait(int semid, int semnum) {
    struct sembuf sb = {semnum, -1, 0};
    semop(semid, &sb, 1);
}

void sem_signal(int semid, int semnum) {
    struct sembuf sb = {semnum, 1, 0};
    semop(semid, &sb, 1);
}

int main(void) {
    int shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    int semid = semget(SEM_KEY, 2, 0666);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }

    struct shmseg *shmp = shmat(shmid, NULL, 0);
    if (shmp == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    printf("Reader: waiting for updates...\n");
    while (1) {
        sem_wait(semid, SEM_FULL); // ждать, пока буфер заполнен

        if (strcmp(shmp->data, "end") == 0)
            break;

        printf("Reader: read '%s'\n", shmp->data);
        sem_signal(semid, SEM_EMPTY); // разрешить запись
    }

    sem_signal(semid, SEM_EMPTY);
    shmdt(shmp);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    printf("Reader: finished.\n");
    return 0;
}
