#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int fd_wr, fd_rd;
    if(argc < 2) {
        printf("You must specify the program number.\n");
        return 0;
    }
    else 
    {
        if(strcmp(argv[1], "1") == 0)
        {
            fd_wr = open("write_1.txt", O_WRONLY | O_CREAT | O_SYNC | O_TRUNC , 0644);
            fd_rd = open("read_1.txt", O_RDONLY);
            if(fd_wr < 0 || fd_rd < 0) {
                perror("Error while opening files");
                exit(1);
            }
            pid_t chpid = fork();
            if(chpid < 0) {
                perror("Error while forking");
                exit(1);
            }
            if(chpid > 0) 
            {
                // parent process
                dup2(fd_wr, 1);
                int ch = getc(stdin);
                while(ch != EOF) 
                {
                    putc(ch, stdout);
                    fflush(stdout);
                    ch = getc(stdin);
                }
            }
            else 
            {
                // child process
                dup2(fd_rd, STDIN_FILENO);
                while (1) {
                    int ch = getc(stdin);
                    if (ch == EOF) {
                    // Если файл временно пуст — подожди и попробуй снова
                    usleep(100000); // 0.1 секунды
                    clearerr(stdin); // сбросить флаг EOF
                    continue;
                    }
                    putc(ch, stdout);
                    fflush(stdout); // если выводишь в терминал
                }
            }
        } else if (strcmp(argv[1], "2") == 0)
        {
            
            fd_wr = open("read_1.txt", O_WRONLY | O_CREAT | O_SYNC | O_TRUNC, 0644);
            fd_rd = open("write_1.txt", O_RDONLY);
            if(fd_wr < 0 || fd_rd < 0) {
                perror("Error while opening files");
                exit(1);
            }
            pid_t chpid = fork();
            if(chpid < 0) {
                perror("Error while forking");
                exit(1);
            }
            if(chpid > 0) 
            {
                // parent process
                dup2(fd_wr, 1);
                int ch = getc(stdin);
                while(ch != EOF) 
                {
                    putc(ch, stdout);
                    fflush(stdout);
                    ch = getc(stdin);
                }
            }
            else 
            {
                // child process
                dup2(fd_rd, STDIN_FILENO);
                while (1) {
                    int ch = getc(stdin);
                    if (ch == EOF) {
                    // Если файл временно пуст — подожди и попробуй снова
                    usleep(100000); // 0.1 секунды
                    clearerr(stdin); // сбросить флаг EOF
                    continue;
                    }
                    putc(ch, stdout);
                    fflush(stdout); // если выводишь в терминал
                }
            }
        } else 
        {
            printf("incorrect program number! (correcr 1 or 2)\n");
            return 0;
        }
    }
}
