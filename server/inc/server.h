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
	uint32_t max_conn;
	uint32_t cur_conn;
	uint32_t backlog;
	uint32_t port;
} server_t;

/* --------------- DEFINES --------------- */

#define BUF_SIZE			4096
#define DEFAULT_PORT		8989
#define DEFAULT_POOL_SIZE	100
#define DEFAULT_BACKLOG		20

/* --------------- PROTOTIPOS --------------- */

int   create_socket(void);
void  init_server(server_t* server);

// rutinas
void* server_routine(void* arg);
void* producer_routine(void* arg);
void* consumer_routine(void* arg);
void* handle_connection(void* sock);
