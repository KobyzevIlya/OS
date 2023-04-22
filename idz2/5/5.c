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
#define SHM_SIZE 4096

int shm;
const char *shm_name = "shared_memory";
void *shm_ptr;

int shm_semaphores;
const char *shm_semaphores_name = "shared_memory_semaphores";
sem_t* semaphores;


void clear_all() {
    if (munmap(shm_ptr, BUFFER_SIZE) == -1) {
        perror("Incorrect munmap");
    }
    if (shm_unlink(shm_name) == -1) {
        perror("Incorrect shm_unlink");
    }
    if (munmap(semaphores, SHM_SIZE) == -1) {
        perror("Incorrect munmap");
    }
    if (shm_unlink(shm_semaphores_name) == -1) {
        perror("Incorrect shm_unlink");
    }
    sem_destroy(&semaphores[0]);
    sem_destroy(&semaphores[1]);
    sem_destroy(&semaphores[2]);
    sem_destroy(&semaphores[3]);
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

    /*
    0 = start
    1 = end
    2 = result_start
    3 = result_end
    */

    if ((shm_semaphores = shm_open(shm_semaphores_name, O_CREAT | O_RDWR, 0666)) == -1) {
        perror("->Can't create shared memory<-\n");
        exit(10);
    }

    if (ftruncate(shm_semaphores, SHM_SIZE) == -1) {
        perror("->Can't truncate shared memory<-\n");
        exit(10);
    }

    if ((semaphores = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_semaphores, 0)) == MAP_FAILED) {
        perror("->Can't mmap shared memory<-\n");
        exit(10);
    }

    sem_init(&semaphores[0], 1, 0);
    sem_init(&semaphores[1], 1, 0);
    sem_init(&semaphores[2], 1, 0);
    sem_init(&semaphores[3], 1, 0); 

    // цикл поклонников
    for (int i = 0; i < number; ++i) {
        process = fork();
        if (process == 0) { // поклонник
            printf("->Admirer №%d thinks about his valentine<-\n", i + 1);

            char valentine[BUFFER_SIZE];
            sprintf(valentine, "Unique valentine №%d. %s", i + 1, message_template);

            sem_wait(&semaphores[0]);
                // отправка валентинки
                memcpy(shm_ptr, &valentine, strlen(valentine) + 1);

            sem_post(&semaphores[1]);

            sem_wait(&semaphores[2]);
                // отправка своего номера
                memcpy(shm_ptr, &i, sizeof(int));

                sem_post(&semaphores[0]);
                sem_wait(&semaphores[1]);

                // получение ответав
                int result;
                memcpy(&result, shm_ptr, sizeof(int));
                if (result) {
                    printf("\n======>Admirer №%d is very happy<======\n\n", i + 1);
                } else {
                    printf("Admirer №%d will get drunk today\n", i + 1);
                } 
            sem_post(&semaphores[3]);

            return 0;
        }
    }

    printf("Beauty is ready to accept valentines\n");

    // цикл принятия валентинок
    for (int i = 0; i < number; ++i) {
        sem_post(&semaphores[0]);
        sem_wait(&semaphores[1]);

        // чтение валентинки
        memcpy(&buffer, shm_ptr, BUFFER_SIZE);

        printf("Beauty received a message: %s\n", buffer);
    }

    int chosen = rand() % number;
    printf("\n======>Beauty carefully considered everything and chose the number %d<======\n\n", chosen + 1);

    // цикл отправки ответа
    for (int i = 0; i < number; ++i) {
        sem_post(&semaphores[2]);
            sem_wait(&semaphores[0]);

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

            sem_post(&semaphores[1]);
        sem_wait(&semaphores[3]);
    }

    clear_all();

    return 0;
}