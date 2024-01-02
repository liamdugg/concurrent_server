
/* --------------- --------------- */
/* --------------- INCLUDES --------------- */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>

// #include "bmp.h"

/* --------------- TYPEDEFS --------------- */

typedef struct {
	pid_t pid;
	int socket;
} child_t;

typedef struct {
	int port;
	int max_clients;
	int backlog;
} server_config_t;

typedef struct {
	server_config_t config;
	// WIP
} producer_t;

typedef struct {
	// WIP
} consumer_t;

/* --------------- DEFINES --------------- */
// STATES
#define ST_WAIT_CLIENT	0
#define ST_WAIT_MSG		1
#define ST_RECV_MSG		2

/* --------------- PROTOTIPOS --------------- */