#include "board.h"
#include "move.h"

#define TABLE_SIZE 87654321
#define OPEN_ADDRESSING 1000


/*
* Zobrist hashing starts by randomly generating bitstrings for each possible element of a board game, i.e. 
* for each combination of a piece and a position (in the game of TUCAnts1.01, that's 2 pieces × BOARD_COLUMNS*BOARD_ROWS board positions
*/
void init_zobrist();

/*
* array for each combination of a piece and a position 
*/

long zobrist_table[BOARD_COLUMNS*BOARD_ROWS][2];


/*
* The final Zobrist hash is computed by combining the random bitstrings using bitwise XOR with the Position
*
*
* If the bitstrings are long enough, different board positions will almost certainly hash to different values
*/
unsigned long zobrist_hash(Position *);

/*
* type = 0 => PosTransp is empty
* type = 0x3 (011) => PosTransp is Lower
* type = 0x5 (101) => PosTransp is Upper
* type = 0x7 (111) => PosTransp is Exact
*
* type & 0x2 => not upper
* type & 0x4 => not lower
* type & 0x1 => not empty
*
*
* we don't use enum to save space and time!
* bitwise checks are way faster! (at first we had enum but we exemplified from zobrist!)
*/

typedef struct {
	unsigned long zobrist_key;
	int upperBound;
	int lowerBound;
	char upperDepth;
	char lowerDepth;
	char type; //determines if it's exact,upper,lower
}PosTransp;

PosTransp* pos_transp_table;


void saveExact(Position*, int, int, char);
void saveLower(Position*, int, char);
void saveUpper(Position*, int, char);

PosTransp* retrieve(Position*);

unsigned int hashCode(long);
void init_hash_table();
void freeTable();



