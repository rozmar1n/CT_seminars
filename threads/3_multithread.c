#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

struct trig_job
{
    double angle_rad;
    double * out;
};

void * sin_worker (void * arg)
{
    struct trig_job * job = (struct trig_job *) arg;
    double s = sin (job->angle_rad);
    *(job->out) = s * s;
    fprintf (stderr, "sin^2(%.3f) = %.6f\n", job->angle_rad, *(job->out));
    return NULL;
}

void * cos_worker (void * arg)
{
    struct trig_job * job = (struct trig_job *) arg;
    double c = cos (job->angle_rad);
    *(job->out) = c * c;
    fprintf (stderr, "cos^2(%.3f) = %.6f\n", job->angle_rad, *(job->out));
    return NULL;
}

int main (void)
{
    pthread_t thread1, thread2;
    const double angle = 1.2345;//в радианах

    double sin_sq = 0.0;
    double cos_sq = 0.0;

    struct trig_job sin_job = { angle, &sin_sq };
    struct trig_job cos_job = { angle, &cos_sq };

    if (pthread_create (&thread1, NULL, &sin_worker, &sin_job) != 0)
    {
        fprintf (stderr, "Error (thread1)\n");
        return 1;
    }

    if (pthread_create (&thread2, NULL, &cos_worker, &cos_job) != 0)
    {
        fprintf (stderr, "Error (thread2)\n");
        return 1;
    }

    pthread_join (thread1, NULL);
    pthread_join (thread2, NULL);

    double sum = sin_sq + cos_sq;
    fprintf (stderr, "sum = sin^2 + cos^2 = %.6f\n", sum);
    fprintf (stderr, "Expected ~1.0, deviation = %.6e\n", fabs (1.0 - sum));

    return 0;
}
