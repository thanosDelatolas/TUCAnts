#include "board.h"
#include "move.h"
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>

#define TABLE_SIZE 87000000
#define OPEN_ADDRESSING 1000

//returns a random 64 bit number
uint64_t get64rand(); 

/*
* Zobrist hashing starts by randomly generating bitstrings for each possible element of a board game, i.e. 
* for each combination of a piece and a position (in the game of TUCAnts1.01, that's 2 pieces × BOARD_COLUMNS*BOARD_ROWS board positions
*/
void init_zobrist();

enum transp_flag {EXACT,UPPER_BOUND,LOWER_BOUND, NONE};

typedef struct{
	//TUCAnts positi,on in zobritst
	uint64_t zobrist;

	int upperBound;
	int lowerBound;
	int depth;

	enum transp_flag flag; 
	
}HashEntry;

/*
* array for each combination of a piece and a position 
*/
uint64_t zobrist_table[BOARD_COLUMNS*BOARD_ROWS][2]; //white and black 


/*
* The final Zobrist hash is computed by combining the random bitstrings using bitwise XOR with the Position
*
*
* If the bitstrings are long enough, different board positions will almost certainly hash to different values
*/
uint64_t zobrist_hash(Position *);


HashEntry* hashTable;
int overwrites;

void init_hash_table();
void freeTable();


void saveExact(Position* pos, int upperBound, int lowerBound, int depth);
void saveUpper(Position* pos, int upperBound, int depth);
void saveLower(Position* pos, int lowerBound, int depth); 
HashEntry* retrieve(Position* pos);

unsigned int hashCode(uint64_t key);
