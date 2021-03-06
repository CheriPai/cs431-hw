#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define FULL_SEM "full_sem"
#define EMPTY_SEM "empty_sem"
#define LOCK_SEM "lock_sem"
#define N 100

sem_t *full_sem;
sem_t *empty_sem;
sem_t *lock_sem;
int *buffer;

// Dummy function used to "consume" items in buffer
void consume_item(int item) {
    return;
}

// Consumes items
void consumer() {
    int item;
    int iempty;
    int ifull;
    int i = 0;

    while(i < 500) {
        sem_wait(full_sem);
        sem_wait(lock_sem);
        sem_getvalue(full_sem, &ifull);
        item = buffer[ifull];
        consume_item(item);
        sem_post(lock_sem);
        sem_post(empty_sem);

        // Print message if buffer is empty
        sem_getvalue(empty_sem, &iempty);
        if(iempty+1 == N) {
            printf("Buffer empty\n");
        }


        i++;
    }
}

int main(int argc, char * argv[])
{
    full_sem = sem_open(FULL_SEM, O_RDWR);
    empty_sem = sem_open(EMPTY_SEM, O_RDWR);
    lock_sem = sem_open(LOCK_SEM, O_RDWR);   //grab the semaphores
    buffer = (int *)shmat(atoi(argv[1]), NULL, 0);

    consumer();
    shmdt(buffer);

    return 0;
}
