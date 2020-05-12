#include "move.h"

typedef struct node{
	Move* data;
	struct node *next;
} node;


typedef struct {
	node *start;
	node *tail;
} list;


void initList(list *sList);
void push(list *sList, Move* data);
Move* pop(list *sList);
void freeList(list*);
list* merge(list*, list*);
Move* top(list *sList);
void printList(list*);
void printMove(Move *aMove);
void emptyList(list*);