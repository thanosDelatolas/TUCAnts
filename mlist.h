#include "move.h"

/*
* This is a list with moves that is used in the agent (client.c)
*/

typedef struct node{
	Move* data;
	struct node *next;
} node;


typedef struct {
	node *head;
	node *tail;
} mlist;


void initList(mlist* list_pointers);
void push(mlist* list_pointers, Move* data);
Move* peek(mlist* list_pointers);
Move* pop(mlist* list_pointers);

void freeList(mlist*);

void emptyList(mlist*);