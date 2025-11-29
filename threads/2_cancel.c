#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
/* Модифицировать программу,
 * чтобы замерить среднее время завершения нити после сигнала на завершение.
 */

/* Поток каждую секунду даёт точку и засыпает.
 * sleep(1) — точка отмены: pthread_cancel пробуждает поток,
 * и он завершается в ближайшем system call, который проверяет флаг отмены. */
void * any_func (void * arg)
{
    (void)arg; /* параметр не используется */

    while (1)
    {
        fprintf (stderr, ".");
        sleep (1); /* создаёт точку отмены для pthread_cancel */
    }

    return NULL;
}

/* Вычисляет разницу end-start в наносекундах. */
static long long diff_ns (const struct timespec * start,
                          const struct timespec * end)
{
    return (end->tv_sec - start->tv_sec) * 1000000000LL +
           (end->tv_nsec - start->tv_nsec);
}

int main (void)
{
    const int attempts = 5;
    const unsigned pre_cancel_delay = 2;

    long long total_ns = 0;

    for (int i = 0; i < attempts; ++i)
    {
        pthread_t thread;
        void * result = NULL;

        if (pthread_create (&thread, NULL, &any_func, NULL) != 0)
        {
            fprintf (stderr, "Error\n");
            return 1;
        }

        sleep (pre_cancel_delay);

        struct timespec start = {0}, end = {0};
        clock_gettime (CLOCK_MONOTONIC, &start);

        pthread_cancel (thread);

        if (!pthread_equal (pthread_self (), thread))
        {
            pthread_join (thread, &result);
        }

        clock_gettime (CLOCK_MONOTONIC, &end);
        long long elapsed = diff_ns (&start, &end);
        total_ns += elapsed;

        fprintf (stderr, "\niter %d: waited %.3f ms after cancel (result=%s)\n",
                 i + 1,
                 (double) elapsed / 1e6,
                 result == PTHREAD_CANCELED ? "PTHREAD_CANCELED" : "unknown");
    }

    double avg_ms = (double) total_ns / attempts / 1e6;
    fprintf (stderr, "\nAverage wait after cancel: %.3f ms over %d tries\n",
             avg_ms, attempts);

    return 0;
}
