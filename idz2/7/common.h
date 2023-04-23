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