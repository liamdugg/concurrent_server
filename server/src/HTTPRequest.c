#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include "../inc/HTTPRequest.h"

// GET /file.html HTTP/1.1\r\n			--> request_line
// Header 1\r\n							--> request_header
// Header 2\r\n							--> request_header
// Header n\r\n							--> request_header
// \r\n									--> empty line
// BODY									--> request_body

// Solo voy a parsear la primera linea para el server
// ya que lo que voy a realizar es bastante basico
// y no tiene sentido perder demasiado tiempo
// TODO: parsear headers
// TODO: parsear body (de ser necesario) 

int request_init(HTTPRequest_t* request,char* request_str){

	char* aux; 

	memset(request->path, 0, sizeof(request->path));
	memset(request->method, 0, sizeof(request->method));
	memset(request->version, 0, sizeof(request->version));

	// get method
	aux = strtok(request_str, " ");
	strcpy(request->method, aux);

	// get path
	aux = strtok(NULL, " ");
	
	if(strcmp(aux, "/") == 0){
		strcpy(request->path, "./sv_files/home.html");
	}

	else{
		strcpy(request->path, "./sv_files");
		strcat(request->path, aux);
	}

	// get version
	aux = strtok(NULL, "\r");
	strcpy(request->version, aux);

	// para debug
	if( (strcmp(request->method, HTTP_GET) != 0 && strcmp(request->method, HTTP_POST) !=0) || 
		 strcmp(request->version, HTTP_1_1) != 0){

			printf("invalid method or version\n\n");
			return -1;
	}

	return 0;
}