#include "list.h"
/** Implemented technics:
*
* minimax
* a-b pruning
* quiescence search
* Transposition table
* MTD(F)
* Iterative Deepening
*/

/****white player wants to minimaze and black player wants to maximazi
*
* gain 100 points for each ant that you own
* gain points for the depth (in board ) of each ant 
*
* lose 100 points for each ant that the opponent own
* lose points for the depth (in board ) of each opponent's ant
*
* pos -> score*90 because score has queens and food
*/
int evaluate_function(Position* pos);

/*
*It is an extension of the evaluation function to defer evaluation until the position is stable enough to be evaluated
* enough to be evaluated means no jump available
*
* evaluate_function works better because none of the ants is threatened!
*/
int quiescence_search(Position* pos);

/*** Main function of the agent
*
*/
Move* make_move(Position* pos, int depth);

int iterativeDeepening(Position* pos, Move* agent_move);

//returns a list with all possible moves from a position
list* find_moves(Position *aPosition);

void follow_jump(list* moveList, Move* move, int k /* depth of recursion*/,char i, char j, Position *aPosition);

//similar to canJump(board.h) but for moves
int dirMoveFrom ( char row, char col, char player, Position *pos);

int alpha_beta(Position *aPosition, char depth, int alpha, int beta, char maximizingPlayer, Move* finalMove);