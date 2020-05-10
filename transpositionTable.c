#include "transpositionTable.h"
#include <stdio.h>
#include <stdlib.h>

uint64_t get64rand() {
    return
    (((uint64_t) rand() <<  0) & 0x000000000000FFFFull) |
    (((uint64_t) rand() << 16) & 0x00000000FFFF0000ull) |
    (((uint64_t) rand() << 32) & 0x0000FFFF00000000ull) |
    (((uint64_t) rand() << 48) & 0xFFFF000000000000ull);
}

/*
* Zobrist hashing starts by randomly generating bitstrings for each possible element of a board game, i.e. 
* for each combination of a piece and a position (in the game of TUCAnts1.01, that's 2 pieces × BOARD_COLUMNS*BOARD_ROWS board positions
*/
void zobrist_init(){
	// used in random
	srand( time( NULL ) );
	int i;
	for (i=0; i < BOARD_COLUMNS*BOARD_ROWS; i++){
		zobrist_table[i][0] = get64rand();
		zobrist_table[i][1] = get64rand();
	}
	
}

/*
* The final Zobrist hash is computed by combining the random bitstrings using bitwise XOR with the Position
*
*
* If the bitstrings are long enough, different board positions will almost certainly hash to different values
*/

uint64_t zobrist_hash(Position* pos){
	int i,j;
	uint64_t h = 0;
	for (i= 0; i < BOARD_ROWS; i++){
		for (j=0; j < BOARD_COLUMNS; j++){
			if (pos->board[i][j] == WHITE ||pos->board[i][j] == BLACK )
				h = h ^ zobrist_table[i*BOARD_COLUMNS + j][(unsigned)pos->board[i][j]];
		}

	}

	return h;
}



void init_hash_table(){
	hashTable = (DataItem*)malloc(sizeof(DataItem)*TABLE_SIZE);
	if(hashTable == NULL){
		printf("Please reduce TABLE_SIZE in transositionTable.h\n");
		exit(0);
	}
}

void freeTable(){
	free(hashTable);
}