#include <stdio.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define SHMEM_SIZE 4096
#define SH_MESSAGE "Hello World!\n"

#define SEM_KEY 2007
#define SHM_KEY 2007
// Написать комментарии, отладить работу

// Универсальный аргумент для semctl().
union semnum {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
} sem_arg;

int main(void) {
    int shm_id, sem_id;
    char *shm_buf;
    int shm_size;
    struct shmid_ds ds;
    struct sembuf sb[1];
    unsigned short sem_vals[1];

    // Создаём новый сегмент разделяемой памяти фиксированного размера.
    shm_id = shmget(SHM_KEY, SHMEM_SIZE, IPC_CREAT | IPC_EXCL | 0600);

    if (shm_id == -1) 
    {
      fprintf(stderr, "shmget() error\n");
      return 1;
    }

    // Создаём набор из одного семафора для синхронизации с читателем.
    sem_id = semget(SEM_KEY, 1, 0600 | IPC_CREAT | IPC_EXCL);

    if (sem_id == -1) 
    {
      fprintf(stderr, "semget() error\n");
      return 1;
    }

    printf("Semaphore: %d\n", sem_id);
    sem_vals[0] = 1;
    sem_arg.array = sem_vals;

    // Инициализируем семафор значением 1 — ресурс пока свободен.
    if (semctl(sem_id, 0, SETALL, sem_arg) == -1) 
    {
      fprintf(stderr, "semctl() error\n");
      return 1;
    }

    // Подключаем сегмент к адресному пространству процесса.
    shm_buf = (char *)shmat(shm_id, NULL, 0);
    if (shm_buf == (char *)-1) 
    {
      fprintf(stderr, "shmat() error\n");
      return 1;
    }

    // Узнаём реальный размер сегмента, чтобы убедиться, что места хватит.
    shmctl(shm_id, IPC_STAT, &ds);

    shm_size = ds.shm_segsz;
    if (shm_size < strlen(SH_MESSAGE)) 
    {
      fprintf(stderr, "error: segsize=%d\n", shm_size);
      return 1;
    }

    // Записываем строку в разделяемую память для читателя.
    strcpy(shm_buf, SH_MESSAGE);

    printf("ID: %d\n", shm_id);

    // Подготавливаем описание операции для семафора.
    sb[0].sem_num = 0;
    sb[0].sem_flg = SEM_UNDO;

    // Первое захватывание: уменьшаем семафор с 1 до 0 без ожидания.
    sb[0].sem_op = -1;
    semop(sem_id, sb, 1);

    // Второе захватывание: блокируемся до сигнала от читателя.
    sb[0].sem_op = -1;
    semop(sem_id, sb, 1);

    // После завершения очищаем семафор и сегмент, отсоединяем память.
    semctl(sem_id, 1, IPC_RMID, sem_arg);
    shmdt(shm_buf);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
