#include "common.h"

int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];

    printf("->Admirer №%d thinks about his valentine<-\n", getpid());
    printf("->Enter your valentine:");
    scanf("%s", buffer);

    key_t shm_key = ftok("common.h", 'e');
    if ((shmid = shmget(shm_key, BUFFER_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    shm_ptr = (char *) shmat(shmid, NULL, 0);
    if (shm_ptr == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    key_t start_key = ftok("common.h", 'a');
    start_sem = semget(start_key, 1, 0666 | IPC_CREAT);

    key_t end_key = ftok("common.h", 'b');
    end_sem = semget(end_key, 1, 0666 | IPC_CREAT);

    key_t result_start_key = ftok("common.h", 'c');
    result_sem_start = semget(result_start_key, 1, 0666 | IPC_CREAT);

    key_t result_end_key = ftok("common.h", 'd');
    result_sem_end = semget(result_end_key, 1, 0666 | IPC_CREAT);

    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_flg = 0;

    char valentine[BUFFER_SIZE + 20];
    sprintf(valentine, "%d: %s", getpid(), buffer);

    sb.sem_op = -1;
    semop(start_sem, &sb, 1);

        // отправка валентинки
        memcpy(shm_ptr, &valentine, strlen(valentine) + 1);

    sb.sem_op = 1;
    semop(end_sem, &sb, 1);

    sb.sem_op = -1;
    semop(result_sem_start, &sb, 1);
        // отправка своего номера
        pid_t pid = getpid();
        memcpy(shm_ptr, &pid, sizeof(int));

        sb.sem_op = 1;
        semop(start_sem, &sb, 1);
        sb.sem_op = -1;
        semop(end_sem, &sb, 1);

        // получение ответав
        int result;
        memcpy(&result, shm_ptr, sizeof(int));
        if (result) {
            printf("\n======>You are very happy<======\n\n");
        } else {
            printf("You will get drunk today\n");
        } 
    sb.sem_op = 1;
    semop(result_sem_end, &sb, 1);

    return 0;
}