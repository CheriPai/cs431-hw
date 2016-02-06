#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


const int NUM_ARGS = 5;
const int MAX_BUFFER_SIZE = 1024;     // Maximum buffer size
sem_t *mutex;                         // Controls access to critical region
sem_t *empty;                         // Counts empty buffer slots
sem_t *full;                          // Counts full buffer slots
int **buffer;                         // Buffer shared by producer and consumer


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

        // Print message if buffer is full
        sem_getvalue(&full, &ifull);
        if(ifull+1 == MAX_BUFFER_SIZE) {
            printf("Buffer full\n");
        }
    }
}


// Consumes items
void *consumer() {
    int item;
    int iempty;
    int ifull;

    while(true) {
        sem_wait(&full);
        sem_wait(&mutex);
        sem_getvalue(&full, &ifull);
        item = buffer[ifull];
        sem_post(&mutex);
        sem_post(&empty);

        // Print message if buffer is empty
        sem_getvalue(&empty, &iempty);
        if(iempty+1 == MAX_BUFFER_SIZE) {
            printf("Buffer empty\n");
        }

        consume_item(item);
    }
}


int main(int argc, char **argv) {

    if(argc != NUM_ARGS) {
        printf("Number of arguments must be 5\n");
        exit(EXIT_FAILURE);
    }

    // Initialize values from command line arguments
    int numProducers = (int) strtol(argv[1], NULL, 10);
    int numConsumers = (int) strtol(argv[2], NULL, 10);
    int numBuffers = (int) strtol(argv[3], NULL, 10);
    int numItems = (int) strtol(argv[4], NULL, 10);

    // Initialize mutex, empty, and full for each buffer
    mutex = malloc(numBuffers * sizeof(sem_t));
    empty = malloc(numBuffers * sizeof(sem_t));
    full = malloc(numBuffers * sizeof(sem_t));
    buffer = malloc(numBuffers * sizeof(int *)); 

    // Initialize buffer and semaphore values
    for(int i = 0; i < numBuffers; ++i) {
        buffer[i] = malloc(MAX_BUFFER_SIZE * sizeof(int));
        sem_init(&mutex[i], 0, 1);
        sem_init(&empty[i], 0, MAX_BUFFER_SIZE);
        sem_init(&full[i], 0, 0);
    }

    // Initialize thread arrays
    pthread_t *producerThread = malloc(numProducers * sizeof(pthread_t));
    pthread_t *consumerThread = malloc(numConsumers * sizeof(pthread_t));

    // Generate seed for RNG in produce_item()
    srand(time(NULL));

    // Create threads for producers
    for(int i = 0; i < numProducers; ++i) {
        int ret = pthread_create(&producerThread[i], NULL, producer, (void *) NULL);
        if(ret) {
            fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
            exit(EXIT_FAILURE);
        }
    }

    // Create threads for consumers
    for(int i = 0; i < numConsumers; ++i) {
        int ret = pthread_create(&consumerThread[i], NULL, consumer, (void *) NULL);
        if(ret) {
            fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
            exit(EXIT_FAILURE);
        }
    }

    // Join producer threads
    for(int i = 0; i < numProducers; ++i) {
        pthread_join(producerThread[i], NULL);
    }

    // Join consumer threads
    for(int i = 0; i < numConsumers; ++i) {
        pthread_join(consumerThread[i], NULL);
    }

    return 0;
}