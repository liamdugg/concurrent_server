#define _GNU_SOURCE
#include <signal.h>
#include <poll.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "../inc/HTTPRequest.h"

int main(void){
	
	const int aux = 1;
	int sockfd;
    struct sockaddr_in servaddr;
	//struct pollfd poll;
	
	char recv_str[4096];
	char req_str[] = "GET / HTTP/1.1\r\nContent-type:text/html\r\n\r\n<html></html>\n";
	
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &aux, sizeof(int)) < 0){
    	printf("Error en setsockopt()\n");
		return -1;
	}

    bzero(&servaddr, sizeof(servaddr));
 
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(8989);
 
    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }

	send(sockfd, req_str, strlen(req_str), 0);
	recv(sockfd, recv_str, sizeof(recv_str), 0);

	printf("\n%s\n", recv_str);

	while(1){
	}

	close(sockfd);
	return 0;
}