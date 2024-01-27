
/* --------------- TYPEDEFS --------------- */

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

typedef struct {

	int sock;
	uint32_t cur_conn;
	
	// config
	uint32_t max_conn;
	uint32_t backlog;
	uint32_t port;
} server_t;

/* --------------- DEFINES --------------- */

#define BMP_PATH 			"/dev/bmp180"

#define BUF_SIZE			4096
#define DEFAULT_PORT		8989
#define DEFAULT_MAX_CON		100
#define DEFAULT_BACKLOG		20

#define TEMP_INDEX			0
#define TEMP_BYTE_1 		0
#define TEMP_BYTE_0 		1

#define PRES_INDEX			1
#define PRES_BYTE_3 		2
#define PRES_BYTE_2 		3
#define PRES_BYTE_1 		4
#define PRES_BYTE_0 		5

#define EXIT_SERVER(x, y)	printf(x); run = false; return y; 

/* --------------- PROTOTIPOS --------------- */

void  sigusr2_handler(int a);
void  sigint_handler(int a);
void* producer_routine(void* arg);