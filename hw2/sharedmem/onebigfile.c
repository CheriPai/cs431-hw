#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h> 
#include <math.h>
#include <stdlib.h>
#include <sys/wait.h>

#define FULL_SEM "full_sem"
#define EMPTY_SEM "empty_sem"
#define LOCK_SEM "lock_sem"
#define N 100

sem_t *full_sem;
sem_t *empty_sem;
sem_t *lock_sem;

// Dummy function used to "consume" items in buffer
void consume_item(int item) {
    return;
}


// Produces an item by returning a random integer
int produce_item() {
    return rand();
}


int main()
{
  full_sem = sem_open(FULL_SEM, O_CREAT, 0644, 0); //tell how full the buffer is
  empty_sem = sem_open(EMPTY_SEM, O_CREAT, 0644, N); //tell how empty the buffer is
  lock_sem = sem_open(LOCK_SEM, O_CREAT, 0644, 0); // control access to critical region
  pid_t producer_pid;
  pid_t consumer_pid;
  int segment_id;    //ID to Shared Memory Segment


  

   
  const int shared_segment_size= 0x28;    //bytes allocate rounded up to integer multip of page size
  //ALLOCATE SHARED MEMORY SEGMENT
  segment_id = shmget(IPC_PRIVATE, N * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  const int ARG_SIZE = (int)((ceil(log10(segment_id))+1)*sizeof(char));
  char argument[ARG_SIZE];
  sprintf(argument, "%d", segment_id);
  char *arg_list_p[] = { "./producer", argument, NULL};
  char *arg_list_c[] = { "consumer", argument, NULL};
  char *arg_list_ls[] = {"/", NULL};
  

  producer_pid = fork();
  if (producer_pid != 0)
  {
    //this is the parent
    printf("%d", producer_pid);
  }
  else
  {

    full_sem = sem_open(FULL_SEM, 0);
    empty_sem = sem_open(EMPTY_SEM, 0);
    lock_sem = sem_open(LOCK_SEM, 0);   //grab the semaphores

    int *buffer = (int *)shmat(segment_id, NULL, 0);
    int item;
    int ifull;
    
    while(1) {
        item = produce_item();
        sem_wait(empty_sem);
        sem_wait(lock_sem);
        sem_getvalue(full_sem, &ifull);
        buffer[ifull] = item;
        sem_post(lock_sem);
        sem_post(full_sem);

        // Print message if buffer is full
        sem_getvalue(full_sem, &ifull);
        if(ifull+1 == N) {
            printf("Buffer full\n");
        }
    }
     
  }
  consumer_pid = fork();
  if(consumer_pid != 0)
  {
    //this is the parent
  }
  else
  {

        full_sem = sem_open(FULL_SEM, 0);
    empty_sem = sem_open(EMPTY_SEM, 0);
    lock_sem = sem_open(LOCK_SEM, 0);   //grab the semaphores

    int  *buffer = (int *)shmat(segment_id, NULL, 0);
        int item;
    int iempty;
    int ifull;

    while(1) {
        sem_wait(full_sem);
        sem_wait(lock_sem);
        sem_getvalue(full_sem, &ifull);
        item = buffer[ifull];
        sem_post(lock_sem);
        sem_post(empty_sem);

        // Print message if buffer is empty
        sem_getvalue(empty_sem, &iempty);
        if(iempty+1 == N) {
            printf("Buffer empty\n");
        }

        consume_item(item);
    }
  }

  wait(producer_pid);
  wait(consumer_pid);

  return 0;





  //char* shared_memory;    //Starting Address of Shared Memory  Recall char = 1 byte
  //struct shmid_ds shmbuffer;
  //int *array[5];
  //int segment_size;


  // //ATTACH SHARED MEMORY SEGMENT
  // shared_memory = (char*) shmat (segment_id, 0, 0);
  // printf("shared memory attached at address %p\n", shared_memory);


  // //DETERMINE SHARED MEMORY SEGMENT SIZE
  // shmctl (segment_id, IPC_STAT, &shmbuffer);
  // segment_size = shmbuffer.shm_segsz;
  // printf( "Shared Memory Segment Size = %d\n", segment_size);


  // //WRITE STRING TO SHARED MEMORY SEGMENT  --> Using "SPRINTF"
  // sprintf (shared_memory, "Hello CS431!");


  // //DETACH THE SHARED MEMORY SEGMENT  
  // shmdt(shared_memory);


  // //REATTACH SHARED MEMORY SEGMENT, AT A DIFFERENT ADDRESS!!!   //Specify Page Address in process Adress Space, 0x50000, to attach the shared memory
  // shared_memory = (char*)shmat(segment_id,(void*)0x50000,0);
  // printf("shared memory reattached at address %p\n", shared_memory);

  
  // //PRINT OUT STRING FROM SHARED MEMORY
  // printf( "%s\n", shared_memory);
  
  // //DETACH SHARED MEMORY
  // shmdt(shared_memory);

  // //DEALLOCATE THE SHARED MEMORY SEGMENT   ****(Don't Forget This Step!!!!)****
  // shmctl(segment_id, IPC_RMID, 0);

  return 0;
}
