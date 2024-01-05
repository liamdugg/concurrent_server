/* --------------- INCLUDES --------------- */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <pthread.h>

#include <semaphore.h>

#include <netinet/in.h>
#include <arpa/inet.h>


#include "../inc/queue.h"

/* --------------- TYPEDEFS --------------- */
typedef struct sockaddr SA;
typedef struct sockaddr_in SA_IN;

typedef struct {
	pthread_t* thread;
} thread_t;

/* --------------- DEFINES --------------- */

// STATES
#define ST_WAIT_CLIENT	0
#define ST_WAIT_MSG		1
#define ST_RECV_MSG		2
#define POOL_SIZE		20
#define BACKLOG			20

#define BUF_SIZE		4096
#define SERVER_PORT		8989

/* --------------- PROTOTIPOS --------------- */
void* server_routine(void* arg);
void* producer_routine(void* arg);
void* consumer_routine(void* arg);
void* handle_connection(void* sock);
int create_socket(void);