#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int fd;
    ssize_t size;
    char string[60];

    /* Попытаемся открыть файл с именем в первом параметре выззова только
    для операций чтения */

    if((fd = open(argv[1], O_RDONLY)) < 0)
    {
        /* Если файл открыть не удалось, печатаем об этом сообщение и прекращаем
        работу */
        printf("Can\'t open file\n");
        exit(-1);
    }

    /* Читаем фаил пока не кончится и печатаем */
    
    size = read(fd, string, 58);
    string[size] = '\0';
    while(size > 0) 
    {
        printf("%s", string);
        size = read(fd, string, 58);
        string[size] = '\0';
    }
    /*  Записываем файл под новым именем */
    int err = rename(argv[1], argv[2]); 
    if(err) {
        perror("Errror while renaming");
    }
    /* Закрываем файл */
    if (close(fd) < 0) {
        printf("Can\'t close file\n");
    }
    /*  Открываем файл в редакторе */
    if(argc >= 2) {
        char cmd[256];
        sprintf(cmd, "nano %s", argv[2]);
        system(cmd);
    }
    return 0;
}
