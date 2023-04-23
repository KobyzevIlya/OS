#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>

#define BUFFER_SIZE 256

int shmid;
char *shm_ptr;

int start_sem;
int end_sem;
int result_sem_start;
int result_sem_end;


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
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    // установка сида и обработчика прерываний
    srand(time(NULL));
    signal(SIGINT, handle_sigint);

    int number = atoi(argv[1]);

    int process;

    char buffer[BUFFER_SIZE];
    char message_template[] = "Written with love after much thought. Template written using freevalentines.com, remove this sentence before sending.";
    
    if ((shmid = shmget(IPC_PRIVATE, BUFFER_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    shm_ptr = (char *) shmat(shmid, NULL, 0);
    if (shm_ptr == (char *) -1) {
        perror("shmat");
        exit(1);
    }
    
    key_t start_key = ftok(argv[0], 'a');
    start_sem = semget(start_key, 1, 0666 | IPC_CREAT);
    semctl(start_sem, 0, SETVAL, 0);

    key_t end_key = ftok(argv[0], 'b');
    end_sem = semget(end_key, 1, 0666 | IPC_CREAT);
    semctl(end_sem, 0, SETVAL, 0);

    key_t result_start_key = ftok(argv[0], 'c');
    result_sem_start = semget(result_start_key, 1, 0666 | IPC_CREAT);
    semctl(result_sem_start, 0, SETVAL, 0);

    key_t result_end_key = ftok(argv[0], 'd');
    result_sem_end = semget(result_end_key, 1, 0666 | IPC_CREAT);
    semctl(result_sem_end, 0, SETVAL, 0);

    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_flg = 0;

    // цикл поклонников
    for (int i = 0; i < number; ++i) {
        process = fork();
        if (process == 0) { // поклонник
            printf("->Admirer №%d thinks about his valentine<-\n", i + 1);

            char valentine[BUFFER_SIZE];
            sprintf(valentine, "Unique valentine №%d. %s", i + 1, message_template);

            sb.sem_op = -1;
            semop(start_sem, &sb, 1);

                // отправка валентинки
                memcpy(shm_ptr, &valentine, strlen(valentine) + 1);

            sb.sem_op = 1;
            semop(end_sem, &sb, 1);

            sb.sem_op = -1;
            semop(result_sem_start, &sb, 1);
                // отправка своего номера
                memcpy(shm_ptr, &i, sizeof(int));

                sb.sem_op = 1;
                semop(start_sem, &sb, 1);
                sb.sem_op = -1;
                semop(end_sem, &sb, 1);

                // получение ответав
                int result;
                memcpy(&result, shm_ptr, sizeof(int));
                if (result) {
                    printf("\n======>Admirer №%d is very happy<======\n\n", i + 1);
                } else {
                    printf("Admirer №%d will get drunk today\n", i + 1);
                } 
            sb.sem_op = 1;
            semop(result_sem_end, &sb, 1);

            return 0;
        }
    }

    printf("Beauty is ready to accept valentines\n");

    // цикл принятия валентинок
    for (int i = 0; i < number; ++i) {
        sb.sem_op = 1;
        semop(start_sem, &sb, 1);
        sb.sem_op = -1;
        semop(end_sem, &sb, 1);

        // чтение валентинки
        memcpy(&buffer, shm_ptr, BUFFER_SIZE);

        printf("Beauty received a message: %s\n", buffer);
    }

    int chosen = rand() % number;
    printf("\n======>Beauty carefully considered everything and chose the number %d<======\n\n", chosen + 1);

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