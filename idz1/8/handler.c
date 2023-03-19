#include "common.h"

// функция, которая разворачивает слова, разделитель - любой символ, не являющийся буквой латинского алфавита
void reorder(char buffer[]) {
    char *start = buffer;
    char *end = buffer;
    while (*end != '\0') {
        while (*start != '\0' && !isalpha(*start))
        {
            start++;
        }
        end = start;
        while (*end != '\0' && isalpha(*end))
        {
            end++;
        }
        char *p = start;
        char *q = end - 1;
        while (p < q)
        {
            char temp = *p;
            *p++ = *q;
            *q-- = temp;
        }
        start = end;
    }
}

int main(int argc, char *argv[]) {
    char input_filename[256];  // имя файла для чтения
    char output_filename[256]; // имя файла для записи

    int input_descriptor;
    int output_descriptor;

    int fd1;
    int fd2;

    char buffer[BUF_SIZE]; // буфер

    (void)umask(0);

    if ((sem = sem_open(sem_name, O_CREAT, 0666, 0)) == 0) {
        perror("Reader: can't create admin semaphore");
        exit(10);
    }

    if (access(fd1_name, F_OK) == -1) {
        if (mknod(fd1_name, S_IFIFO | 0666, 0) < 0) {
            printf("->Reader: error while creating channel 1<-\n");
            exit(10);
        }
    }

    if (access(fd2_name, F_OK) == -1) {
        if (mknod(fd2_name, S_IFIFO | 0666, 0) < 0) {
            printf("->Reader: error while creating channel 2<-\n");
            exit(10);
        }
    }

    if ((fd1 = open(fd1_name, O_RDONLY)) < 0) { // открытие первого канала
        printf("->Handler: error with open FIFO 1 for reading<-\n");
        exit(10);
    }

    int size = read(fd1, buffer, BUF_SIZE);
    if (size < 0) {
        printf("->Handler: error with reading from pipe 1<-\n");
        exit(10);
    }

    reorder(buffer);

    if (close(fd1) != 0) { // закрытие канала
        printf("->Reader: error with closing fifo 1<-\n");
        exit(10);
    }

    if ((fd2 = open(fd2_name, O_WRONLY)) < 0) { // открытие второго канала
        printf("->Handler: error with open FIFO 1 for reading<-\n");
        exit(10);
    }

    size = write(fd2, buffer, size); // запись текста из буфера в канал
    if (size < 0) {
        printf("->Handler: error with writing<-\n");
        exit(10);
    }

    if (close(fd2) < 0) {
        printf("->Reader: error with closing fifo 2<-\n");
        exit(10);
    }

    printf("->Handler: exit<-\n");

    if (sem_post(sem) == -1) {
        printf("Handler: can't increment admin semaphore");
        exit(10);
    }

    if (sem_close(sem) == -1) {
        printf("Handler: incorrect close of busy semaphore");
        exit(10);
    }

    return 0;
}