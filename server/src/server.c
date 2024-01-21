#include "../inc/server.h"
#include "../inc/HTTPRequest.h"
#include "../inc/HTTPResponse.h"

/* --------------- GLOBALES --------------- */

server_t* server;
pthread_t thread_pool[DEFAULT_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t th_cond = PTHREAD_COND_INITIALIZER; // lets threads wait until something happens 

/* --------------- FUNCIONES --------------- */

int main(void){
	
	SA_IN cli_addr;
	int sv_sock, cli_sock;
	int addr_size = sizeof(SA_IN);

	// levanto la configuracion
	server = (server_t*) malloc(sizeof(server_t));
	init_server(server);

	// creo threads consumidores
	for(int i=0; i < DEFAULT_POOL_SIZE; i++)
		pthread_create(&thread_pool[i], NULL, consumer_routine, NULL);

	// creo socket del servidor
	if((sv_sock = create_server_socket()) == -1)
		return -1;

	// loop
	while(true){

		// espero conexiones en listen		
		printf("Esperando conexiones...\n");
		
		if((cli_sock = accept(sv_sock, (SA*)&cli_addr, (socklen_t*)&addr_size)) == -1){
			printf("Server --> Error en accept()\n");
			return -1;
		}

		pthread_mutex_lock(&mutex);
		
		// pusheo la conexion a la q
		server->cur_conn++;
		q_push(&cli_sock);
		
		// aviso que hay una conexion disponible
		pthread_cond_signal(&th_cond);
		pthread_mutex_unlock(&mutex);
	}

	free(server);
	return 0;
}

void* handle_connection(void* p_sock){

	int sock = *((int*) p_sock);
	
	HTTPRequest_t req;
	char response_str[BUF_SIZE];

	size_t bytes_read;
	char buf[BUF_SIZE];

	FILE* fp;
	char  file_str[BUF_SIZE];

	// lo uso como "thread id"
	int th_id = server->cur_conn;

	while(1){
		
		memset(buf, 0, BUF_SIZE);
		printf("\n[Server][TH %i] Esperando request\n", th_id);
			
		if((bytes_read = recv(sock, buf, sizeof(buf), 0)) < 0){
			printf("[Server][TH %i] --> Error recibiendo\n", th_id);
		}

		else if(bytes_read == 0){
			printf("[Server][TH %i] --> Cliente desconectado\n", th_id);
			break;
		}

		printf("\n---------- [TH %i] REQ START ----------\n", th_id);
		printf("\n%s", buf);		
		
		http_request_get(&req, buf);
		
		if(strcmp(req.method, HTTP_GET) == 0){ // status not found
			
			memset(response_str, 0, sizeof(response_str));


			if((fp = fopen(req.path, "r")) == NULL){
								
				fp = fopen("./sv_files/notfound.html", "r");

				// obtengo el string para enviar				
				http_response_not_found(fp, req.format, response_str);
				//printf("\n%s\n", response_str);
				
				// envio
				send(sock, response_str, strlen(response_str), 0);
				printf("[Server][TH %i] ENVIO --> 404\n", th_id);
				fclose(fp);
			}

			else { // status ok
				
				memset(file_str, 0, sizeof(file_str));
				memset(response_str, 0, sizeof(response_str));

				// obtengo el string para enviar
				http_response_ok(fp, req.format, response_str);
				//printf("\n%s\n", response_str);
				
				// envio respuesta
				send(sock, response_str, strlen(response_str), 0);
				printf("[Server][TH %i] ENVIO --> 200\n", th_id);
				fclose(fp);
			}

			printf("\n---------- [TH %i] REQ END ----------\n", th_id);
		}

		else if(strcmp(req.method, HTTP_POST) == 0){

		}

	}

	printf("[Server][TH %i] --> Cerrando conexion\n", th_id);
	server->cur_conn--;
	close(sock);
	return NULL;
}

/* --------------- RUTINAS DE THREADS --------------- */

void* consumer_routine(void* arg){

	int* cli = NULL;

	// loop
	while(true){
		
		// lockeo el acceso a la q de sockets
		pthread_mutex_lock(&mutex);
		
		// si no hay que atender, espero la cond_signal de que hay algo
		if((cli = q_pop()) == NULL){
			// esto ya hace el unlock del mutex
			pthread_cond_wait(&th_cond, &mutex);
			cli = q_pop();
		}
		
		// libero el mutex
		pthread_mutex_unlock(&mutex);
		
		// hay un socket que atender
		if(cli != NULL){
			handle_connection(cli);
			cli = NULL;
		}
	}

	// nunca deberia llegar hasta aca
	return NULL;
}

void* producer_routine(void* arg){

	// loop
	while(true){

	}
}

void* server_routine(void* arg){

	// loop
	while(true){

	}
}

/* --------------- AUXILIARES --------------- */

void init_server(server_t* server){
	
	FILE* config_file;
	
	if( (config_file = fopen("./sv_files/config.txt", "r")) != NULL){
		fscanf(config_file, "conexiones,%i\r\nbacklog,%i\r\npuerto,%i", &(server->max_conn), &(server->backlog), &(server->port));
		fclose(config_file);
	}

	else { // configuracion default
		server->port = DEFAULT_PORT;
		server->backlog = DEFAULT_BACKLOG;
		server->max_conn = DEFAULT_POOL_SIZE;
		printf("No se encontro archivo de configuracion, se setearon valores por default.\n");
	}

	server->cur_conn = 0;

	printf("Puerto: %i\n", server->port);
	printf("Backlog: %i\n", server->backlog);
	printf("Conexiones maximas: %i\n", server->max_conn);
}

int create_server_socket(){
	
	int sock;
	SA_IN sv_addr;
	const int aux = 1;

	// creo socket del server
	if( (sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("Server --> Error en socket()\n");
		return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &aux, sizeof(int)) < 0)
    	printf("Server --> Error en setsockopt()\n");

	// inicializo struct de address
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(DEFAULT_PORT);
	sv_addr.sin_addr.s_addr = INADDR_ANY;

	// bindeo socket del server a un puerto
	if(bind(sock, (SA*)&sv_addr, sizeof(sv_addr)) != 0){
		printf("Server --> Error en bind()\n");
		return -1;
	}

	// dejo al socket escuchando
	if(listen(sock, DEFAULT_BACKLOG) == -1){
		printf("Server --> Error en listen()\n");
		return -1;
	}

	return sock;
}