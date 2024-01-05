#include "../inc/queue.h"

node_t* head = NULL;
node_t* tail = NULL;

void q_push(int* cli_sock){

	node_t *new_node = malloc(sizeof(node_t));

	new_node->cli_sock = cli_sock;
	new_node->next = NULL;

	if(tail == NULL)
		head = new_node;

	else
		tail->next = new_node;
	
	tail = new_node;
}

int* q_pop(){


	if(head == NULL) // if empty
		return NULL;
	
	else{ // if not empty

		int* res = head->cli_sock;
		node_t *temp = head;
		head = head->next;

		if(head == NULL)
			tail = NULL;
		
		free(temp);
		return res;
	}
}