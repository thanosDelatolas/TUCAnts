#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
#include "client.h"
#include "transpositionTable.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>

#define MAX_DEPTH 12
#define INFINITY 999999999

#define MAX_TIME 7

/**********************************************************/
Position gamePosition;		// Position we are going to use

Move moveReceived;			// temporary move to retrieve opponent's choice
Move myMove;				// move to save our choice and send it to the server

unsigned char myColor;				// to store our color
int mySocket;				// our socket

char * agentName = "DeVou!";		//default name.. change it! keep in mind MAX_NAME_LENGTH

char * ip = "127.0.0.1";	// default ip (local machine)
/**********************************************************/

int max(int a, int b)
{
	if (a > b)
		return a;
	return b;
}
int min(int a, int b)
{
	if(a < b)
		return a;
	return b;
}

int main( int argc, char ** argv )
{
	int c;
	opterr = 0;

	while( ( c = getopt ( argc, argv, "i:p:h" ) ) != -1 )
		switch( c )
		{
			case 'h':
				printf( "[-i ip] [-p port]\n" );
				return 0;
			case 'i':
				ip = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case '?':
				if( optopt == 'i' || optopt == 'p' )
					printf( "Option -%c requires an argument.\n", ( char ) optopt );
				else if( isprint( optopt ) )
					printf( "Unknown option -%c\n", ( char ) optopt );
				else
					printf( "Unknown option character -%c\n", ( char ) optopt );
				return 1;
			default:
			return 1;
		}



	connectToTarget( port, ip, &mySocket );
	init_zobrist();
	init_hash_table();

	char msg;

/**********************************************************/
// used in random
	srand( time( NULL ) );
	
/**********************************************************/

	while( 1 )
	{

		msg = recvMsg( mySocket );

		switch ( msg )
		{
			case NM_REQUEST_NAME:		//server asks for our name
				sendName( agentName, mySocket );
				break;

			case NM_NEW_POSITION:		//server is trying to send us a new position
				getPosition( &gamePosition, mySocket );
				printPosition( &gamePosition );
				break;

			case NM_COLOR_W:			//server indorms us that we have WHITE color
				myColor = WHITE;
				printf("My color is %d\n",myColor);
				break;

			case NM_COLOR_B:			//server indorms us that we have BLACK color
				myColor = BLACK;
				printf("My color is %d\n",myColor);
				break;

			case NM_PREPARE_TO_RECEIVE_MOVE:	//server informs us that he will send opponent's move
				getMove( &moveReceived, mySocket );
				moveReceived.color = getOtherSide( myColor );
				doMove( &gamePosition, &moveReceived );		//play opponent's move on our position
				printPosition( &gamePosition );
				break;

			case NM_REQUEST_MOVE:		//server requests our move
				myMove.color = myColor;


				if( !canMove( &gamePosition, myColor ) )
				{
					myMove.tile[ 0 ][ 0 ] = -1;		//null move
				}
				else
				{
					Move *tempMove = make_move(&gamePosition);
					myMove = *tempMove;
					free(tempMove);
				}

				sendMove( &myMove, mySocket );			//send our move
				doMove( &gamePosition, &myMove );		//play our move on our position
				printPosition( &gamePosition );

				
				break;

			case NM_QUIT:			//server wants us to quit...we shall obey
				close( mySocket );
				return 0;
		}

	} 
	freeTable();
	return 0;
}

Move* make_move(Position* pos){

	
	Move* agent_move = malloc(sizeof(Move));

	Position* tempPosition = malloc(sizeof(Position));
	memcpy(tempPosition, pos, sizeof(Position));

	alpha_beta(tempPosition, 11, -INFINITY, INFINITY, 1, agent_move);
	//MTDF(tempPosition, evaluate_function(pos), 10, agent_move);
	//iterativeDeepening(tempPosition, agent_move);
	
	free(tempPosition);
	
	return agent_move;
}

/****white player wants to minimaze and black player wants to maximaze
*
* gain 100 points for each ant that you own
* gain points for the depth (in board ) of each ant 
*
* lose 100 points for each ant that the opponent own
* lose points for the depth (in board ) of each opponent's ant
*
* pos -> score*90 because score has queens and food
*/

int evaluate_function (Position *pos) {
    int i,j, heuristic = 0;
    
    for (i = 0; i < BOARD_ROWS; i++){
        for ( j = 0; j < BOARD_COLUMNS; j++){
            if (pos->board[i][j] == myColor){
       			heuristic += 100;
           		if(myColor == WHITE)
           			heuristic += i;
           		else
           			heuristic += (BOARD_ROWS-i-1);


           	}
            else if (pos->board[i][j] == getOtherSide(myColor)){
           		heuristic -= 100;
           		if(myColor == BLACK)
           			heuristic -= i;
           		else
           			heuristic -= (BOARD_ROWS-i-1);
           	}

        }
    }

    //scores of each player *90 beacause score is important!
    heuristic = heuristic + pos->score[myColor]*90 - pos->score[getOtherSide(myColor)]*90;
    //printf("Evaluation %d\n",heuristic );

    return heuristic;
}

