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
	hashTable = (HashEntry*)malloc(sizeof(HashEntry)*TABLE_SIZE);
	if(hashTable == NULL){
		printf("Please reduce TABLE_SIZE in transositionTable.h\n");
		exit(0);
	}

	for (int i = 0; i < TABLE_SIZE; ++i){
		hashTable[i].flag = NONE;
	}
	overwrites = 0;
}

void freeTable(){
	free(hashTable);
}

unsigned int hashCode(uint64_t key){
	return key % TABLE_SIZE;
}

void saveExact(Position* pos, int upperBound, int lowerBound, int depth){
	uint64_t key = zobrist_hash(pos);

	unsigned int hash = hashCode(key);

	

	int i=0;

	while(hashTable[hash].flag != NONE && i < OPEN_ADDRESSING){
		i++;
		hash++;

		//wrap arround table
		hash = hash % TABLE_SIZE;
	}

	//check if we have to overwrite...
	if(hashTable[hash].flag != NONE){
		if(abs(hashTable[hash].depth) - abs(depth) >= 2)
			return;
		//overwrite
		if(abs(depth) - abs(hashTable[hash].depth)  >= 2){
			overwrites++;
		}

	}

	hashTable[hash].zobrist = key;
	hashTable[hash].upperBound = upperBound;
	hashTable[hash].lowerBound = lowerBound;
	hashTable[hash].depth = depth;
	hashTable[hash].flag = EXACT;
	
	
}

void saveUpper(Position* pos, int upperBound, int depth){
	uint64_t key = zobrist_hash(pos);

	unsigned int hash = hashCode(key);

	

	int i=0;

	while(hashTable[hash].flag != NONE && i < OPEN_ADDRESSING){
		i++;
		hash++;

		//wrap arround table
		hash = hash % TABLE_SIZE;
	}

	//check if we have to overwrite...
	if(hashTable[hash].flag != NONE){
		if(abs(hashTable[hash].depth) - abs(depth) >= 2)
			return;
		//overwrite
		if(abs(depth) - abs(hashTable[hash].depth)  >= 2){
			overwrites++;
		}

	}

	hashTable[hash].zobrist = key;
	hashTable[hash].upperBound = upperBound;
	hashTable[hash].lowerBound = -1;
	hashTable[hash].depth = depth;
	hashTable[hash].flag = UPPER_BOUND;
	
	
}


void saveLower(Position* pos, int lowerBound, int depth){
	uint64_t key = zobrist_hash(pos);

	unsigned int hash = hashCode(key);

	

	int i=0;

	while(hashTable[hash].flag != NONE && i < OPEN_ADDRESSING){
		i++;
		hash++;

		//wrap arround table
		hash = hash % TABLE_SIZE;
	}

	//check if we have to overwrite...
	if(hashTable[hash].flag != NONE){
		if(abs(hashTable[hash].depth) - abs(depth) >= 2)
			return;
		//overwrite
		if(abs(depth) - abs(hashTable[hash].depth)  >= 2){
			overwrites++;
		}

	}

	hashTable[hash].zobrist = key;
	hashTable[hash].upperBound = -1;
	hashTable[hash].lowerBound = lowerBound;
	hashTable[hash].depth = depth;
	hashTable[hash].flag = LOWER_BOUND;
	
	
}


HashEntry* retrieve(Position* pos){

	uint64_t zobrist_key = zobrist_hash(pos);
	unsigned int hash = hashCode(zobrist_key);

	int i=0;

	while(hashTable[hash].flag != NONE && i < OPEN_ADDRESSING){

		//we have saved this position
		if(hashTable[hash].zobrist == zobrist_key)
			return &hashTable[hash];
		i++;
		hash++;

		//wrap arround table
		hash = hash % TABLE_SIZE;
	}

	return NULL;



}