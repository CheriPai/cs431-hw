#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define FILEPATH "/tmp/mmapped.bin"
#define N  (100)
#define FILE_LENGTH (N * sizeof(int))

int count = 0;
int buffer[N];

int produce_item() {
    
    return (int) (((double)100) * rand() / (RAND_MAX + 1.0));
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
        if(count == 0){
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int i, fd, result;
    int *file_memory;  
    pid_t child;
    child = fork();
    char* program = "./MemMapConsum";;
    char* arglist[] = {"./MemMapConsum", "MemMapConsum", NULL} ;

    fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    lseek(fd, FILE_LENGTH+1, SEEK_SET);
    write(fd, "", 1);

    file_memory = (int*)mmap(0, FILE_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);


    if(child == 0){
        for (i = 1; i <=N; ++i) {
           file_memory[i] = produce_item(); 
        }
    } else {
        wait();
        execvp(program, arglist);
        consumer();
        
    }

    close(fd);
    munmap(file_memory, FILE_LENGTH);
    return 0;
}