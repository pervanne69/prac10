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

// операция P (–1)
void sem_wait(int semid, int semnum) {
    struct sembuf sb = {semnum, -1, 0};
    semop(semid, &sb, 1);
}

// операция V (+1)
void sem_signal(int semid, int semnum) {
    struct sembuf sb = {semnum, 1, 0};
    semop(semid, &sb, 1);
}

int main(void) {
    int shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    int semid = semget(SEM_KEY, 2, 0666 | IPC_CREAT);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }

    // Инициализация семафоров: EMPTY=1 (можно писать), FULL=0 (пусто)
    semctl(semid, SEM_EMPTY, SETVAL, 1);
    semctl(semid, SEM_FULL, SETVAL, 0);

    struct shmseg *shmp = shmat(shmid, NULL, 0);
    if (shmp == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    for (int i = 0; i < 5; i++) {
        sem_wait(semid, SEM_EMPTY); // ждать, пока буфер пуст

        FILE *fp = popen("date", "r");
        fgets(shmp->data, BUF_SIZE, fp);
        pclose(fp);
        printf("Writer: wrote '%s'\n", shmp->data);

        sem_signal(semid, SEM_FULL); // разрешить чтение
        sleep(2);
    }

    sem_wait(semid, SEM_EMPTY);
    strcpy(shmp->data, "end");
    sem_signal(semid, SEM_FULL);

    shmdt(shmp);
    return 0;
}
