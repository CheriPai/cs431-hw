
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define N 100

int count = 0;
int buffer[N];

int produce_item() {
    
    return (int) (((double)100) * rand() / (RAND_MAX + 1.0));
}

void consume_item(int item) {
    return;
}

void producer(FILE *write)
{

  int i,res;
  for(i = 1; i <= N; i++) {
    res = produce_item();
    fprintf(write, "%d ", res);
    buffer[count++] = res;
  }
  fclose(write);
  exit(0);
}


void consumer(FILE *read)
{
  int n,i,item;

  while(1) {
    int n = fscanf(read, "%d", &i);
    item = buffer[--count];
    consume_item(item);
    if(n == 1) {
      printf("%d\n", i);
    } else { 
      break;
    }
  }
  fclose(read);
  exit(0);
}

int main()
{
  srand(time(NULL));
  int fds[2];
  pid_t prod, con;

  pipe(fds);

  FILE *write, *read;

  prod = fork();
  con = fork();

  read  = fdopen(fds[0], "r");
  write = fdopen(fds[1], "w");
  
  if(prod == 0) {
    fclose(read);
    producer(write);
  }

  if(con == 0) {
    fclose(write);
    consumer(read);
  }

  fclose(read);
  fclose(write);

  wait(NULL);
  wait(NULL);

  return 0;

}