#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>

#define BUFFER_SIZE 256

int shm;
const char *shm_name = "shared_memory";
void *shm_ptr;

sem_t *start_sem;
const char *start_sem_name = "start_sem_name";
sem_t *end_sem;
const char *end_sem_name = "end_sem_name";
sem_t *result_sem_start;
const char *result_sem_start_name = "result_sem_start_name";
sem_t *result_sem_end;
const char *result_sem_end_name = "result_sem_end_name";

void clear_all() {
    if (munmap(shm_ptr, BUFFER_SIZE) == -1) {
        perror("Incorrect munmap");
    }
    if (shm_unlink(shm_name) == -1) {
        perror("Incorrect shm_unlink");
    }
    if (sem_close(start_sem) == -1) {
        perror("Incorrect close of start semaphore");
    }
    if (sem_close(end_sem) == -1) {
        perror("Incorrect close of end semaphore");
    }
    if (sem_close(result_sem_start) == -1) {
        perror("Incorrect close of result start semaphore");
    }
    if (sem_close(result_sem_end) == -1) {
        perror("Incorrect close of result end semaphore");
    }
    if (sem_unlink(start_sem_name) == -1) {
        perror("Incorrect unlink of start semaphore");
    }
    if (sem_unlink(end_sem_name) == -1) {
        perror("Incorrect unlink of end semaphore");
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

    // установка сида и обработчика прерываний
    srand(time(NULL));
    signal(SIGINT, handle_sigint);

    int number = atoi(argv[1]);

    int process;

    char buffer[BUFFER_SIZE];
    char message_template[] = "Written with love after much thought. Template written using freevalentines.com, remove this sentence before sending.";

    if ((shm = shm_open(shm_name, O_CREAT | O_RDWR, 0666)) == -1) {
        perror("->Can't create shared memory<-\n");
        exit(10);
    }

    if (ftruncate(shm, BUFFER_SIZE) == -1) {
        perror("->Can't truncate shared memory<-\n");
        exit(10);
    }
    
    if ((shm_ptr = mmap(0, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0)) == MAP_FAILED) {
        perror("->Can't mmap shared memory<-\n");
        exit(10);
    }

    if ((start_sem = sem_open(start_sem_name, O_CREAT, 0666, 0)) == 0) {
        perror("->Can't create start semaphore<-\n");
        exit(10);
    }

    if ((end_sem = sem_open(end_sem_name, O_CREAT, 0666, 0)) == 0) {
        perror("->Can't create end semaphore<-\n");
        exit(10);
    }

    if ((result_sem_start = sem_open(result_sem_start_name, O_CREAT, 0666, 0)) == 0) {
        perror("->Can't create result start semaphore<-\n");
        exit(10);
    }

    if ((result_sem_end = sem_open(result_sem_end_name, O_CREAT, 0666, 0)) == 0) {
        perror("->Can't create result end semaphore<-\n");
        exit(10);
    }

    // цикл поклонников
    for (int i = 0; i < number; ++i) {
        process = fork();
        if (process == 0) { // поклонник
            printf("->Admirer №%d thinks about his valentine<-\n", i + 1);

            char valentine[BUFFER_SIZE];
            sprintf(valentine, "Unique valentine №%d. %s", i + 1, message_template);

            sem_wait(start_sem);

                // отправка валентинки
                memcpy(shm_ptr, &valentine, strlen(valentine) + 1);

            sem_post(end_sem);

            sem_wait(result_sem_start);
                // отправка своего номера
                memcpy(shm_ptr, &i, sizeof(int));

                sem_post(start_sem);
                sem_wait(end_sem);

                // получение ответав
                int result;
                memcpy(&result, shm_ptr, sizeof(int));
                if (result) {
                    printf("\n======>Admirer №%d is very happy<======\n\n", i + 1);
                } else {
                    printf("Admirer №%d will get drunk today\n", i + 1);
                } 
            sem_post(result_sem_end);

            return 0;
        }
    }

    printf("Beauty is ready to accept valentines\n");

    // цикл принятия валентинок
    for (int i = 0; i < number; ++i) {
        sem_post(start_sem);
        sem_wait(end_sem);

        // чтение валентинки
        memcpy(&buffer, shm_ptr, BUFFER_SIZE);

        printf("Beauty received a message: %s\n", buffer);
    }

    int chosen = rand() % number;
    printf("\n======>Beauty carefully considered everything and chose the number %d<======\n\n", chosen + 1);

    // цикл отправки ответа
    for (int i = 0; i < number; ++i) {
        sem_post(result_sem_start);
            sem_wait(start_sem);

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

            sem_post(end_sem);
        sem_wait(result_sem_end);
    }

    clear_all();

    return 0;
}