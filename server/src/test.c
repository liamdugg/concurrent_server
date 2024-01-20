#include "../inc/HTTPRequest.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void){
	
	HTTPRequest_t req;
	
	char req_str[] = "GET /home.html HTTP/1.1\r\nContent-type:text/html\r\nContent-length\r\n\r\n<html></html>\n";

	request_init(&req, req_str);

	printf("METHOD  --> %s | length: %li\n", req.method, strlen(req.method));
	printf("PATH    --> %s | length: %li\n", req.path, strlen(req.path));
	printf("VERSION --> %s | length: %li\n", req.version, strlen(req.version));
	
	request_free(&req);
	return 0;
}