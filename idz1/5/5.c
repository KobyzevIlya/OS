#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/stat.h>

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

    char fd1_name[] = "fd1";
    char fd2_name[] = "fd2";
    int fd1;
    int fd2;

    int process1; // id первого процесса
    int process2; // id второго процесса

    char buffer[BUF_SIZE]; // буфер

    umask(0);

    strcpy(input_filename, argv[1]); // получение имен файлов из командной строки
    strcpy(output_filename, argv[2]);

    if (mknod(fd1_name, S_IFIFO | 0666, 0) < 0) {
        printf("->Reader: error while creating channel<-\n");
        exit(10);
    }

    process1 = fork(); // первый форк
    if (process1 < 0) {
        printf("->Reader: can't fork process1<-\n");
        exit(10);
    } else if (process1 > 0) { //parent
        // открытие файла для чтения
        if ((input_descriptor = open(input_filename, O_RDONLY)) == -1) {
            printf("->Error with opening input file\n");
            exit(10);
        }
        
        int size = read(input_descriptor, buffer, BUF_SIZE); // считывание текста из файла в буфер
        if (size < 0) {
            printf("->Reader: error with reading file<-\n");
            exit(10);
        }

        if ((fd1 = open(fd1_name, O_WRONLY)) < 0) {
            printf("->Reader: error with open FIFO for writing<-\n");
            exit(10);
        }

        size = write(fd1, buffer, size);
        if (size < 0) {
            printf("->Reader: error with writing<-\n");
            exit(10);
        }
        
        if (close(fd1) != 0) {
            printf("->Reader: error with closing fifo<-\n");
            exit(10);
        }

        // закрытие файла
        if (close(input_descriptor) != 0) {
            printf("->Error with closing input file<-\n");
            exit(10);
        }
        
        wait(NULL); // ожидание дочернего процесса
        printf("->Reader: exit<-\n");
    } else { // child
        if (mknod(fd2_name, S_IFIFO | 0666, 0) < 0) {
            printf("->Handler: error while creating channel<-\n");
            exit(10);
        }

        process2 = fork(); // второй форк

        if (process2 < 0) {
            printf("->Handler: can't fork process1<-\n");
            exit(10);
        } else if (process2 > 0) { //child-parent
            if ((fd1 = open(fd1_name, O_RDONLY)) < 0) {
                printf("->Handler: error with open FIFO for reading<-\n");
                exit(10);
            }

            int size = read(fd1, buffer, BUF_SIZE);
            if (size < 0) {
                printf("->Handler: error with reading file<-\n");
                exit(10);
            }

            if (close(fd1) != 0) {
                printf("->Handler: error with closing fifo 1<-\n");
                exit(10);
            }

            reorder(buffer); // обработка текста

            if ((fd2 = open(fd2_name, O_WRONLY)) < 0) {
                printf("->Handler: error with open FIFO for writing<-\n");
                exit(10);
            }

            size = write(fd2, buffer, size);
            if (size < 0) {
                printf("->Handler: error with writing<-\n");
                exit(10);
            }

            if (close(fd2) != 0) {
                printf("->Handler: error with closing fifo 2<-\n");
                exit(10);
            }
            
            wait(NULL);
            printf("->Handler: exit<-\n");
        } else { //child-child
            if ((fd2 = open(fd2_name, O_RDONLY)) < 0) {
                printf("->Writer: error with open FIFO for reading<-\n");
                exit(10);
            }

            int size = read(fd2, buffer, BUF_SIZE);
            if (size < 0) {
                printf("->Writer: error with reading file<-\n");
                exit(10);
            }

            if (close(fd2) != 0) {
                printf("->Writer: error with closing fifo<-\n");
                exit(10);
            }
            
            // открытие файла для записи. Если его нет, он будет создан. Если он уже существует, содержимое удалится
            if ((output_descriptor = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
                printf("->Error with opening output file\n");
                exit(10);
            }
            
            write(output_descriptor, buffer, strlen(buffer)); // запись текста из буфера в файл
            
            // закрытие файлов
            if (close(output_descriptor) != 0) {
                printf("->Error with closing output file<-\n");
                exit(10);
            }

            printf("->Writer: exit<-\n");
        }
        remove(fd1_name);
        remove(fd2_name);
    }

    return 0;
}