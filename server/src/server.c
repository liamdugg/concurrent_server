#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include <string.h>
#include <signal.h>
#include <fcntl.h>

#include <pthread.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "../inc/server.h"
#include "../inc/HTTPRequest.h"
#include "../inc/HTTPResponse.h"

/* --------------- GLOBALES --------------- */

bool run = true;
server_t* server;
float* shm_consumer;
pthread_t producer_th;
struct epoll_event* sock_ev;

pthread_cond_t  shm_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t shm_mutex = PTHREAD_MUTEX_INITIALIZER;

/* --------------- FUNCIONES --------------- */

static int  server_create_socket(void);
static int  server_handle_connection(int sock);
static void server_get_config(server_t* server);
static void server_exit(int shm_id, int epollfd);
static int  get_sensor_values(int fd, float* store);

static int  socket_set_nonblocking(int sock);
static int  socket_add_to_epoll(int epollfd, int sockfd, uint32_t events);

int main(void){

	/* ------------------------------ VARIABLES ------------------------------ */
	
	// socket vars
	int cli_sock;
	sockaddr_in cli_addr;
	int addr_size = sizeof(sockaddr_in);

	// shared memory vars
	key_t key;								
	int shm_id;
	
	// epoll vars
	int epollfd, nfds; 						
	
	/* ------------------------------ SERVER INIT ------------------------------ */

	// levanto la configuracion
	server = (server_t*) malloc(sizeof(server_t));
	server_get_config(server);

	// creo socket del servidor
	if((server->sock = server_create_socket()) == -1){
		EXIT_SERVER("[Server] --> Error en server_create_socket()\n", -1);
	}

	signal(SIGUSR2, (__sighandler_t) sigusr2_handler); 			// creo signal
	signal(SIGINT,  (__sighandler_t)  sigint_handler);
	pthread_create(&producer_th, NULL, producer_routine, NULL);	// creo thread productor (lector del sensor)

	// Creo shared memory
	key = ftok("SHAREDMEM", 1);
	
	if((shm_id = shmget(key, sizeof(float)*2, 0666|IPC_CREAT)) < 0){ 
    	EXIT_SERVER("[Server] --> Error en shmget()\n", -1);
	}
	
	if((shm_consumer = (float*)shmat(shm_id, NULL, 0)) == NULL){
    	EXIT_SERVER("[Server] --> Error en shmat()\n", -1);
	}
	
	// creo epoll
	if((epollfd = epoll_create(1)) == -1){
		EXIT_SERVER("[Server] --> Error en epoll_create()\n", -1);
	}

	// agrego el server->sock a epoll
	socket_add_to_epoll(epollfd, server->sock, EPOLLIN | EPOLLRDHUP | EPOLLHUP);

	/* ------------------------------ SERVER LOOP ------------------------------ */

	int i;
	printf("[Server] --> Arrancando\n");
	
	while(run){

		// espero eventos
		nfds = epoll_wait(epollfd, sock_ev, server->max_conn, -1);

		for(i=0; i < nfds; i++){
			
			if(sock_ev[i].data.fd == server->sock){ // event en server->sock

				if(sock_ev[i].events & EPOLLIN){ // hay algo para leer, accept()
					
					if((cli_sock = accept(server->sock, (sockaddr*)&cli_addr, (socklen_t*)&addr_size)) == -1){
						printf("Error en accept()\n");
						continue;
					}
					
					// seteo cliente como nonblocking y lo agrego a epoll
					socket_set_nonblocking(cli_sock);
					socket_add_to_epoll(epollfd, cli_sock, EPOLLIN | EPOLLHUP | EPOLLRDHUP);

					if(server->cur_conn == 0){
						pthread_mutex_lock(&shm_mutex);
						pthread_cond_signal(&shm_cond);
						pthread_mutex_unlock(&shm_mutex);
					}

					//pthread_mutex_lock(&)
					
					server->cur_conn++;
					printf("[Server] --> Cliente %i conectado\n", sock_ev[i].data.fd);
				}
				
				else{ // cierro el server, hubo un error
					printf("[Server] --> Cerrando servidor\n");
					run = false;
					break;
				}
			}

			else if(sock_ev[i].events & EPOLLHUP || sock_ev[i].events & EPOLLRDHUP){ // cliente desconectado
				
				printf("[Server] --> Cliente %i desconectado\n", sock_ev[i].data.fd);
				epoll_ctl(epollfd, EPOLL_CTL_DEL, sock_ev[i].data.fd, NULL);
				server->cur_conn--;
				close(sock_ev[i].data.fd);
			}

			else if(sock_ev[i].events & EPOLLIN){ // evento en un socket cliente
				server_handle_connection(sock_ev[i].data.fd);
			}
			
		}
	}

	// exit, no deberia llegar hasta aca
	server_exit(shm_id, epollfd);
	return 0;
}

