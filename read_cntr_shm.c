/* Filename: read_cntr_shm.c */
#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>

#define SHM_KEY 0x12345
struct shmseg {
   int cntr;
   int write_complete;
   int read_complete;
};

int main(int argc, char *argv[]) {
   int shmid, numtimes;
   struct shmseg *shmp;
   int total_count;
   int cntr;
   int sleep_time;
   if (argc != 2)
   total_count = 10000;
   
   else {
      total_count = atoi(argv[1]);
      if (total_count < 10000)
      total_count = 10000;
   }
   shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644|IPC_CREAT);
   
   if (shmid == -1) {
      perror("Разделяемая память ");
      return 1;
   }
   /* Доступ к разделяемой памяти, чтобы получить указатель на нее */
   shmp = shmat(shmid, NULL, 0);
   
   if (shmp == (void *) -1) {
      perror("Доступ к разделяемой памяти");
      return 1;
   }
   
   /* Читает разделяемую память cntr и печатает на стандартный вывод*/
   while (shmp->write_complete != 1) {
      if (shmp->cntr == -1) {
         perror("read");
         return 1;
      }
      sleep(3);
   }
   printf("Читающий процесс: Разделяемая память: счетчик = %d\n", shmp->cntr);
   printf("Читающий процесс: Чтение завершено, Доступ к разделяемой памяти\n");
   shmp->read_complete = 1;
   
   if (shmdt(shmp) == -1) {
      perror("shmdt");
      return 1;
   }
   printf("Читающий процесс: завершено\n");
   return 0;
}

