#include "common.h"

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

    // установка обработчика прерываний
    signal(SIGINT, handle_sigint);

    int number = atoi(argv[1]);

    char buffer[BUFFER_SIZE];

    if ((shm = shm_open(shm_name, O_CREAT | O_RDWR, 0666)) == -1) {
        perror("->Beauty: Can't create shared memory<-\n");
        exit(10);
    }

    if (ftruncate(shm, BUFFER_SIZE) == -1) {
        perror("->Beauty: Can't truncate shared memory<-\n");
        exit(10);
    }
    
    if ((shm_ptr = mmap(0, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0)) == MAP_FAILED) {
        perror("->Beauty: Can't mmap shared memory<-\n");
        exit(10);
    }

    if ((start_sem = sem_open(start_sem_name, O_CREAT, 0666, 0)) == 0) {
        perror("->Beauty: Can't create start semaphore<-\n");
        exit(10);
    }

    if ((end_sem = sem_open(end_sem_name, O_CREAT, 0666, 0)) == 0) {
        perror("->Beauty: Can't create end semaphore<-\n");
        exit(10);
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
        sem_post(start_sem);
        sem_wait(end_sem);

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