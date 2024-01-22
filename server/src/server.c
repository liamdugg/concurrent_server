#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "../inc/server.h"
#include "../inc/HTTPRequest.h"
#include "../inc/HTTPResponse.h"

/* --------------- GLOBALES --------------- */

server_t* server; 

/* --------------- FUNCIONES --------------- */

static int  server_create_socket(void);
static int  server_handle_connection(int sock);
static void server_get_config(server_t* server);

static int  socket_set_nonblocking(int sock);
static int  socket_add_to_epoll(int epollfd, int sockfd, uint32_t events);

int main(void){

	// aux vars
	int i;
	bool run = true;
	
	// socket vars
	int cli_sock;
	sockaddr_in cli_addr;
	int addr_size = sizeof(sockaddr_in);
	
	// epoll vars
	int epollfd, nfds;
	struct epoll_event sock_ev[DEFAULT_MAX_CON];
	
	// levanto la configuracion
	server = (server_t*) malloc(sizeof(server_t));
	server_get_config(server);

	// creo socket del servidor
	if((server->sock = server_create_socket()) == -1){
		printf("[Server] --> Error en server_create_socket()\n");
		return -1;
	}

	// creo epoll
	if((epollfd = epoll_create(1)) == -1){
		printf("[Server] --> Error en epoll_create()\n");
		return -1;
	}

	// agrego el server->sock a epoll
	socket_add_to_epoll(epollfd, server->sock, EPOLLIN | EPOLLRDHUP | EPOLLHUP);

	/* --------------- SERVER LOOP --------------- */

	while(run == true){

		// espero eventos
		nfds = epoll_wait(epollfd, sock_ev, server->max_conn, -1);
		
		for(i=0; i < nfds; i++){
			
			if(sock_ev[i].data.fd == server->sock){ // event en server->sock

				if(sock_ev[i].events & EPOLLIN){ // hay algo para leer, accept()
					
					if((cli_sock = accept(server->sock, (sockaddr*)&cli_addr, (socklen_t*)&addr_size)) == -1){
						printf("[Server] --> Error en accept()\n");
						return -1;
					}

					// seteo cliente como nonblocking y lo agrego a epoll
					socket_set_nonblocking(cli_sock);
					socket_add_to_epoll(epollfd, cli_sock, EPOLLIN | EPOLLHUP | EPOLLRDHUP);
					
					server->cur_conn++;
					printf("[Server] --> Cliente conectado\n");
				}
				
				else{ // cierro el server, hubo un error
					run = false;
					break;
				}
			}

			else if(sock_ev[i].events & EPOLLIN){ // evento en un socket cliente
				server_handle_connection(sock_ev[i].data.fd);
			}
			
			else if(sock_ev[i].events & EPOLLHUP || sock_ev[i].events & EPOLLRDHUP){ // cliente desconectado
				
				printf("[Server] --> Cliente desconectado\n");
				epoll_ctl(epollfd, EPOLL_CTL_DEL, sock_ev[i].data.fd, NULL);
				server->cur_conn--;
				close(sock_ev[i].data.fd);
			}
		}
	}

	// exit, no deberia llegar hasta aca
	close(server->sock);
	close(epollfd);
	free(server);
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
		printf("[Server][Client %i] --> Error recibiendo\n", sock);
		return -1;
	}

	printf("\n-------------------- [CLI %i] REQ START --------------------\n", sock);
	printf("\n%s", buf);		
	
	http_request_get(&req, buf);
	
	if(strcmp(req.method, HTTP_GET) == 0){

		if((fp = fopen(req.path, "r")) == NULL){ // status not found

			if((fp = fopen("./sv_files/notfound.html", "r")) == NULL)
				return -1;
			
			// armo string			
			http_response_not_found(fp, req.format, response_str);
			
			// envio
			send(sock, response_str, strlen(response_str), 0);
			printf("[Server][Client %i] ENVIO --> 404\n", sock);
		}

		else { // status ok
			
			// armo string
			http_response_ok(fp, req.format, response_str);
			
			// envio
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

	server->cur_conn = 0;

	printf("Puerto: %i\n", server->port);
	printf("Backlog: %i\n", server->backlog);
	printf("Conexiones maximas: %i\n", server->max_conn);
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