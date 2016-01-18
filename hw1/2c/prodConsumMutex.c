#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define N 100                   // Maximum buffer size
sem_t mutex;                    // Controls access to critical region
sem_t empty;                    // Counts empty buffer slots
sem_t full;                     // Counts full buffer slots
int buffer[N];                  // Buffer shared by producer and consumer


// Produces an item by returning a random integer
int produce_item() {
    return rand();
}


// Dummy function used to "consume" items in buffer
void consume_item(int item) {
    return;
}


// Produces items
void *producer() {
    int item;
    int ifull;
    
    while(true) {
        item = produce_item();
        sem_wait(&empty);
        sem_wait(&mutex);
        sem_getvalue(&full, &ifull);
        buffer[ifull] = item;
        sem_post(&mutex);
        sem_post(&full);
        if(ifull+1 == 99) {
            printf("Buffer full\n");
        }
    }
}


// Consumers items
void *consumer() {
    int item;
    int iempty;

    while(true) {
        sem_wait(&full);
        sem_wait(&mutex);
        sem_getvalue(&empty, &iempty);
        item = buffer[iempty];
        sem_post(&mutex);
        sem_post(&empty);
        if(iempty+1 == 99) {
            printf("Buffer empty\n");
        }
        consume_item(item);
    }
}


int main() {
    int iret1, iret2;

    // Initialize semaphore values
    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, N);
    sem_init(&full, 0, 0);

    // Generate seed for RNG in produce_item()
    srand(time(NULL));

    // Create thread for producer
    pthread_t producerThread;
    iret1 = pthread_create(&producerThread, NULL, producer, (void *) NULL);
    if(iret1) {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
        exit(EXIT_FAILURE);
    }

    // Create thread for consumer
    pthread_t consumerThread;
    iret2 = pthread_create(&consumerThread, NULL, consumer, (void *) NULL);
    if(iret1) {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret2);
        exit(EXIT_FAILURE);
    }

    pthread_join(producerThread, NULL);
    pthread_join(consumerThread, NULL);

    return 0;
}
