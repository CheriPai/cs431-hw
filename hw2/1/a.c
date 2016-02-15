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
sem_t maxProduction;
sem_t totalProduced;
sem_t bufferPrinterTerminated;


// Holds parameters passed to producer and consumer function
struct Sizes {
    int numBuffers;
    int numItems;
    int id;
};


// Produces an item by returning a random integer
int produce_item() {
    return rand();
}


// Dummy function used to "consume" items in buffer
void consume_item(int item) {
    return;
}


int getLargestBufferIndex(sem_t *b, int numBuffers) {
    int max = 0;
    int index = 0;
    for(int i = 0; i < numBuffers; ++i) {
        int v;
        sem_getvalue(&b[i], &v);
        if(v > max) {
            max = v;
            index = i;
        }
    }
    return index;
}


// Produces items
void *producer(void *sizes) {
    int item;
    int ifull;
    int produced;
    struct Sizes *s = (struct Sizes *) sizes;
    int bufferIndex;

    
    while(true) {
        // Stop thread if we have produced the desired amount
        sem_getvalue(&totalProduced, &produced);
        if(produced >= s->numItems) {
            printf("Producer Thread %d is finished\n", s->id);
            return NULL;
        }

        // Choose the emptiest buffer
        bufferIndex = getLargestBufferIndex(empty, s->numBuffers);

        // Wait if we have produced 1000 items
        sem_wait(&maxProduction);
        item = produce_item();

        sem_wait(&empty[bufferIndex]);
        sem_wait(&mutex[bufferIndex]);
        sem_getvalue(&full[bufferIndex], &ifull);
        buffer[bufferIndex][ifull] = item;
        sem_post(&totalProduced);
        sem_post(&mutex[bufferIndex]);
        sem_post(&full[bufferIndex]);
    }
}


// Consumes items
void *consumer(void *sizes) {
    int item;
    int ifull;
    int totalFull;
    int produced;
    struct Sizes *s = (struct Sizes *) sizes;
    int bufferIndex;

    while(true) {
        // Choose the fullest buffer
        bufferIndex = getLargestBufferIndex(full, s->numBuffers);

        int terminated;
        sem_getvalue(&bufferPrinterTerminated, &terminated);
        sem_getvalue(&full[bufferIndex], &ifull);
        // Calculate the total amount of items in all buffers
        totalFull = 0;
        for(int i = 0; i < s->numBuffers; ++i) {
            sem_getvalue(&full[i], &ifull);
            totalFull += ifull;
        }

        sem_getvalue(&totalProduced, &produced);
        if(terminated && totalFull == 0) {
            printf("Consumer Thread %d is finished\n", s->id);
            return NULL;
        } else if(totalFull == 0) {
            printf("Consumer Thread %d is yielding\n", s->id);
        }
        if(sem_trywait(&full[bufferIndex]) == 0) {
            sem_wait(&mutex[bufferIndex]);
            sem_getvalue(&full[bufferIndex], &ifull);
            item = buffer[bufferIndex][ifull];
            sem_post(&mutex[bufferIndex]);
            sem_post(&empty[bufferIndex]);
            consume_item(item);
        }
    }
}


// Prints status of buffers every 1000 items produced
void *bufferPrinter(void *sizes) {
    int production;
    int produced;
    struct Sizes *s = (struct Sizes *) sizes;
    while(true) {
        sem_getvalue(&maxProduction, &production);
        sem_getvalue(&totalProduced, &produced);
        if(produced % 1000 == 0) {
            printf("%d items created\n", produced);
            for(int i = 0; i < s->numBuffers; ++i) {
                int items;
                sem_getvalue(&full[i], &items);
                printf("SharedBuffer%d has %d items\n", i, items);
            }
            for(int i = 0; i < 1000; ++i) {
                sem_post(&maxProduction);
            }
            if(produced >= s->numItems) {
                // FIXME: Remove print statement
                sem_post(&bufferPrinterTerminated);
                printf("Buffer printer is finished\n");
                return NULL;
            }
        }
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

    // Initialize struct to pass to threads
    struct Sizes *producerSizes = malloc(numProducers * sizeof(struct Sizes));
    struct Sizes *consumerSizes = malloc(numConsumers * sizeof(struct Sizes));

    // Initialize mutex, empty, and full for each buffer
    mutex = malloc(numBuffers * sizeof(sem_t));
    empty = malloc(numBuffers * sizeof(sem_t));
    full = malloc(numBuffers * sizeof(sem_t));
    buffer = malloc(numBuffers * sizeof(int *)); 

    // Initialize buffer and semaphore values
    sem_init(&maxProduction, 0, 1000);
    sem_init(&totalProduced, 0, 0);
    sem_init(&bufferPrinterTerminated, 0, 0);
    for(int i = 0; i < numBuffers; ++i) {
        buffer[i] = malloc(MAX_BUFFER_SIZE * sizeof(int));
        sem_init(&mutex[i], 0, 1);
        sem_init(&empty[i], 0, MAX_BUFFER_SIZE);
        sem_init(&full[i], 0, 0);
    }

    // Initialize thread arrays
    pthread_t *producerThread = malloc(numProducers * sizeof(pthread_t));
    pthread_t *consumerThread = malloc(numConsumers * sizeof(pthread_t));
    pthread_t bufferPrinterThread;

    // Generate seed for RNG in produce_item()
    srand(time(NULL));

    // Initializes parameter structs for producer and consumer threads
    for(int i = 0; i < numProducers; ++i) {
        producerSizes[i].numBuffers = numBuffers;
        producerSizes[i].numItems = numItems;
        producerSizes[i].id = i;
    }
    for(int i = 0; i < numConsumers; ++i) {
        consumerSizes[i].numBuffers = numBuffers;
        consumerSizes[i].numItems = numItems;
        consumerSizes[i].id = i;
    }

    // Create threads for producers
    for(int i = 0; i < numProducers; ++i) {
        int ret = pthread_create(&producerThread[i], NULL, producer, &producerSizes[i]);
        if(ret) {
            fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
            exit(EXIT_FAILURE);
        }
    }

    // Create threads for consumers
    for(int i = 0; i < numConsumers; ++i) {
        int ret = pthread_create(&consumerThread[i], NULL, consumer, &consumerSizes[i]);
        if(ret) {
            fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
            exit(EXIT_FAILURE);
        }
    }

    // Initialize parameters for buffer printer thread
    struct Sizes s;
    s.numBuffers = numBuffers;
    s.numItems = numItems;

    // Create thread for buffer printer
    int ret = pthread_create(&bufferPrinterThread, NULL, bufferPrinter, &s);
    if(ret) {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    // Join producer threads
    for(int i = 0; i < numProducers; ++i) {
        pthread_join(producerThread[i], NULL);
    }

    // Join consumer threads
    for(int i = 0; i < numConsumers; ++i) {
        pthread_join(consumerThread[i], NULL);
    }

    // Join buffer printer thread
    pthread_join(bufferPrinterThread, NULL);

    // Free up dynamically allocated memory
    free(mutex);
    free(empty);
    free(full);
    free(producerThread);
    free(consumerThread);
    free(producerSizes);
    free(consumerSizes);
    for(int i = 0; i < numBuffers; ++i) {
        free(buffer[i]);
    }
    free(buffer);

    return 0;
}
