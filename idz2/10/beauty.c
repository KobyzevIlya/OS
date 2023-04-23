#include "common.h"

void clear_all() {
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    if (sem_close(result_sem_start) == -1) {
        perror("Incorrect close of result start semaphore");
    }
    if (sem_close(result_sem_end) == -1) {
        perror("Incorrect close of result end semaphore");
    }

    if (sem_unlink(result_sem_start_name) == -1) {
        perror("Incorrect unlink of result start semaphore");
    }
    if (sem_unlink(result_sem_end_name) == -1) {
        perror("Incorrect unlink of result end semaphore");
    }
}

void handle_sigint(int sig) {
    clear_all();

    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    // установка обработчика прерываний
    signal(SIGINT, handle_sigint);

    int number = atoi(argv[1]);

    char buffer[BUFFER_SIZE];

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
        perror("->Beauty: Can't create result start semaphore<-\n");
        exit(10);
    }

    if ((result_sem_end = sem_open(result_sem_end_name, O_CREAT, 0666, 0)) == 0) {
        perror("->Beauty: Can't create result end semaphore<-\n");
        exit(10);
    }

    printf("Beauty is ready to accept valentines\n");

    // цикл принятия валентинок
    for (int i = 0; i < number; ++i) {
        // чтение валентинки
        if (msgrcv(msgid, &msg, BUFFER_SIZE, 1, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        strcpy(buffer, msg.mtext);

        int num;
        char text[BUFFER_SIZE];
        sscanf(buffer, "%d: %[^\n]", &num, text);

        printf("Beauty received a message: %s from admirer №%d\n", text, num);
    }

    int chosen;
    printf("->Enter number of admirer you like:");
    scanf("%d", &chosen);

    printf("\n======>Beauty carefully considered everything and chose the number %d<======\n\n", chosen);

    // цикл отправки ответа
    for (int i = 0; i < number; ++i) {
        sem_post(result_sem_start);
        sem_wait(result_sem_end);

                // чтение номера поклонника
                int admirer_number;
                int result;
                if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
                    perror("msgrcv");
                    exit(1);
                }
                admirer_number = atoi(msg.mtext);

                // отправка ответа
                if (admirer_number == chosen) {
                    int result = 1;
                    snprintf(msg.mtext, BUFFER_SIZE, "%d", result);
                    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
                        perror("msgsnd");
                        exit(1);
                    }
                } else {
                    int result = 0;
                    snprintf(msg.mtext, BUFFER_SIZE, "%d", result);
                    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
                        perror("msgsnd");
                        exit(1);
                    }
                }

        sem_post(result_sem_start);
        sem_wait(result_sem_end);
    }

    clear_all();

    return 0;
}