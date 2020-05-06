//player that does random moves!
void random_move();

//player based on AI theory
void ai_move();

/* used from evaluate function...*/
typedef struct{
	int food;
	int queens;
	int soldiers;
	int unprotected_soldiers;

	//soldiers that are threatened...
	int in_danger_soldiers;

	//how many soldiers this playes threatens
	int threatening_soldiers;
} PlayerInfo;
/**
* Weigths on each parameter of PlayerInfo 
*/
//evaluates a state based on PlayerInfo
int evaluate_function();
