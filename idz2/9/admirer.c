#include "common.h"

int main() {
    char buffer[BUFFER_SIZE];

    printf("->Admirer №%d thinks about his valentine<-\n", getpid());
    printf("->Enter your valentine:");
    scanf("%s", buffer);

    mkfifo(fifo, 0666);
    fd = open(fifo, O_RDWR);

    if ((start_sem = sem_open(start_sem_name, O_CREAT, 0666, 0)) == 0) {
        perror("->Admirer: Can't create start semaphore<-\n");
        exit(10);
    }

    if ((end_sem = sem_open(end_sem_name, O_CREAT, 0666, 0)) == 0) {
        perror("->Admirer: Can't create end semaphore<-\n");
        exit(10);
    }

    if ((result_sem_start = sem_open(result_sem_start_name, O_CREAT, 0666, 0)) == 0) {
        perror("->Admirer: Can't create result start semaphore<-\n");
        exit(10);
    }

    if ((result_sem_end = sem_open(result_sem_end_name, O_CREAT, 0666, 0)) == 0) {
        perror("->Admirer: Can't create result end semaphore<-\n");
        exit(10);
    }

    char valentine[BUFFER_SIZE + 20];
    sprintf(valentine, "%d: %s", getpid(), buffer);

    sem_wait(start_sem);

    // отправка валентинки
    write(fd, valentine, strlen(valentine) + 1);

    sem_post(end_sem);

    sem_wait(result_sem_start);
    // отправка своего номера
    pid_t pid = getpid();
    write(fd, &pid, sizeof(int));

    sem_post(start_sem);
    sem_wait(end_sem);

    // получение ответа
    int result;
    read(fd, &result, sizeof(int));
    if (result) {
        printf("\n======>You are very happy<======\n\n");
    } else {
        printf("You will get drunk today\n");
    } 
    sem_post(result_sem_end);

    return 0;
}
