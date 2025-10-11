#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define SEM_KEY 2007
#define SHM_KEY 2007
// Написать комментарии, отладить работу

int main(int argc, char **argv) 
{
    int shm_id, sem_id;
    char *shm_buf;
    struct sembuf sb[1];

    // Получаем дескриптор уже созданного сегмента разделяемой памяти.
    shm_id = shmget(SHM_KEY, 1, 0600);
    if (shm_id == -1) 
    {
      fprintf(stderr, "shmget() error\n");
      return 1;
    }

    // Открываем существующий семафор, которым владелец нас блокирует.
    sem_id = semget(SEM_KEY, 1, 0600);
    if (sem_id == -1) 
    {
      fprintf(stderr, "semget() error\n");
      return 1;
    }

    // Подключаем сегмент в адресное пространство процесса читателя.
    shm_buf = (char *)shmat(shm_id, 0, 0);
    if (shm_buf == (char *)-1) 
    {
      fprintf(stderr, "shmat() error\n");
      return 1;
    }

    // Выводим строку, записанную владельцем в общую память.
    printf("Message: %s\n", shm_buf);

    // Готовим структуру операции: будем поднимать семафор.
    sb[0].sem_num = 0;
    sb[0].sem_flg = SEM_UNDO;

    // Отправляем сигнал владельцу — увеличиваем значение семафора.
    sb[0].sem_op = 1;
    semop(sem_id, sb, 1);

    // Отсоединяемся от сегмента и завершаем работу.
    shmdt(shm_buf);

    return 0;
}
