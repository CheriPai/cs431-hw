#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


const int NUM_ARGS = 5;
const int MAX_BUFFER_SIZE = 1024;     // Maximum buffer size
pthread_mutex_t *mutex;               // Controls access to critical region
pthread_mutex_t lock;                 // Locks read and write to counters
pthread_cond_t produced1000;          // Puts producer to sleep
pthread_cond_t buffersFull;           // Puts producer to sleep if full
pthread_cond_t buffersEmpty;          // Puts consumer to sleep if empty
int *empty;                           // Counts empty buffer slots
int *full;                            // Counts full buffer slots
int **buffer;                         // Buffer shared by producer and consumer
int totalProduced;
int lastProduced = 0;
bool bufferPrinterTerminated;


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


int getLargestBufferIndex(int *b, int numBuffers) {
    int max = 0;
    int index = 0;
    for(int i = 0; i < numBuffers; ++i) {
        if(b[i] > max) {
            max = b[i];
            index = i;
        }
    }
    return index;
}


// Produces items
void *producer(void *sizes) {
    int item;
    struct Sizes *s = (struct Sizes *) sizes;
    int bufferIndex;
    
    while(true) {
        // Stop thread if we have produced the desired amount
        if(totalProduced >= s->numItems) {
            printf("Producer Thread %d is finished\n", s->id);
            return NULL;
        }

        // Choose the emptiest buffer
        bufferIndex = getLargestBufferIndex(empty, s->numBuffers);

        // Wait if we have produced 1000 items
        pthread_mutex_lock(&lock);
        if(totalProduced % 1000 == 0 && totalProduced > 0 && 
           totalProduced < s->numItems && !bufferPrinterTerminated) {
            pthread_cond_wait(&produced1000, &lock);
        }

        // Wait if all the buffers are full
        while(empty[bufferIndex] == 0) {
            pthread_cond_wait(&buffersFull, &lock);
            bufferIndex = getLargestBufferIndex(empty, s->numBuffers);
        }

        item = produce_item();
        pthread_mutex_lock(&mutex[bufferIndex]);
        buffer[bufferIndex][full[bufferIndex]] = item;
        
        // Change counter values
        ++totalProduced;
        --empty[bufferIndex];
        ++full[bufferIndex];
        pthread_cond_broadcast(&buffersEmpty);
        pthread_mutex_unlock(&mutex[bufferIndex]);
        pthread_mutex_unlock(&lock);
    }
}


// Consumes items
void *consumer(void *sizes) {
    int item;
    struct Sizes *s = (struct Sizes *) sizes;
    int bufferIndex;

    while(true) {
        // Wait if all the buffers are empty
        pthread_mutex_lock(&lock);
        bufferIndex = getLargestBufferIndex(full, s->numBuffers);
        while(full[bufferIndex] == 0) {
            if(bufferPrinterTerminated) {
                printf("Consumer Thread %d is finished\n", s->id);
                pthread_mutex_unlock(&lock);
                return NULL;
            } else {
                printf("Consumer Thread %d is yielding\n", s->id);
                pthread_cond_wait(&buffersEmpty, &lock);
            }
            bufferIndex = getLargestBufferIndex(full, s->numBuffers);
        }

        pthread_mutex_lock(&mutex[bufferIndex]);
        item = buffer[bufferIndex][full[bufferIndex]];
        ++empty[bufferIndex];
        --full[bufferIndex];
        pthread_cond_broadcast(&buffersFull);
        pthread_mutex_unlock(&mutex[bufferIndex]);
        pthread_mutex_unlock(&lock);
        consume_item(item);
    }
}


// Prints status of buffers every 1000 items produced
void *bufferPrinter(void *sizes) {
    struct Sizes *s = (struct Sizes *) sizes;
    while(true) {
        pthread_mutex_lock(&lock);
        if(totalProduced % 1000 == 0 && totalProduced != lastProduced) {
            printf("%d items created\n", totalProduced);
            for(int i = 0; i < s->numBuffers; ++i) {
                printf("SharedBuffer%d has %d items\n", i, full[i]);
            }
            lastProduced = totalProduced;
            pthread_cond_broadcast(&produced1000);
        }
        if(totalProduced >= s->numItems) {
            // FIXME: Remove print statement
            bufferPrinterTerminated = true;
            pthread_cond_broadcast(&buffersEmpty);
            pthread_mutex_unlock(&lock);
            printf("Buffer printer is finished\n");
            return NULL;
        }
        pthread_mutex_unlock(&lock);
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
    mutex = malloc(numBuffers * sizeof(pthread_mutex_t));
    empty = malloc(numBuffers * sizeof(int));
    full = malloc(numBuffers * sizeof(int));
    buffer = malloc(numBuffers * sizeof(int *)); 

    // Initialize buffer and mutex
    for(int i = 0; i < numBuffers; ++i) {
        buffer[i] = malloc(MAX_BUFFER_SIZE * sizeof(int));
        empty[i] = MAX_BUFFER_SIZE;
        pthread_mutex_init(&mutex[i], NULL);
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
