/* Filename: write_cntr_shm.c */

#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define SHM_KEY 0x12345
struct shmseg {
   int cntr;
   int write_complete;
   int read_complete;
};
void shared_memory_cntr_increment(int pid, struct shmseg *shmp, int total_count);

int main(int argc, char *argv[]) {
   int shmid;
   struct shmseg *shmp;
   char *bufptr;
   int total_count;
   int sleep_time;
   pid_t pid;
   if (argc != 2){
      total_count = 10000;
   }else {
      total_count = atoi(argv[1]);
      if (total_count < 10000)
      total_count = 10000;
   }
   printf("Total Count is %d\n", total_count);
   shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644|IPC_CREAT);

   if (shmid == -1) {
      perror("Shared memory");
      return 1;
   }

// доступ к общему сегменту, чтобы получить указатель 
   shmp = shmat(shmid, NULL, 0);
   if (shmp == (void *) -1) {
      perror("Доступ к разделяемой памяти");
      return 1;
   }
   shmp->cntr = 0;
// создали дочерний процесс
   pid = fork();

   /* процесс родитель – однажды запишет */
   if (pid > 0) {
      shared_memory_cntr_increment(pid, shmp, total_count);
   } else if (pid == 0) {
      shared_memory_cntr_increment(pid, shmp, total_count);
      return 0;
   } else {
      perror("Fork Failure\n");
      return 1;
   }
   while (shmp->read_complete != 1)
   sleep(1);

   if (shmdt(shmp) == -1) {
      perror("shmdt");
      return 1;
   }

   if (shmctl(shmid, IPC_RMID, 0) == -1) {
      perror("shmctl");
      return 1;
   }
   printf("Пишущий процесс: завершено\n");
   return 0;
}

/* Увеличить счетчик в общей разделяемой памяти total_count с шагом 1 */
void shared_memory_cntr_increment(int pid, struct shmseg *shmp, int total_count) {
   int cntr;
   int numtimes;
   int sleep_time;
   cntr = shmp->cntr;
   shmp->write_complete = 0;
   if (pid == 0)
   printf("WRITE_SHM: CHILD: сейчас записываю\n");
   else if (pid > 0)
   printf("WRITE_SHM: PARENT: сейчас записываю\n");
   //printf("SHM_CNTR is %d\n", shmp->cntr);
   
   /* Увеличить счетчик в общей разделяемой памяти total_count с шагом 1*/
   for (numtimes = 0; numtimes < total_count; numtimes++) {
      cntr += 1;
      shmp->cntr = cntr;
      
      /* засыпать на секунду на каждую тысячу раз */
      sleep_time = cntr % 1000;
      if (sleep_time == 0)
      sleep(1);
   }
   
   shmp->write_complete = 1;
   if (pid == 0)
   printf("SHM_WRITE: CHILD: Запись выполнена\n");
   else if (pid > 0)
   printf("WRITE_SHM: PARENT: Запись выполнена\n");
   return;
}

