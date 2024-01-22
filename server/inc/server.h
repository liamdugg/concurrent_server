
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

#define BUF_SIZE			4096
#define DEFAULT_PORT		8989
#define DEFAULT_MAX_CON		100
#define DEFAULT_BACKLOG		20

/* --------------- PROTOTIPOS --------------- */