static int server_handle_connection(int sock){

	FILE* fp;
	HTTPRequest_t req;
	size_t bytes_read;

	char buf[BUF_SIZE];
	char file_str[BUF_SIZE];
	char response_str[BUF_SIZE];
	
	memset(buf, 0, sizeof(buf));
	memset(file_str, 0, sizeof(file_str));
	memset(response_str, 0, sizeof(response_str));

	/* --------------- --------------- */
	
	if((bytes_read = recv(sock, buf, sizeof(buf), 0)) < 0){
		EXIT_SERVER("[Server] --> Error en recv()\n", -1);
	}

	printf("\n-------------------- [CLI %i] REQ START --------------------\n", sock);
	printf("%s\n", buf);
		
	http_request_get(&req, buf);

	if(!strcmp(req.format, "csv")){ // GET Request de un dato

	    float bmp_data[2];

		pthread_mutex_lock(&shm_mutex);
		memcpy(bmp_data, shm_consumer, sizeof(bmp_data));
		pthread_mutex_unlock(&shm_mutex);

		printf("[Server][Consumer] --> Temperatura: %.1f °C\n", bmp_data[TEMP_INDEX]);
		printf("[Server][Consumer] --> Presion: %.2f hPa\n", bmp_data[PRES_INDEX]);

		http_response_sensor(bmp_data[TEMP_INDEX], bmp_data[PRES_INDEX], response_str);
		send(sock, response_str, strlen(response_str), 0);

		printf("[Server][Client %i] ENVIO DATO --> 200\n", sock);
	}

	else if(strcmp(req.method, HTTP_GET) == 0){ // GET request de .html / .css / .js

		if((fp = fopen(req.path, "r")) == NULL){ // Not Found

			if((fp = fopen("./sv_files/notfound.html", "r")) == NULL){
				EXIT_SERVER("[Server] --> Error en fopen(notfound.html)\n", -1);
			}

			http_response_not_found(fp, req.format, response_str);
			
			send(sock, response_str, strlen(response_str), 0);
			printf("[Server][Client %i] ENVIO --> 404\n", sock);
		}

		else { // status ok
			
			http_response_ok(fp, req.format, response_str);			
			send(sock, response_str, strlen(response_str), 0);
			
			printf("[Server][Client %i] ENVIO --> 200\n", sock);
		}

		fclose(fp);
		printf("\n-------------------- [CLI %i] REQ END --------------------\n", sock);
	}
	
	else if(strcmp(req.method, HTTP_POST) == 0){
		// TODO: implementar
	}

	else 
		return -1;

	return 0;
}

void* producer_routine(void* arg){
	
	key_t key;
	int shm_id;
	float* shm_producer;
	
	int bmp; // file descriptor del sensor

	// Creo shared memory
	key = ftok("SHAREDMEM", 1);

	if ((shm_id = shmget(key, 2*sizeof(float), 0666|IPC_CREAT)) < 0){ 
        EXIT_SERVER("[Server] --> Error en shmget()\n", NULL);
	}
		
	if((shm_producer = (float*)shmat(shm_id, NULL, 0)) == NULL){
       EXIT_SERVER("[Server] --> Error en shmat()\n", NULL);
	}

	//abro el sensor
	if((bmp = open("/dev/bmp180", O_RDWR)) ==  -1){
		EXIT_SERVER("[Server] --> Error en open(bmp)\n", NULL);
	}

	while(run){	

		pthread_mutex_lock(&shm_mutex);

		// si no hay conexiones activas suspendo el thread
		if(server->cur_conn == 0){ 
			printf("[Server][Producer] --> A dormir\n");
			pthread_cond_wait(&shm_cond, &shm_mutex);
			printf("[Server][Producer] --> Despierto\n");
		}
		
		if(server->cur_conn > 0)
			get_sensor_values(bmp, shm_producer);		

		pthread_mutex_unlock(&shm_mutex);
		
		// pongo a dormir 1 segundo para que no este leyendo todo el tiempo
		// TODO: hacer el tiempo entre datos ajustable (1seg, 2seg, 5seg, etc...)
		sleep(1);
	}

	close(bmp);
	shmdt(shm_producer);
	return NULL;
}

/* --------------- SIGNALS --------------- */

