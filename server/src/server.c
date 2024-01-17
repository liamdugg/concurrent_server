#include "../inc/server.h"

/* --------------- GLOBALES --------------- */

pthread_t thread_pool[DEFAULT_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t th_cond = PTHREAD_COND_INITIALIZER; // lets threads wait until something happens 

/* --------------- FUNCIONES --------------- */

int main(void){
	
	SA_IN cli_addr;
	server_t* server;
	int sv_sock, cli_sock;
	int addr_size = sizeof(SA_IN);

	// levanto la configuracion
	server = (server_t*)malloc(sizeof(server_t));
	init_server(server);

	// creo threads consumidores
	for(int i=0; i < DEFAULT_POOL_SIZE; i++)
		pthread_create(&thread_pool[i], NULL, consumer_routine, NULL);

	// creo socket del servidor
	if((sv_sock = create_socket()) == -1)
		return -1;

	// loop
	while(true){

		// espero conexiones en listen		
		printf("Esperando conexiones...\n");
		
		if((cli_sock = accept(sv_sock, (SA*)&cli_addr, (socklen_t*)&addr_size)) == -1){
			printf("Server --> Error en accept()\n");
			return -1;
		}

		server->cur_conn++;

		pthread_mutex_lock(&mutex);
		q_push(&cli_sock);
		pthread_cond_signal(&th_cond);
		pthread_mutex_unlock(&mutex);
	}

	free(server);
	return 0;
}

void* handle_connection(void* p_sock){

	int sock = *((int*) p_sock);
	size_t bytes_read;

	char buf[BUF_SIZE];
	memset(buf, 0, BUF_SIZE);
	
	if((bytes_read = recv(sock, buf, sizeof(buf), 0)) < 0){
		printf("Server --> Error recibiendo\n");
	}

	printf("Client --> %s\n", buf);
	fflush(stdout);

	memset(buf, 0, BUF_SIZE);
	strcpy(buf, "Se recibio el mensaje correctamente\n");
	
	send(sock,buf, strlen(buf), 0);
	printf("Server --> cerrando conexion...\n");
	close(sock);
	return NULL;
}

/* --------------- RUTINAS DE THREADS --------------- */

void* consumer_routine(void* arg){

	// loop
	while(true){
		
		int* cli;

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
		if(cli != NULL)
			handle_connection(cli);
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
	
	if( (config_file = fopen("./config.txt", "r")) != NULL){
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

int create_socket(){
	
	int sock;
	SA_IN sv_addr;

	// creo socket del server
	if( (sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("Server --> Error en socket()\n");
		return -1;
	}

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