/*
*It is an extension of the evaluation function to defer evaluation until the position is stable enough to be evaluated
* enough to be evaluated means no jump available
*
* evaluate_function works better because none of the ants is threatened!
*/
int quiescence_search(Position* pos){
		int i, j;
		for( i = 0; i < BOARD_ROWS; i++ ){
			for( j = 0; j < BOARD_COLUMNS; j++ ){
				if( pos->board[ i ][ j ] == pos->turn ){
					if( canJump( i, j, pos->turn, pos ) ){
						return TRUE;
					}
				}
			}
		}
		return FALSE;
}
/*
* alpha_beta with memory!
*  A transposition table stores and retrieves the previously searched portions of the tree in memory
*  to reduce the overhead of re-exploring parts of the search tree
*/
int alpha_beta(Position *pos, char depth, int alpha, int beta, char maximizingPlayer, Move* finalMove){  

	PosTransp* pos_transp = retrieve(pos);
	if( pos_transp != NULL && finalMove == NULL){ 
		//a saved position exists!!

		//if not upper		
		if((pos_transp -> type & 0x2) && (pos_transp->lowerBound >= beta)){
			if(pos_transp -> lowerDepth >= depth){
				hitsLower++;
				return pos_transp -> lowerBound;
			}
		}

		//if not upper but lowerBound < beta
		if((pos_transp -> type & 0x2))
			if(pos_transp -> lowerDepth >= depth){
				alpha = max(alpha, pos_transp -> lowerBound);
			}

		//if not lower			
		if((pos_transp -> type & 0x4) && (pos_transp -> upperBound <= alpha)){
			if(pos_transp -> upperDepth >= depth){
				hitsUpper++;
				return pos_transp -> upperBound;
			}
		}

		//if not lower but upperBound > alpha
		if((pos_transp -> type & 0x4)){
			if(pos_transp -> upperDepth >= depth){
				
				beta = min (beta, pos_transp -> upperBound);
			}
		}
			
	}
	

	if (depth <= 0){ //if we reached the maximum depth of our recursion
		if(!quiescence_search(pos)){ // and there are no captures
			return evaluate_function(pos); //return heuristic
		}
		
	}

	//descover all legal moves in this position
	list *children = find_moves(pos);   
	Move * child = NULL;


	if (top(children) == NULL){    
		freeList(children);
		return evaluate_function(pos);
	}

	Position* tempPosition = malloc(sizeof(Position));

	int tempScore, g;

	int a, b;


	if (maximizingPlayer){

		g = -INFINITY;
		a = alpha;
		while( g<beta && ((child = pop(children)) != NULL)){


			memcpy(tempPosition, pos, sizeof(Position));
			doMove(tempPosition, child);

			tempScore = alpha_beta(tempPosition, depth-1, a, beta, 0, NULL);

			if(g < tempScore){
				g = tempScore;
				if(finalMove != NULL){
					
					memcpy(finalMove, child, sizeof(Move));
				}
			}
			a = max(a, g);
			
			free(child);
		}
	}
	else{
		g = INFINITY;
		b = beta;
		while( g>alpha && (child = pop(children)) != NULL){ //for each child position

			memcpy(tempPosition, pos, sizeof(Position));
			doMove(tempPosition, child);
			
			tempScore = alpha_beta(tempPosition, depth-1, alpha, b, 1, NULL);
			
			g = min(g, tempScore);
			b = min(b, g);

			
			free(child);
		}
	}

	freeList(children);
	free(tempPosition);
	
	if(g <= alpha){
		saveUpper(pos, g, depth);
	}
	if(g > alpha && g < beta){
		saveExact(pos, g, g, depth);
	}
	if (g>= beta)
		saveLower(pos, g, depth);
	

	return g;
}


int iterativeDeepening(Position* pos, Move* agent_move){
	//first guess
	int f = evaluate_function(pos);


	char d=8;
	clock_t start_clock = clock();
	while(1){
		f = MTDF(pos, f, d, agent_move);
		//give max 7 seconds to find best move
		if(((clock() - start_clock)/CLOCKS_PER_SEC > MAX_TIME) || (d > MAX_DEPTH)){
			printf("Max Score: %d\n", f);
			printf("Time used: %ld\n", (clock() - start_clock)/CLOCKS_PER_SEC);
			printf("Depth of iteration: %d\n", d);
			break;
		}
		d++;

	}
	printf("Time used: %ld\n", (clock() - start_clock)/CLOCKS_PER_SEC);
	printf("Best Move: %d\n", f);

	return f;
}

//this coded is based on wikipedia's pseudocode (https://en.wikipedia.org/wiki/MTD-f)
int MTDF(Position* pos, int f, char d, Move* agent_move){
	int upperBound = INFINITY;
	int lowerBound = -INFINITY;

	int b;
	int g = f;

	Move* move = (Move*)malloc(sizeof(Move));

	while(lowerBound < upperBound){
		b = max(g, lowerBound+1);

		g = alpha_beta(pos, d, b-1, b, TRUE, move);

		if(g < b)
			upperBound = g;
		else{
			lowerBound = g;
			memcpy(agent_move, move, sizeof(Move));
		}

	}

	free(move);
	return g;
}



