#include "../inc/server.h"

// thread_t th_consumers[POOL_SIZE];
// thread_t th_server;
// thread_t th_producer;

pthread_t thread_pool[POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t th_cond = PTHREAD_COND_INITIALIZER;

// pthread_cond lets threads wait until something happens 
// and it can do useful work

int main(void){
	
	int sv_sock, cli_sock;
	SA_IN cli_addr;
	int addr_size;

	// creo los threads
	for(int i=0; i < POOL_SIZE; i++)
		pthread_create(&thread_pool[i], NULL, consumer_routine, NULL);

	// creo socket del servidor
	if((sv_sock = create_socket()) == -1)
		return -1;

	addr_size = sizeof(SA_IN);

	// loop
	while(true){

		// espero conexiones en listen		
		printf("Esperando conexiones...\n");
		
		if((cli_sock = accept(sv_sock, (SA*)&cli_addr, (socklen_t*)&addr_size)) == -1){
			printf("Server --> Error en accept()\n");
			return -1;
		}

		pthread_mutex_lock(&mutex);
		q_push(&cli_sock);
		pthread_cond_signal(&th_cond);
		pthread_mutex_unlock(&mutex);
	}

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
	sv_addr.sin_port = htons(SERVER_PORT);
	sv_addr.sin_addr.s_addr = INADDR_ANY;

	// bindeo socket del server a un puerto
	if(bind(sock, (SA*)&sv_addr, sizeof(sv_addr)) != 0){
		printf("Server --> Error en bind()\n");
		return -1;
	}

	// dejo al socket escuchando
	if(listen(sock, BACKLOG) == -1){
		printf("Server --> Error en listen()\n");
		return -1;
	}

	return sock;
}


// RUTINAS DE THREADS
void* consumer_routine(void* arg){

	// loop
	while(true){
		
		int* cli;
	
		pthread_mutex_lock(&mutex);
		
		if((cli = q_pop()) == NULL){
			pthread_cond_wait(&th_cond, &mutex);
			cli = q_pop();
		}

		pthread_mutex_unlock(&mutex);

		if(cli != NULL)
			handle_connection(cli);
	}
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