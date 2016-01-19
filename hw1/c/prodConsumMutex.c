#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define N 100                   // Maximum buffer size
pthread_mutex_t count_mutex;	// Locks the counter
int count;						// Array Position/Counter
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
    
    while(true) {
        item = produce_item();
        pthread_mutex_lock(&count_mutex);
        if(count < N)
        	buffer[count++] = item;
        else
        	printf("Buffer full\n");
        pthread_mutex_unlock(&count_mutex);
    }
}


// Consumes items
void *consumer() {
    int item;
 
    while(true) {
        pthread_mutex_lock(&count_mutex);
        if(count > 0){
        	item = buffer[--count];
        	consume_item(item);
        }
        else{
        	printf("Buffer empty\n");
        }
	    pthread_mutex_unlock(&count_mutex);
    }
}


int main() {
    int iret1, iret2;

    count = 0;

    // Initialize Mutex with default attributes
   	pthread_mutex_init(&count_mutex, NULL);

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
