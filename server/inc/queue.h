#include <stdlib.h>

struct node{
	struct node* next;
	int *cli_sock;
}; 

typedef struct node node_t;

void q_push(int* cli_sock);
int* q_pop(void);