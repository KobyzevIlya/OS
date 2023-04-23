#include "common.h"

int main() {
    char buffer[BUFFER_SIZE];

    // ввод валентинки
    printf("->Admirer №%d thinks about his valentine<-\n", getpid());
    printf("->Enter your valentine:");
    scanf("%s", buffer);

    // создание семафоров и разделяемой памяти
    if ((shm = shm_open(shm_name, O_CREAT | O_RDWR, 0666)) == -1) {
        perror("->Admirer: Can't create shared memory<-\n");
        exit(10);
    }

    if (ftruncate(shm, BUFFER_SIZE) == -1) {
        perror("->Admirer: Can't truncate shared memory<-\n");
        exit(10);
    }
    
    if ((shm_ptr = mmap(0, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0)) == MAP_FAILED) {
        perror("->Admirer: Can't mmap shared memory<-\n");
        exit(10);
    }

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
        memcpy(shm_ptr, &valentine, strlen(valentine) + 1);

    sem_post(end_sem);

    sem_wait(result_sem_start);
        // отправка своего номера
        pid_t pid = getpid();
        memcpy(shm_ptr, &pid, sizeof(int));

        sem_post(start_sem);
        sem_wait(end_sem);

        // получение ответав
        int result;
        memcpy(&result, shm_ptr, sizeof(int));
        if (result) {
            printf("\n======>You are very happy<======\n\n");
        } else {
            printf("You will get drunk today\n");
        } 
    sem_post(result_sem_end);

    return 0;
}