#include "board.h"
#include "move.h"

long zobrist_table[BOARD_COLUMNS*BOARD_ROWS][2]; //white and black ant