/** Implemented technics:
*
* minimax
* a-b pruning
* quiescence search
* Transposition table
* MTD(F)
* Iterative Deepening
*/

//player that does random moves
void random_move();

/****white player wants to maximaze and black player wants to minimaze
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
