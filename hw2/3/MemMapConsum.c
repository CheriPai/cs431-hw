#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FILEPATH "/tmp/mmapped.bin"
#define N (100)
#define FILE_LENGTH (N * sizeof(int))

int main(int argc, char *argv[])
{
    int i, fd;
    int *file_memory; 

    fd = open(FILEPATH, O_RDONLY);
   
    file_memory = (int*)mmap(0, FILE_LENGTH, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	
    
    for (i = 1; i <=N; ++i) {
	printf("%d\n", file_memory[i]);
    }

    munmap(file_memory, FILE_LENGTH);
    return 0;
}