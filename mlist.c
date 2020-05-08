#include "mlist.h"
#include <stdio.h>
#include <stdlib.h>
/*
* This is a list with moves that is used in the agent (client.c)
*/

//creates an empty list
void initList(mlist* list_pointers){
	list_pointers -> head = NULL;
	list_pointers -> tail = NULL;
}

//push a Move to the list
void push(mlist* list_pointers, Move* data){
	node* n =(node*) malloc(sizeof(node));
	n->data = data;
	n->next = list_pointers -> head;

	if(list_pointers -> head == NULL)
		list_pointers -> tail = n;
	list_pointers -> head = n;
}

//get data of the first Moce
Move* peek(mlist* list_pointers){
	if(list_pointers == NULL || list_pointers->head == NULL)
		return NULL;
	return list_pointers -> head -> data;
}

//Pop a move
Move* pop(mlist* list_pointers){
	if(list_pointers == NULL || list_pointers -> head == NULL)
		return NULL;
	Move* move_data;
	
	node* n = list_pointers -> head;
	list_pointers -> head = list_pointers -> head -> next;
	//empty list
	if(list_pointers -> head == NULL)
		list_pointers->tail = NULL;
	move_data = n->data;

	free(n);
	
	return move_data;
}


void freeList(mlist* list_pointers){
	while(list_pointers -> head != NULL)
		free(pop(list_pointers));
	free(list_pointers);
}
void emptyList(mlist* list_pointers){
	while(list_pointers -> head != NULL)
		free(pop(list_pointers));
}