//methods to find valid moves! Jumps included!
void follow_jump(list* moveList, Move* move, int rec_depth /* depth of recursion*/,char i, char j, Position *pos){
	
	
	int possibleJumps, playerDirection;
	char color = move->color;
	move->tile[0][rec_depth] = i;
	move->tile[1][rec_depth] = j;

	//adding valid jump moves in the list!
	if(!(possibleJumps = canJump(i, j, color, pos))){
		move->tile[0][rec_depth+1] = -1;
		if(isLegal(pos, move)){
			push(moveList, move);
		}
		else
			free(move);
		return;
	}

	if( color == WHITE )		
		playerDirection = 1;
	else
		playerDirection = -1;

	//left jump
	if(possibleJumps == 1){ 
		follow_jump(moveList, move, rec_depth+1, i + 2*playerDirection, j-2, pos);
	}

	//right jump
	if(possibleJumps == 2){ 
		follow_jump(moveList, move, rec_depth+1, i + 2*playerDirection, j+2, pos);
	}

	//both jumps
	if(possibleJumps == 3){ 
		Move * newMove = malloc(sizeof(Move));
		memcpy(newMove, move, sizeof(Move));
		

		follow_jump(moveList, move, rec_depth+1, i + 2*playerDirection, j-2, pos);
		follow_jump(moveList, newMove, rec_depth+1, i + 2*playerDirection, j+2, pos);

	}
	
}

list* find_moves(Position *pos) {
	int i, j, jumpPossible = FALSE, movePossible = FALSE, playerDirection;
	list* moveList = malloc(sizeof(list));

	initList(moveList);
	Move *move;

	char curColor = pos->turn;

	if( curColor == WHITE )	
		playerDirection = 1;
	else
		playerDirection = -1;

	for( i = 0; i < BOARD_ROWS; i++ ){
		for( j = 0; j < BOARD_COLUMNS; j++){
			if( pos->board[ i ][ j ] == curColor ){
				if( canJump( i, j, curColor, pos ) ){
					if(!jumpPossible)
						emptyList(moveList); //any simple moves are deleted => only jump moves staying

					move = malloc(sizeof(Move));
					move->color = curColor;
					follow_jump(moveList, move, 0, i, j, pos); 
					jumpPossible = TRUE;
				}

				if((jumpPossible == FALSE) && (movePossible = dirMoveFrom ( i, j, curColor, pos))){

					//left move
					if(movePossible % 2 == 1){ 
						move = malloc(sizeof(Move));
						move->color = pos->turn;
						move->tile[0][0] = i;
						move->tile[1][0] = j;
						move->tile[0][1] = i + playerDirection;
						move->tile[1][1] = j-1;
						move->tile[0][2] = -1;
						if(isLegal(pos, move)){
							push(moveList, move);}
						else
							free(move);
					}

					if(movePossible > 1){
						move = malloc(sizeof(Move));
						move->color = pos->turn;
						move->tile[0][0] = i;
						move->tile[1][0] = j;
						move->tile[0][1] = i + playerDirection;
						move->tile[1][1] = j+1;
						move->tile[0][2] = -1;
						if(isLegal(pos, move)){
							push(moveList, move);}
						else
							free(move);
					}

				}

			}
		}
	}

	//if we can't move add empty move!
	if(top(moveList)==NULL){ 
		move = malloc(sizeof(Move));
		move->color = curColor;
		move->tile[0][0] = -1;
		push(moveList, move);
		return moveList;
	}
	return moveList;
						
}


int dirMoveFrom( char row, char col, char player, Position * pos ) {
	int returnValue = 0;
	assert( ( player ==  WHITE ) || ( player == BLACK ) );
	if( player == WHITE )	//white player
	{	
		if( row + 1 < BOARD_ROWS)
		{
			if( col - 1 >= 0 )
			{
				if( (pos->board[ row + 1][ col - 1] == EMPTY)  || ( pos->board[ row + 1][ col - 1] == RTILE))
				{
					returnValue = returnValue + 1;	//left move possible
				}
			}

			if( col + 1 < BOARD_COLUMNS )
			{
				if( (pos->board[ row + 1][ col + 1] == EMPTY)  || ( pos->board[ row + 1][ col + 1] == RTILE))
				{
					returnValue = returnValue + 2;	//right move possible
				}
			}

		}
	}
	else	//black player
	{
		if( row - 1 >= 0 )
		{
			if( col - 1 >= 0 )
			{
				if( (pos->board[ row - 1][ col - 1] == EMPTY) ||(pos->board[ row - 1][ col - 1] == RTILE))
				{
					returnValue = returnValue + 1;	//left move possible
				}
			}

			if( col + 1 < BOARD_COLUMNS )
			{
				if( (pos->board[ row - 1][ col + 1] == EMPTY) || (pos->board[ row - 1][ col + 1] == RTILE)) 
				{
					returnValue = returnValue + 2;	//right move possible
				}
			}

		}
	}

	return returnValue;

}