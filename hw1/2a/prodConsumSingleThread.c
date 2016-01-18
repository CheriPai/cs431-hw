#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define N 100                   // Maximum buffer size
int count = 0;                  // Current buffer size
int buffer[N];                  // Buffer shared by producer and consumer


int produce_item() {
    return rand();
}


void consume_item(int item) {
    return;
}


void producer() {
    int item;

    while(true) {
        if(count == N) {
            break;
        }
        item = produce_item();
        buffer[count++] = item;
    }
}


void consumer() {
    int item;

    while(true) {
        item = buffer[--count];
        consume_item(item);
        if(count == 0) {
            break;
        }
    }
}


int main() {
    srand(time(NULL));
    for(int i = 0; i < 10; ++i) {
        printf("Buffer empty. Staring producer\n");
        producer();
        printf("Buffer full. Starting consumer\n");
        consumer();
    }
    return 0;
}
