#define HTTP_200	"HTTP/1.1 200 OK\r\n"
#define HTTP_404	"HTTP/1.1 404 Not Found\r\n"

int http_response_sensor(float temp, float press, char* response_str);
int http_response_ok(FILE* fp, char* file_type, char* response_store);
int http_response_not_found(FILE* fp, char* file_type, char* response_store);