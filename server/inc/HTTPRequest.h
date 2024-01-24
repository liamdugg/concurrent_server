#define HTTP_GET	"GET"
#define HTTP_POST	"POST"
#define HTTP_1_1	"HTTP/1.1"

typedef struct {
	
	char path[30];
	char format[12];
	char method[8];
	char version[9];

} HTTPRequest_t;

int http_request_get(HTTPRequest_t* request,char* request_str);