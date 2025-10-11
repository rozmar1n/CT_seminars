#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 0x5150
#define SEM_KEY 0x5150
#define SEM_OWNER 0
#define SEM_PLAYER 1

/* Единственное поле, которое делим между процессами: текущее значение счётчика. */
struct shared_state {
  int value;
};

/* Аргумент для semctl() — классический union, аналогичный POSIX-примеру. */
union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

/* Обёртка вокруг semop() для операции ожидания (P-операция). */
static int sem_wait(unsigned int semid, unsigned short index) {
  struct sembuf op = {index, -1, SEM_UNDO};
  return semop(semid, &op, 1);
}

/* Обёртка для операции сигнала (V-операция). */
static int sem_signal(unsigned int semid, unsigned short index) {
  struct sembuf op = {index, 1, SEM_UNDO};
  return semop(semid, &op, 1);
}

/* Выводит сообщение об ошибке и завершает процесс. */
static void perror_exit(const char *ctx) {
  perror(ctx);
  exit(EXIT_FAILURE);
}

/* Создатель ресурса считывает стартовое число и управляет жизненным циклом IPC. */
static void run_owner(void) {
  const size_t shm_size = sizeof(struct shared_state);
  /* Создаём новый сегмент разделяемой памяти под одно целое число. */
  int shm_id = shmget(SHM_KEY, shm_size, IPC_CREAT | IPC_EXCL | 0600);

  if (shm_id == -1) {
    if (errno == EEXIST) {
      fprintf(stderr,
              "Shared memory already exists — please clean it up (ipcrm) and retry.\n");
    }
    perror_exit("shmget");
  }

  /* Создаём набор семафоров: [0] — владелец, [1] — подключившийся игрок. */
  int sem_id = semget(SEM_KEY, 2, IPC_CREAT | IPC_EXCL | 0600);
  if (sem_id == -1) {
    int saved = errno;
    shmctl(shm_id, IPC_RMID, NULL);
    errno = saved;
    perror_exit("semget");
  }

  /* Встраиваем сегмент в адресное пространство и получаем указатель. */
  struct shared_state *state =
      (struct shared_state *)shmat(shm_id, NULL, 0);
  if (state == (void *)-1) {
    int saved = errno;
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    errno = saved;
    perror_exit("shmat");
  }

  /* На всякий случай зануляем структуру перед использованием. */
  memset(state, 0, sizeof(*state));

  printf("Enter initial value: ");
  fflush(stdout);

  int input = 0;
  if (scanf("%d", &input) != 1) {
    fprintf(stderr, "Invalid input.\n");
    shmdt(state);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    exit(EXIT_FAILURE);
  }
  state->value = input;

  union semun arg;
  unsigned short init_vals[2] = {1, 0}; /* владелец стартует первым */
  arg.array = init_vals;
  if (semctl(sem_id, 0, SETALL, arg) == -1) {
    perror("semctl");
    shmdt(state);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    exit(EXIT_FAILURE);
  }

  printf("Starting ping-pong. PID=%d\n", getpid());
  fflush(stdout);

  while (1) {
    if (sem_wait(sem_id, SEM_OWNER) == -1) {
      perror_exit("semop(owner wait)");
    }

    int current = state->value;
    if (current <= 0) {
      sem_signal(sem_id, SEM_PLAYER); /* разбудить напарника для корректного выхода */
      break;
    }

    current -= 1;
    state->value = current;

    printf("[owner] value=%d\n", current);
    fflush(stdout);
    sleep(1);

    if (sem_signal(sem_id, SEM_PLAYER) == -1) {
      perror_exit("semop(owner signal)");
    }

    if (current <= 0) {
      break;
    }
  }

  shmdt(state);
  semctl(sem_id, 0, IPC_RMID);
  shmctl(shm_id, IPC_RMID, NULL);
  printf("Owner finished; resources released.\n");
}

/* Второй процесс подключается и по очереди вычитает единицу. */
static void run_player(void) {
  /* Находим созданный владельцем сегмент. */
  int shm_id = shmget(SHM_KEY, sizeof(struct shared_state), 0600);
  if (shm_id == -1) {
    perror_exit("shmget");
  }

  /* Получаем доступ к набору семафоров. */
  int sem_id = semget(SEM_KEY, 2, 0600);
  if (sem_id == -1) {
    perror_exit("semget");
  }

  /* Подключаем сегмент памяти, чтобы работать с числом совместно. */
  struct shared_state *state =
      (struct shared_state *)shmat(shm_id, NULL, 0);
  if (state == (void *)-1) {
    perror_exit("shmat");
  }

  printf("Player connected. PID=%d\n", getpid());
  fflush(stdout);

  while (1) {
    if (sem_wait(sem_id, SEM_PLAYER) == -1) {
      perror_exit("semop(player wait)");
    }

    int current = state->value;
    if (current <= 0) {
      sem_signal(sem_id, SEM_OWNER);
      break;
    }

    current -= 1;
    state->value = current;

    printf("[player] value=%d\n", current);
    fflush(stdout);
    sleep(1);

    if (sem_signal(sem_id, SEM_OWNER) == -1) {
      perror_exit("semop(player signal)");
    }

    if (current <= 0) {
      break;
    }
  }

  shmdt(state);
  printf("Player finished.\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s [create|join]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "create") == 0) {
    run_owner();
  } else if (strcmp(argv[1], "join") == 0) {
    run_player();
  } else {
    fprintf(stderr, "Unknown role '%s'. Use create or join.\n", argv[1]);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
