#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>


#define PORT "5050"
#define BACKLOG 2
#define N 100                   // Maximum buffer size
#define MAXDATASIZE 2 // max number of bytes we can get at once 
#define FULL_SEM "full_sem"
#define EMPTY_SEM "empty_sem"
#define LOCK_SEM "lock_sem"

sem_t full_sem;
sem_t empty_sem;
sem_t lock_sem;

char buf[MAXDATASIZE];
char buffer[N];
int sockfd, prod_fd, cons_fd; 
bool pending_consume, pending_produce;
int ind;
char *temp;
char *pAck;
char producer_buffer;

void consume() {
    return;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
void * listen_consumer()
{   
    int numbytes;
    int item;
    int ifull;
    while(1) {
        numbytes = recv(cons_fd, buf, MAXDATASIZE, 0);
        sem_getvalue(&full_sem, &ifull);
         if(ifull == 0)
        {
            //buffer was empty, can't consume, set flag that consume is waiting
            //no ack means consumer.c will block on no recv
            pending_consume = 1;
        }
        else
        {
            //send to consumer and decrement index
            //check pending_produce flag
            sem_wait(&full_sem);
            sem_wait(&lock_sem);
            sem_getvalue(&full_sem, &ifull);
            temp = &buffer[ifull];
            send(cons_fd, temp, 1, 0);
            if(pending_produce)
            {
                buffer[ifull] = producer_buffer;
                pending_produce = 0;
                send(prod_fd, pAck, 1, 0);
                sem_post(&full_sem);
            }
            sem_post(&lock_sem);
            sem_post(&empty_sem);
        }

    }
}

void * listen_producer()
{
    int numbytes;
    int ifull;
    while(1)
    {
        numbytes = recv(prod_fd, buf, MAXDATASIZE, 0);
        if(numbytes == 0 || numbytes == -1)
        {
            printf("Closing on producer%d", numbytes);
            close(prod_fd);
            close(cons_fd);
            return NULL;
        }

            sem_getvalue(&full_sem, &ifull);
            if( ifull+ 1 == N)
            {
                //set flag to produce on next consume and increment index
                pending_produce = 1;
                producer_buffer = buf[0];
                printf("index >= N\n");
            }
            else
            {
                //produce, increment index, and send ack
                //check pending_consume flag
                sem_wait(&empty_sem);
                sem_wait(&lock_sem);
                sem_getvalue(&full_sem, &ifull);
                buffer[ifull] = buf[0];
                send(prod_fd, pAck, 1, 0);
                if(pending_consume)
                {
                    temp = &buffer[ifull];
                    sem_post(&empty_sem);
                    send(cons_fd, temp, 1, 0);
                }                
                sem_post(&lock_sem);
                sem_post(&full_sem);
            }
    }
    return NULL;
}

int main(void)
{
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    pid_t producer_pid;
    pid_t consumer_pid;
    pending_produce = 0;
    pending_consume = 0;
    ind = 0;
    char ack = 'a';
    pAck = &ack;
    sem_init(&full_sem, 0, 0);
    sem_init(&empty_sem, 0, 100);
    sem_init(&lock_sem, 0, 1);
    pthread_t producer_thread, consumer_thread;
    char host_name[128];

    gethostname(host_name, sizeof host_name);

   char *arg_list_p[] = { "./producer", host_name, NULL};
   char *arg_list_c[] = {"./consumer", host_name, NULL};

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections...\n");


//exec producer
    producer_pid = fork();
    if(producer_pid == 0)
    {
        //this is child
        printf("starting producer");
        execvp("./producer", arg_list_p);     
        fprintf(stderr, "An error occured in execvp\n");
        abort();
    }


    //connect to producer
    sin_size = sizeof their_addr;
    prod_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (prod_fd == -1)
    {
        perror("accept");
        //continue;
    }
    inet_ntop(their_addr.ss_family,
    get_in_addr((struct sockaddr *)&their_addr),
        s, sizeof s);

   printf("server: connected to producer\n");

//exec consumer
    consumer_pid = fork();
    if(consumer_pid == 0)
    {
        //this is child
        printf("starting consumer");
        execvp("./consumer", arg_list_c);     
        fprintf(stderr, "An error occured in execvp\n");
        abort();
    }


    //connect to consumer
    sin_size = sizeof their_addr;
    sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (cons_fd == -1)
    {
        perror("accept");
    }
    inet_ntop(their_addr.ss_family,
    get_in_addr((struct sockaddr *)&their_addr),
        s, sizeof s);
    printf("server: connected to consumer\n");

    pthread_create(&producer_thread, NULL, &listen_producer, NULL);
    pthread_create(&consumer_thread, NULL, &listen_consumer, NULL);
    while(1){}
    close(prod_fd);
    close(cons_fd);
    close(sockfd);

    return 0;
}