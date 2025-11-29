/* Программа для распечатки аргументов командной строки и параметров среды
 * текущего процесса */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]) 
{
    int i;

    for (i = 0; envp[i] != NULL; i++) 
    {
        printf("envp[%d]: %s\n", i, envp[i]);
    }
    for (i = 0; i < argc; i++)
    {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    return 0;
}
