#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>

#define BUF_SIZE 5000

// функция, которая разворачивает слова, разделитель - любой символ, не являющийся буквой латинского алфавита
void reorder(char buffer[]) {
    char *start = buffer;
    char *end = buffer;
    while (*end != '\0') {
        while (*start != '\0' && !isalpha(*start)) {
            start++;
        }
        end = start;
        while (*end != '\0' && isalpha(*end)) {
            end++;
        }
        char *p = start;
        char *q = end - 1;
        while (p < q) {
            char temp = *p;
            *p++ = *q;
            *q-- = temp;
        }
        start = end;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("->Incorrect number of arguments<-\n");
        exit(10);
    }
    
    char input_filename[256]; // имя файла для чтения
    char output_filename[256]; // имя файла для записи

    int input_descriptor;
    int output_descriptor;

    int fd1[2]; // первый неименованный канал
    int fd2[2]; // второй неименованный канал

    int process1; // id первого процесса
    int process2; // id второго процесса

    char buffer[BUF_SIZE]; // буфер

    strcpy(input_filename, argv[1]); // получение имен файлов из командной строки
    strcpy(output_filename, argv[2]);

    // открытие файла для чтения
    if ((input_descriptor = open(input_filename, O_RDONLY)) == -1) {
        printf("->Error with opening input file\n");
        exit(10);
    }

    // открытие файла для записи. Если его нет, он будет создан. Если он уже существует, содержимое удалится
    if ((output_descriptor = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
        printf("->Error with opening output file\n");
        exit(10);
    }

    if (pipe(fd1) < 0) { // создание первого канала
        printf("->Reader: can't open pipe<-\n");
        exit(10);
    }

    // теперь при вызове read() на пустом канале процесс не заблокируется
    int flags_pipe1;
    if ((flags_pipe1 = fcntl(fd1[0], F_GETFL, 0)) == -1) {
        printf("->Reader: can't get pipe flags<-\n");
        exit(10);
    }
    if (fcntl(fd1[0], F_SETFL, flags_pipe1 | O_NONBLOCK) == -1) {
        printf("->Reader: can't set pipe flags<-\n");
        exit(10);
    }

    process1 = fork(); // первый форк
    if (process1 < 0) {
        printf("->Reader: can't fork process1<-\n");
        exit(10);
    } else if (process1 > 0) { //parent
        if (close(fd1[0]) < 0) {
            printf("->Reader: can't close reading side of pipe<-\n");
            exit(10);
        }

        int size = read(input_descriptor, buffer, BUF_SIZE); // считывание текста из файла в буфер
        if (size < 0 && !(errno == EAGAIN || errno == EWOULDBLOCK)) { // errno - случай когда входной файл и канал пустые
            printf("->Reader: error with reading file<-\n");
            exit(10);
        }

        if (size < 0) size = 0;
        write(fd1[1], buffer, size); // запись текста из буфера в канал
        if (size < 0) {
            printf("->Reader: error with writing<-\n");
            exit(10);
        }

        if (close(fd1[1]) < 0) {
            printf("->Reader: can't close writing side of pipe<-\n");
            exit(10);
        }
        
        wait(NULL); // ожидание дочернего процесса
        printf("->Reader: exit<-\n");
    } else { // child
        if (pipe(fd2) < 0) { // открытие второго канала
            printf("->Handler: can't open pipe<-\n");
            exit(10);
        }
        // теперь при вызове read() на пустом канале процесс не заблокируется
        int flags_pipe2;
        if ((flags_pipe2 = fcntl(fd2[0], F_GETFL, 0)) == -1) {
            printf("->Handler: can't get pipe flags<-\n");
            exit(10);
        }
        if (fcntl(fd2[0], F_SETFL, flags_pipe2 | O_NONBLOCK) == -1) {
            printf("->Handler: can't set pipe flags<-\n");
            exit(10);
        }

        process2 = fork(); // второй форк

        if (process2 < 0) {
            printf("->Handler: can't fork process1<-\n");
            exit(10);
        } else if (process2 > 0) { //child-parent
            if (close(fd1[1]) < 0) {
                printf("->Handler: can't close writing side of parent pipe<-\n");
                exit(10);
            }

            int size = read(fd1[0], buffer, BUF_SIZE); // считывание текста из канала в буфер
            if (size < 0 && !(errno == EAGAIN || errno == EWOULDBLOCK)) { // errno - случай когда входной файл и канал пустые
                printf("->Handler: error with reading pipe<-\n");
                exit(10);
            }

            if (close(fd1[0]) < 0) {
                printf("->Handler: can't close reading side of parent pipe<-\n");
                exit(10);
            }

            reorder(buffer); // обработка текста

            if (close(fd2[0]) < 0) {
                printf("->Handler: can't close reading side of pipe<-\n");
                exit(10);
            }

            if (size < 0) size = 0;
            write(fd2[1], buffer, size); // запись текста из буфера во второй канал
            if (size < 0) {
                printf("->Handler: error with writing<-\n");
                exit(10);
            }

            if (close(fd2[1]) < 0) {
                printf("->Handler: can't close writing side of pipe<-\n");
                exit(10);
            }
            
            wait(NULL);
            printf("->Handler: exit<-\n");
        } else { //child-child
            if (close(fd2[1]) < 0) {
                printf("->Writer: can't close writing side of pipe<-\n");
                exit(10);
            }

            int size = read(fd2[0], buffer, BUF_SIZE); // считывание текста из канала в буфер
            if (size < 0) {
                printf("->Writer: error with reading pipe<-\n");
                exit(10);
            }

            if (close(fd2[0]) < 0) {
                printf("->Writer: can't close reading side of pipe<-\n");
                exit(10);
            }

            write(output_descriptor, buffer, strlen(buffer)); // запись текста из буфера в файл
            
            // закрытие файлов
            if (close(input_descriptor) != 0) {
                printf("->Error with closing input file<-\n");
                exit(10);
            }

            if (close(output_descriptor) != 0) {
                printf("->Error with closing output file<-\n");
                exit(10);
            }

            printf("->Writer: exit<-\n");
        }
    }

    return 0;
}