#include "transpositionTable.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


void init_hash_table(){
	
	init_zobrist();
	pos_transp_table = (PosTransp*)malloc(sizeof(PosTransp)*(TABLE_SIZE));

	if(pos_transp_table == NULL){
		printf("Please reduce TABLE_SIZE in transpositionTable.h\n");
		exit(0);
	}
	printf("Reserved : %ld bytes\n",(sizeof(PosTransp)*(TABLE_SIZE)));
	printf("The agent will free this space when you close him!\n");

	//all empty
	for(int i=0; i < TABLE_SIZE; i++)
		pos_transp_table[i].type =0;

}

void init_zobrist(){
	//generate random bit streams
	for (int i=0; i < BOARD_COLUMNS*BOARD_ROWS; i++){
		zobrist_table[i][0] = ((((long) rand() <<  0) & 0x00000000FFFFFFFFull) | (((long) rand() << 32) & 0xFFFFFFFF00000000ull));
		zobrist_table[i][1] = ((((long) rand() <<  0) & 0x00000000FFFFFFFFull) | (((long) rand() << 32) & 0xFFFFFFFF00000000ull));
	}
}



unsigned long zobrist_hash(Position* pos){
	int i,j;
	unsigned long h = 0;
	for (i= 0; i < BOARD_ROWS; i++){
		for (j=0; j < BOARD_COLUMNS; j++){
			if (pos->board[i][j] == WHITE ||pos->board[i][j] == BLACK )
				h = h ^ zobrist_table[i*BOARD_COLUMNS + j][(unsigned)pos->board[i][j]];
		}

	}

	return h;
}


unsigned int hahsCode(long zobrist){
	return  zobrist % TABLE_SIZE;
}

void saveExact(Position* aPos, int upperBound, int lowerBound, char depth){
	unsigned long key = zobrist_hash(aPos);
	unsigned int hash = hahsCode(key);
	int i=0;

	//if it is not empty
	while ((pos_transp_table[hash].type & 0x1)&&(pos_transp_table[hash].zobrist_key != key)&&(i<OPEN_ADDRESSING)){
			i++;
			hash++;

			//wrap around the table
			hash = hash % TABLE_SIZE;
	}

	if(((pos_transp_table[hash].type = 0x7) && (pos_transp_table[hash].upperDepth + pos_transp_table[hash].lowerDepth>=2*depth ))&&(rand() < RAND_MAX/2))
		return;

	pos_transp_table[hash].zobrist_key = key;
	pos_transp_table[hash].upperBound = upperBound;
	pos_transp_table[hash].lowerBound = lowerBound;
	pos_transp_table[hash].upperDepth = depth;
	pos_transp_table[hash].lowerDepth = depth;

	pos_transp_table[hash].type = 0x7;

}


void saveUpper(Position* aPos, int upperBound,  char depth){
	unsigned long key = zobrist_hash(aPos);
	unsigned int hash = hahsCode(key);
	int i=0;
	

	//if it is not empty
	while ((pos_transp_table[hash].type & 0x1)&&(pos_transp_table[hash].zobrist_key != key)&&(i<OPEN_ADDRESSING)){
		i++;
		hash++;

		//wrap around the table
		hash = hash % TABLE_SIZE;
	}

	if((pos_transp_table[hash].type & 0x1)&&(pos_transp_table[hash].zobrist_key == key)) //already the correct position, then just update
	{
		
		if(((pos_transp_table[hash].type & 0x4) && (pos_transp_table[hash].upperDepth >= depth))&&(rand() > RAND_MAX/2))
			return;

	}
	
	pos_transp_table[hash].zobrist_key = key;
	pos_transp_table[hash].upperBound = upperBound;
	pos_transp_table[hash].upperDepth = depth;
	//pos_transp_table[hash].lowerDepth = INT_MIN;
	pos_transp_table[hash].type = 0x5;

}
void saveLower(Position* aPos, int lowerBound, char depth){
	unsigned long key = zobrist_hash(aPos);
	unsigned int hash = hahsCode(key);
	
	int i = 0;

	while ((pos_transp_table[hash].type & 0x1)&&(pos_transp_table[hash].zobrist_key != key)&&(i<OPEN_ADDRESSING)){
		i++; 
		hash++;
		//wrap around the table
		hash = hash % TABLE_SIZE;
	}
	
	if((pos_transp_table[hash].type & 0x1)&&(pos_transp_table[hash].zobrist_key == key)) //already the correct position, then just update
	{
		if((pos_transp_table[hash].type & 0x2) && (pos_transp_table[hash].lowerDepth >= depth)&&(rand() > RAND_MAX/2))
			return;

	}

	
	pos_transp_table[hash].zobrist_key = key;
	pos_transp_table[hash].lowerBound = lowerBound;
	pos_transp_table[hash].lowerDepth = depth;
	//pos_transp_table[hash].upperDepth = INT_MIN;
	pos_transp_table[hash].type = 0x3;

}
PosTransp* retrieve(Position* aPosition){

	unsigned long zobrist = zobrist_hash(aPosition);
	unsigned int hash = hahsCode(zobrist);

	int i =0;
	while((pos_transp_table[hash].type & 0x1)&&(i < OPEN_ADDRESSING)){

		if((pos_transp_table[hash].zobrist_key == zobrist)){
			return &pos_transp_table[hash];
		}
		i++;
		hash++;

		//wrap around the table
		hash = hash % TABLE_SIZE;

	}
	return NULL;


}

void freeTable(){
	free(pos_transp_table);
}