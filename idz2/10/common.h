#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_SIZE 256

struct message {
   long mtype;
   char mtext[BUFFER_SIZE];
};
int msgid;

sem_t *result_sem_start;
const char *result_sem_start_name = "result_sem_start_name";
sem_t *result_sem_end;
const char *result_sem_end_name = "result_sem_end_name";