#include <stdio.h>
#include <pthread.h>
#include <stdint.h> 
void * any_func (void * arg)
{
    int a = *(int *) arg;
    a++;
    return (void *)(intptr_t)a;
}

int main (void)
{
    pthread_t thread;
    int parg = 2007, pdata;

    if (pthread_create (&thread, NULL, &any_func, &parg) != 0)
    {
        fprintf (stderr, "Error\n");
        return 1;
    }

    pthread_join (thread, (void *) &pdata);
    printf ("%d\n", pdata);

    pdata = (int)(intptr_t)any_func(&pdata);
    printf("%d\n", pdata);

    return 0;
}
