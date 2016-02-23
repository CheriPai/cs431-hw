#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdlib.h>
#include <fcntl.h>

#define FULL_SEM "full_sem"
#define EMPTY_SEM "empty_sem"
#define LOCK_SEM "lock_sem"
#define N 100

sem_t *full_sem;
sem_t *empty_sem;
sem_t *lock_sem;
int *buffer;

// Produces an item by returning a random integer
int produce_item() {
    return rand();
}

// Produces items
void producer() {

    int item;
    int ifull;
    int i = 0;
    
    while(i < 500) {
        item = produce_item();
        sem_wait(empty_sem);
        sem_wait(lock_sem);
        sem_getvalue(full_sem, &ifull);
        buffer[ifull] = item;
        sem_post(lock_sem);
        sem_post(full_sem);

        // Print message if buffer is full
        sem_getvalue(full_sem, &ifull);
        if(ifull+1 == N) {
            printf("Buffer full\n");
        }
        i++;
    }
}

int main(int argc, char * argv[])
{
    // thread_sleep(5);
    full_sem = sem_open(FULL_SEM, O_RDWR);
    empty_sem = sem_open(EMPTY_SEM, O_RDWR);
    lock_sem = sem_open(LOCK_SEM, O_RDWR);   //grab the semaphores
   // printf("%s\n", argv[1]);
    buffer = (int *)shmat(atoi(argv[1]), NULL, 0);
    producer();
    shmdt(buffer);

    return 0;


}