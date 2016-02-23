#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h> 
#include <math.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h> 


#define FULL_SEM "full_sem"
#define EMPTY_SEM "empty_sem"
#define LOCK_SEM "lock_sem"
#define N 100

int main()
{
  sem_t *full = sem_open(FULL_SEM, O_CREAT, 0644, 0); //tell how full the buffer is
  sem_t *empty = sem_open(EMPTY_SEM, O_CREAT, 0644, N); //tell how empty the buffer is
  sem_t *lock = sem_open(LOCK_SEM, O_CREAT, 0644, 1); // control access to critical region
  pid_t producer_pid;
  pid_t consumer_pid;
  int segment_id;    //ID to Shared Memory Segment
  int status;
  //ALLOCATE SHARED MEMORY SEGMENT
  segment_id = shmget(IPC_PRIVATE, N * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  const int ARG_SIZE = (int)((ceil(log10(segment_id))+1)*sizeof(char));
  char argument[ARG_SIZE];
  sprintf(argument, "%d", segment_id);
  char *arg_list_p[] = { "./producer", argument, NULL};
  char *arg_list_c[] = { "consumer", argument, NULL};
  

  producer_pid = fork();
  if (producer_pid != 0)
  {
    //this is the parent
  }
  else
  {
      execvp("./producer", arg_list_p);     
      fprintf(stderr, "An error occured in execvp\n");
      abort();
  }
  consumer_pid = fork();
  if(consumer_pid != 0)
  {
    //this is the parent
  }
  else
  {
    execvp("./consumer", arg_list_c);
    fprintf(stderr, "An error occured in execvp\n");
    abort();
}

waitpid(producer_pid, &status, WUNTRACED | WCONTINUED);
waitpid(consumer_pid, &status, WUNTRACED | WCONTINUED);

sem_unlink(FULL_SEM);
sem_unlink(EMPTY_SEM);
sem_unlink(LOCK_SEM);

return 0;
}
