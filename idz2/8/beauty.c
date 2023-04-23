#include "common.h"

void clear_all() {
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

    semctl(start_sem, 0, IPC_RMID);
    semctl(end_sem, 0, IPC_RMID);
    semctl(result_sem_start, 0, IPC_RMID);
    semctl(result_sem_end, 0, IPC_RMID);
}

void handle_sigint(int sig) {
    clear_all();

    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <number>\n", "common.h");
        return 1;
    }

    // установка обработчика прерываний
    signal(SIGINT, handle_sigint);

    int number = atoi(argv[1]);

    char buffer[BUFFER_SIZE];

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

    printf("Beauty is ready to accept valentines\n");

    // цикл принятия валентинок
    for (int i = 0; i < number; ++i) {
        sb.sem_op = 1;
        semop(start_sem, &sb, 1);
        sb.sem_op = -1;
        semop(end_sem, &sb, 1);

        // чтение валентинки
        memcpy(&buffer, shm_ptr, BUFFER_SIZE);

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
        sb.sem_op = 1;
        semop(result_sem_start, &sb, 1);
            sb.sem_op = -1;
            semop(start_sem, &sb, 1);

                // чтение номера поклонника
                int admirer_number;
                memcpy(&admirer_number, shm_ptr, sizeof(int));

                // отправка ответа
                if (admirer_number == chosen) {
                    int result = 1;
                    memcpy(shm_ptr, &result, sizeof(int));
                } else {
                    int result = 0;
                    memcpy(shm_ptr, &result, sizeof(int));
                }

            sb.sem_op = 1;
            semop(end_sem, &sb, 1);
        sb.sem_op = -1;
        semop(result_sem_end, &sb, 1);
    }

    clear_all();

    return 0;
}