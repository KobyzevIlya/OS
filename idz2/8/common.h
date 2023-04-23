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