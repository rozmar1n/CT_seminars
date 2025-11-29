#include <stdio.h>
#include <pthread.h>
double a;

void * any_func (void * arg)
{
    printf("\n\n\n\nanyfunc\n");
    printf ("arg %p\n", arg);
    printf ("*arg %f\n", *(double *)arg);
    a = *(double *)arg;
    a++;
    printf ("a %f\n", a);
    double * pa = &a;
    printf ("pa %p\n", pa);
    printf ("*pa %f\n", *pa);
    printf("\n\n\n\n");
    return pa;
}

int main (void)
{
    pthread_t thread;
    double data = 2007;
    void * pp = &data;
    int q = 0;
    void * pq = (void *)&q;
    void ** pdata = pq;// = 0; 
    printf ("pp %p\n", pp);
    printf (" * pp %f\n", *((double *)pp));
    if (pthread_create(&thread, NULL,  &any_func, pp) != 0) 
    {
        fprintf (stderr, "Error\n");
        return 1;
    }
    printf ("2 pp %p\n", pp);
    printf ("2 * pp %f\n", *((double *)pp));

    pthread_join (thread, pdata);
    printf ("pdata %p\n", pdata);
    printf ("* pdata %p\n", *pdata);
    printf (" ** pdata %f\n", (double)(**(double **)pdata));
    a++;
    printf (" ** pdata %f\n", (double)(**(double **)pdata));
    
    return 0;
}
