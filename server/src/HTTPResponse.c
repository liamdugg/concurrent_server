#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "../inc/HTTPResponse.h"

static void get_header(char* head_str, char* file_type, int file_size);
static int get_file_size(FILE* fp);
static int get_file_content(FILE* fp, char* file_str);

int http_response_ok(FILE* fp, char* file_type, char * response_store){

	char file_str[4096];
	char header_str[100];

	int file_size = get_file_content(fp, file_str);
		
	get_header(header_str, file_type, file_size);

	strcpy(response_store, HTTP_200);
	strcat(response_store, header_str);
	//strcat(response_store, "\r\n");
	strcat(response_store, file_str);

	return 0;
}

int http_response_not_found(FILE* fp, char* file_type, char * response_store){

	char file_str[4096];
	char header_str[100];

	int file_size;
	
	file_size = get_file_content(fp, file_str);
	get_header(header_str, "html", file_size);

	strcat(response_store, HTTP_404);
	strcat(response_store, header_str);
	//strcat(response_store, "\r\n");
	strcat(response_store, file_str);

	return 0;
}

int http_response_sensor(float temp, float press, char* response_str){
	
	char time_str[30];
	char body_str[80];
	char header_str[100];

	time_t timer = time(NULL);
	struct tm* tm_time = localtime(&timer);
	sprintf(time_str, "%i/%i-%i:%i:%i", tm_time->tm_mday, tm_time->tm_mon+1, tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);

	sprintf(body_str, "%s,%.2f,%.2f", time_str, temp, press);
	get_header(header_str, "csv", strlen(body_str));
	
	strcpy(response_str, HTTP_200);
	strcat(response_str, header_str);
	strcat(response_str, body_str);
	
	return 0;
}

static int get_file_content(FILE *fp, char* file_str){

	int file_size = get_file_size(fp);

	while(!feof(fp)){
		fread(file_str, sizeof(char), file_size, fp);
	}

	return file_size;
}

static int get_file_size(FILE* fp){
	
	int size;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	return size;
}

static void get_header(char* head_str, char* file_type, int file_size){
	
	// hardcodeado
	// TODO: header date, header host
	sprintf(head_str, "Content-type:text/%s; charset=utf-8\r\nContent-length:%d\r\n\r\n", file_type, file_size);
}
