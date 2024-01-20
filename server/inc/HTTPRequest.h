
#define HTTP_GET	"GET"
#define HTTP_POST	"POST"
#define HTTP_1_1	"HTTP/1.1"

#define HTTP_200	"HTTP/1.1 200 OK\r\n"
#define HTTP_404	"HTTP/1.1 404 Not Found\r\n"

typedef struct {
	
	char  method[8];
	char  path[30];
	char  version[9];

} HTTPRequest_t;

int request_init(HTTPRequest_t* request,char* request_str);