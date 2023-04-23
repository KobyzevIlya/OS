#include "common.h"

int main() {
    char buffer[BUFFER_SIZE];

    printf("->Admirer №%d thinks about his valentine<-\n", getpid());
    printf("->Enter your valentine:");
    scanf("%s", buffer);

    key_t key;
    struct message msg;
    msg.mtype = 1;
    if ((key = ftok("common.h", 'a')) == -1) {
        perror("ftok");
        exit(1);
    }
    if ((msgid = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
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

    // отправка валентинки
    strcpy(msg.mtext, valentine);
    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }


    sem_wait(result_sem_start);
    // отправка своего номера
    pid_t pid = getpid();
    snprintf(msg.mtext, BUFFER_SIZE, "%d", pid);
    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    sem_post(result_sem_end);
    sem_wait(result_sem_start);

    // получение ответа
    int result;
    if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
    result = atoi(msg.mtext);

    if (result) {
        printf("\n======>You are very happy<======\n\n");
    } else {
        printf("You will get drunk today\n");
    } 
    sem_post(result_sem_end);

    return 0;
}
