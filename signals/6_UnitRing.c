#define _POSIX_C_SOURCE 200809L /* sigaction, pause */
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>


#define MAX_PIDS 32
#define SHM_PROJ 'R'
#define SEM_PROJ 'S'

struct shared_data {
    uint64_t value;      
    pid_t pids[MAX_PIDS];
};

static volatile sig_atomic_t got_usr1 = 0;
static volatile sig_atomic_t stop = 0;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

static void handle_usr1(int signo) {
    (void)signo;
    got_usr1 = 1;
}

static void handle_term(int signo) {
    (void)signo;
    stop = 1;
}

static int sem_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    return semop(semid, &op, 1);
}

static int sem_unlock(int semid) {
    struct sembuf op = {0, +1, 0};
    return semop(semid, &op, 1);
}

static int get_ipc(int *shmid, int *semid, struct shared_data **data, int create) {
    key_t shm_key = ftok(__FILE__, SHM_PROJ); 
    key_t sem_key = ftok(__FILE__, SEM_PROJ);
    if (shm_key == -1 || sem_key == -1) {
        perror("ftok");
        return -1;
    }

    int shm_flags = 0600 | (create ? IPC_CREAT : 0);
    *shmid = shmget(shm_key, sizeof(struct shared_data), shm_flags);
    if (*shmid == -1) {
        perror("shmget");
        return -1;
    }

    void *addr = shmat(*shmid, NULL, 0);
    if (addr == (void *)-1) {
        perror("shmat");
        return -1;
    }
    *data = (struct shared_data *)addr;

    int sem_flags = 0600 | (create ? IPC_CREAT : 0);
    *semid = semget(sem_key, 1, sem_flags);
    if (*semid == -1) {
        perror("semget");
        return -1;
    }

    if (create) {
        union semun arg;
        arg.val = 1;
        if (semctl(*semid, 0, SETVAL, arg) == -1) {
            perror("semctl SETVAL");
            return -1;
        }

        if (sem_lock(*semid) == -1) {
            perror("sem_lock init");
            return -1;
        }
        (*data)->value = 0;
        memset((*data)->pids, 0, sizeof((*data)->pids));
        sem_unlock(*semid);
    }

    return 0;
}

static void unregister_pid(struct shared_data *data, int semid, pid_t pid) {
    if (sem_lock(semid) == -1) {
        perror("sem_lock unregister");
        return;
    }
    for (int i = 0; i < MAX_PIDS; ++i) {
        if (data->pids[i] == pid) {
            data->pids[i] = 0;
            break;
        }
    }
    sem_unlock(semid);
}

static int run_worker(void) {
    int shmid, semid;
    struct shared_data *data;
    if (get_ipc(&shmid, &semid, &data, 0) == -1) return 1;

    struct sigaction act_usr = {.sa_handler = handle_usr1, .sa_flags = 0};
    sigemptyset(&act_usr.sa_mask);
    if (sigaction(SIGUSR1, &act_usr, NULL) == -1) {
        perror("sigaction SIGUSR1");
        return 1;
    }
    struct sigaction act_term = {.sa_handler = handle_term, .sa_flags = 0};
    sigemptyset(&act_term.sa_mask);
    if (sigaction(SIGINT, &act_term, NULL) == -1 ||
        sigaction(SIGTERM, &act_term, NULL) == -1) {
        perror("sigaction term");
        return 1;
    }

    pid_t pid = getpid();

    if (sem_lock(semid) == -1) {
        perror("sem_lock register");
        return 1;
    }
    int slot = -1;
    for (int i = 0; i < MAX_PIDS; ++i) {
        if (data->pids[i] == 0) {
            data->pids[i] = pid;
            slot = i;
            break;
        }
    }
    sem_unlock(semid);
    if (slot == -1) {
        fprintf(stderr, "No free slots in shared memory\n");
        return 1;
    }

    printf("Worker PID %d registered in slot %d. Waiting for SIGUSR1...\n",
           (int)pid, slot);

    while (!stop) {
        pause();

        if (stop) break;

        if (got_usr1) {
            got_usr1 = 0;

            if (sem_lock(semid) == -1) {
                perror("sem_lock work");
                break;
            }
            data->value += 1;
            uint64_t current = data->value;
            sem_unlock(semid);

            printf("[PID %d] value=%" PRIu64 "\n", (int)pid, current);
            fflush(stdout);
        }
    }

    unregister_pid(data, semid, pid);
    shmdt(data);
    return 0;
}

static int run_manager(void) {
    int shmid, semid;
    struct shared_data *data;
    if (get_ipc(&shmid, &semid, &data, 1) == -1) return 1;

    printf("Manager started. Commands: 's' + Enter to send SIGUSR1, 'q' to quit.\n");

    int ch;
    while ((ch = getchar()) != EOF) {
        if (ch == 'q') break;
        if (ch != 's') continue;

        pid_t pids[MAX_PIDS];
        int count = 0;

        if (sem_lock(semid) == -1) {
            perror("sem_lock manager");
            continue;
        }
        for (int i = 0; i < MAX_PIDS; ++i) {
            if (data->pids[i] > 0) {
                pids[count++] = data->pids[i];
            }
        }
        sem_unlock(semid);

        printf("Sending SIGUSR1 to %d workers...\n", count);
        for (int i = 0; i < count; ++i) {
            if (kill(pids[i], SIGUSR1) == -1) {
                fprintf(stderr, "kill(%d): %s\n", (int)pids[i], strerror(errno));
            }
        }
    }

    shmdt(data);
    return 0;
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s manager|worker\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "manager") == 0) {
        return run_manager();
    } else if (strcmp(argv[1], "worker") == 0) {
        return run_worker();
    } else {
        usage(argv[0]);
        return 1;
    }
}