void sigusr2_handler(int a){
	
	FILE* fp;
	int aux, aux2;

	if((fp = fopen("./sv_files/config.txt", "r")) == NULL){
		printf("[Server] --> No se pudo abrir la nueva configuracion. \n");
		return;
	}

	else{
		fscanf(fp, "conexiones,%i\r\nbacklog,%i\r\npuerto,%i", &(server->max_conn), &(aux), &(aux2));
		fclose(fp);
	}

	sock_ev = (struct epoll_event*) realloc(sock_ev, server->max_conn * sizeof(struct epoll_event));
	printf("[Server] Nueva configuracion --> Conexiones maximas: %i\n", server->max_conn);
}

void sigint_handler(int a){
	
	run = false;

	if(server->cur_conn == 0){
		pthread_mutex_lock(&shm_mutex);
		pthread_cond_signal(&shm_cond);
		pthread_mutex_unlock(&shm_mutex);
	}

	printf("[Server] --> Cerrando servidor\n");
}

/* --------------- AUXILIARES --------------- */

static void server_get_config(server_t* server){
	
	FILE* config_file;

	if( (config_file = fopen("./sv_files/config.txt", "r")) != NULL){
		fscanf(config_file, "conexiones,%i\r\nbacklog,%i\r\npuerto,%i", &(server->max_conn), &(server->backlog), &(server->port));
		fclose(config_file);
	}

	else { // configuracion default
		server->port = DEFAULT_PORT;
		server->backlog = DEFAULT_BACKLOG;
		server->max_conn = DEFAULT_MAX_CON;
		printf("No se encontro archivo de configuracion, se setearon valores por default.\n");
	}

	sock_ev = (struct epoll_event*) malloc(sizeof(struct epoll_event)* server->max_conn);
	if(sock_ev == NULL){
		printf("NULL\n");
	}

	server->cur_conn = 0;

	printf("[Server] --> PID: %i\n", getpid());
	printf("[Server] --> Puerto: %i\n", server->port);
	printf("[Server] --> Backlog: %i\n", server->backlog);
	printf("[Server] --> Conexiones maximas: %i\n", server->max_conn);
}

static int server_create_socket(){

	int sock;
	sockaddr_in sv_addr;
	const int aux = 1;

	// creo socket del server
	if( (sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("[Server] --> Error en socket()\n");
		return -1;
	}

	// configuro como reutilizable (asi no molesta cuando corro el programa de nuevo muy rapido)
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &aux, sizeof(int)) < 0)
    	printf("[Server] --> Error en setsockopt()\n");

	// inicializo struct de address
	bzero((char*)&sv_addr, sizeof(struct sockaddr_in));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(DEFAULT_PORT);
	sv_addr.sin_addr.s_addr = INADDR_ANY;

	// bindeo socket del server a un puerto
	if (bind(sock, (sockaddr*)&sv_addr, sizeof(sv_addr)) != 0){
		printf("[Server] --> Error en bind()\n");
		return -1;
	}
	
	// configuro como no bloqueante
	if (socket_set_nonblocking(sock) != 0){
		printf("[Server] --> Error en set_nonblocking()\n");
		return -1;
	}
	
	// dejo al socket escuchando
	if(listen(sock, DEFAULT_BACKLOG) == -1){
		printf("[Server] --> Error en listen()\n");
		return -1;
	}

	return sock;
}

static int socket_set_nonblocking(int sock){
	
	if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK) == -1)
		return -1;

	return 0;
}

static int socket_add_to_epoll(int epollfd, int sockfd, uint32_t events){

	struct epoll_event event;
	event.events = events;
	event.data.fd = sockfd;

	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event) == -1){
		printf("[Server] --> Error en epoll_ctl_add()\n");
		return -1;
	}

	return 0;
}

static void server_exit(int shm_id, int epollfd){
	
	close(epollfd);
	close(server->sock);
	free(server);
	free(sock_ev);
	
	pthread_join(producer_th, NULL);
	shmdt(shm_consumer);
	shmctl(shm_id, IPC_RMID, NULL);
}

static int get_sensor_values(int fd, float* store){
	
	char data[6];
	float bmp_data[2];
	
	// PRIMERA LECTURA
	if(read(fd, data, sizeof(data)) == -1){
		perror("[Server --> No se pudo realizar la lectura.\n");
		return -1;
	}

	// armo las mediciones en float
	bmp_data[TEMP_INDEX] = (data[TEMP_BYTE_1] << 8) | data[TEMP_BYTE_0];
	bmp_data[TEMP_INDEX] = (float)(bmp_data[TEMP_INDEX]/10);
	
	bmp_data[PRES_INDEX] = (data[PRES_BYTE_3] << 24) | (data[PRES_BYTE_2] << 16) | (data[PRES_BYTE_1] << 8) | data[PRES_BYTE_0];
	bmp_data[PRES_INDEX] = (float)(bmp_data[PRES_INDEX]/100); // a hPa

	memcpy(store, bmp_data, sizeof(float)*2);
	return 0;
}