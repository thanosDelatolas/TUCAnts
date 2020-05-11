#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "list.h"
#include <string.h>
#include <limits.h>
#include "transpositionTable.h"



#define MAX_DEPTH 20


int max(int a, int b){
	if (a > b)
		return a;
	return b;
}
int min(int a, int b){
	if(a < b)
		return a;
	return b;
}

/**********************************************************/
Position gamePosition;		// Position we are going to use

Move moveReceived;			// temporary move to retrieve opponent's choice
Move myMove;				// move to save our choice and send it to the server

char myColor;				// to store our color
int mySocket;				// our socket

char * agentName = "DeVou!";

char * ip = "127.0.0.1";	// default ip (local machine)


int main( int argc, char ** argv ){
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

	char msg;

	//init zobrist table
	init_zobrist();
	//init transposition table
	init_hash_table();
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
				else{

					Move *tempMove = make_move(&gamePosition, 6);
					myMove = *tempMove;
					free(tempMove);
				}

				sendMove( &myMove, mySocket );			//send our move
				//printf("i chose to go from (%d,%d), to (%d,%d)\n",myMove.tile[0][0],myMove.tile[1][0],myMove.tile[0][1],myMove.tile[1][1]);
				doMove( &gamePosition, &myMove );		//play our move on our position
				printPosition( &gamePosition );
				
				break;

			case NM_QUIT:			//server wants us to quit...we shall obey
				close( mySocket );
				return 0;
		}

	}
 

	return 0;
}

Move* make_move(Position* pos, int depth){

	
	Move* agent_move = malloc(sizeof(Move));

	Position* temp_pos = malloc(sizeof(Position));
	memcpy(temp_pos, pos, sizeof(Position));
	alpha_beta(temp_pos, depth, TRUE, INT_MIN, INT_MAX, agent_move);
	
	free(temp_pos);
	
	return agent_move;
}

int alpha_beta(Position* pos, int depth, int maximizingPlayer, int alpha, int beta, Move* finalMove){

	//go down until we are in a stable position
	if(depth <=0  && quiescence_search(pos) == TRUE)
		return evaluate_function(pos);

	Move* child;

	Position* tempPosition = malloc(sizeof(Position));
	

	int value;

	list* children = find_moves(pos);

	//if we cant move..
	if(top(children) == NULL)
		return evaluate_function(pos);

	if(maximizingPlayer == TRUE){
		value = INT_MIN;
		printf("%d\n", pos -> turn );
		while(((child = pop(children)) != NULL)){
			memcpy(tempPosition, pos, sizeof(Position));
			doMove(tempPosition, child);
			
			value = max(value, alpha_beta(tempPosition, depth -1 , FALSE, alpha, beta, NULL));
			alpha = max(value, alpha);
			//move	
			if (finalMove != NULL)
				memcpy(finalMove, child, sizeof(Move));

			if(alpha >= beta)
				break;
		}

	}

	else{
		value = INT_MAX;
		printf("%d\n", pos -> turn );
		while(((child = pop(children)) != NULL)){
			memcpy(tempPosition, pos, sizeof(Position));
			doMove(tempPosition, child);

			value = min(value, alpha_beta(tempPosition, depth -1 , TRUE, alpha, beta, NULL));
			beta  = min(value, beta);

			if(alpha >= beta)
				break;

		}

	}
	
	free(children);
	free(tempPosition); 
	return value;

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
int evaluate_function(Position* pos){
	int i,j, heuristic = 0;
    
	for (i = 0; i < BOARD_ROWS; i++){
    	for ( j = 0; j < BOARD_COLUMNS; j++){
            if (pos -> board[i][j] == myColor){
            	//for each cell that i own, gain 100 points! 
            	heuristic += 100;
            	//add points for the depth of the cell 
           		if(myColor == WHITE)
           			heuristic += i;
           		else
      				heuristic += (BOARD_ROWS-i-1);

           	}
            else if (pos -> board[i][j] == getOtherSide(myColor)){
            	//for each cell that i own, gain 100 points!
       			heuristic -= 100;
            	//add points for the depth of the cell 
           		if(myColor == BLACK)
           			heuristic -= i;
           		else
           			heuristic -= (BOARD_ROWS-i-1);
           	}

        }
    }
    
    int x = heuristic + pos -> score[myColor]*90 - pos -> score[getOtherSide(myColor)]*90;

    if( pos -> turn == WHITE)
    	return -x;
    //scores of each player *90 beacause score is important!
    return  x;

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
				if( canJump( i, j, pos->turn, pos )){
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

int iterativeDeepening(Position* pos, Move* agent_move){
	return -1;
}

list* find_moves(Position *aPosition){
	int i, j, jumpPossible = FALSE, movePossible = FALSE, playerDirection;
	list* moveList = malloc(sizeof(list));

	initList(moveList);

	Move *move;

	char curColor = aPosition->turn;

	if( curColor == WHITE )		// find movement's direction
		playerDirection = 1;
	else
		playerDirection = -1;

	for( i = 0; i < BOARD_ROWS; i++ ){
		for( j = 0; j < BOARD_COLUMNS; j++){
			if( aPosition->board[ i ][ j ] == curColor ){
				if( canJump( i, j, curColor, aPosition ) ){
					if(!jumpPossible)
						emptyList(moveList); //any simple moves are deleted
					move = malloc(sizeof(Move));
					move->color = curColor;
					follow_jump(moveList, move, 0, i, j, aPosition); 
					jumpPossible = TRUE;
				}
				if((jumpPossible == FALSE) && (movePossible = dirMoveFrom ( i, j, curColor, aPosition))){
					if(movePossible % 2 == 1){ //left move possible	
						move = malloc(sizeof(Move));
						move->color = aPosition->turn;
						move->tile[0][0] = i;
						move->tile[1][0] = j;
						move->tile[0][1] = i + playerDirection;
						move->tile[1][1] = j-1;
						move->tile[0][2] = -1;
						if(isLegal(aPosition, move)){
							push(moveList, move);}
						else
							free(move);
					}
					if(movePossible > 1){
						move = malloc(sizeof(Move));
						move->color = aPosition->turn;
						move->tile[0][0] = i;
						move->tile[1][0] = j;
						move->tile[0][1] = i + playerDirection;
						move->tile[1][1] = j+1;
						move->tile[0][2] = -1;
						if(isLegal(aPosition, move)){
							push(moveList, move);}
						else
							free(move);
					}

				}

			}
		}
	}

	if(top(moveList)==NULL){ //if we can't move
		move = malloc(sizeof(Move));
		move->color = curColor;
		move->tile[0][0] = -1;
		push(moveList, move);
	}

	return moveList;

} 

void follow_jump(list* moveList, Move* move, int k /* depth of recursion*/,char i, char j, Position *aPosition){
	
	int possibleJumps, playerDirection;
	char color = move->color;
	move->tile[0][k] = i;
	move->tile[1][k] = j;

	if(!(possibleJumps = canJump(i, j, color, aPosition))){
		move->tile[0][k+1] = -1;

		if(isLegal(aPosition, move))
			push(moveList, move);
		else
			free(move);
		return;
	}

	if( color == WHITE )		// find movement's direction
		playerDirection = 1;
	else
		playerDirection = -1;


	if(possibleJumps == 1) //can jump left
		follow_jump(moveList, move, k+1, i + 2*playerDirection, j-2, aPosition);


	if(possibleJumps == 2) //can jump right
		follow_jump(moveList, move, k+1, i + 2*playerDirection, j+2, aPosition);

	if(possibleJumps == 3){ //we need to split the jumps
		//copying move:
		Move * newMove = malloc(sizeof(Move));
		memcpy(newMove, move, sizeof(Move));
		//following both left and right
		follow_jump(moveList, move, k+1, i + 2*playerDirection, j-2, aPosition);
		follow_jump(moveList, newMove, k+1, i + 2*playerDirection, j+2, aPosition);

	}
	
}

int dirMoveFrom( char row, char col, char player, Position* pos ) {
	int ret = 0;
	
	if(player == WHITE){	
		if( row + 1 < BOARD_ROWS){
			if( col - 1 >= 0 ){
				if( (pos->board[ row + 1][ col - 1] == EMPTY)  || ( pos->board[ row + 1][ col - 1] == RTILE)){
					ret = ret + 1;	//left move possible
				}
			}

			if( col + 1 < BOARD_COLUMNS ){
				if( (pos->board[ row + 1][ col + 1] == EMPTY)  || ( pos->board[ row + 1][ col + 1] == RTILE)){
					ret = ret + 2;	//right move possible
				}
			}

		}
	}
	else{ //black player
		if( row - 1 >= 0 ){
			if( col - 1 >= 0 ){
				if( (pos->board[ row - 1][ col - 1] == EMPTY) ||(pos->board[ row - 1][ col - 1] == RTILE)){
					ret = ret + 1;	//left move possible
				}
			}

			if( col + 1 < BOARD_COLUMNS ){
				if( (pos->board[ row - 1][ col + 1] == EMPTY) || (pos->board[ row - 1][ col + 1] == RTILE)) {
					ret = ret + 2;	//right move possible
				}
			}

		}
	}

	return ret;

}

/**********************************************************/	
	int i, j, k;
	int jumpPossible;
	int playerDirection;
/**********************************************************/


void random_move(){
	// used in random
	srand( time( NULL ) );
	// random player - not the most efficient implementation
	
	if( myColor == WHITE )		// find movement's direction
		playerDirection = 1;
	else
		playerDirection = -1;

	jumpPossible = FALSE;		// determine if we have a jump available
	for( i = 0; i < BOARD_ROWS; i++ ){
		for( j = 0; j < BOARD_COLUMNS; j++ )
		{
			if( gamePosition.board[ i ][ j ] == myColor )
			{
				if( canJump( i, j, myColor, &gamePosition ) )
					jumpPossible = TRUE;
			}
		}
	}

	while( 1 ){
		i = rand() % (BOARD_ROWS);
		j = rand() % BOARD_COLUMNS;

		if( gamePosition.board[ i ][ j ] == myColor ){		//find a piece of ours

			myMove.tile[ 0 ][ 0 ] = i;		//piece we are going to move
			myMove.tile[ 1 ][ 0 ] = j;

			if( jumpPossible == FALSE )
			{
				myMove.tile[ 0 ][ 1 ] = i + 1 * playerDirection;
				myMove.tile[ 0 ][ 2 ] = -1;
				if( rand() % 2 == 0 )	//with 50% chance try left and then right
				{
					myMove.tile[ 1 ][ 1 ] = j - 1;
					if( isLegal( &gamePosition, &myMove ))
						break;

					myMove.tile[ 1 ][ 1 ] = j + 1;
					if( isLegal( &gamePosition, &myMove ))
						break;
				}
				else	//the other 50%...try right first and then left
				{
					myMove.tile[ 1 ][ 1 ] = j + 1;
					if( isLegal( &gamePosition, &myMove ))
						break;

					myMove.tile[ 1 ][ 1 ] = j - 1;
					if( isLegal( &gamePosition, &myMove ))
						break;
				}
			}
			else	//jump possible
			{
				if( canJump( i, j, myColor, &gamePosition ) ){
					k = 1;
					while( canJump( i, j, myColor, &gamePosition ) != 0 )
					{
						myMove.tile[ 0 ][ k ] = i + 2 * playerDirection;
						if( rand() % 2 == 0 )	//50% chance
						{

							if( canJump( i, j, myColor, &gamePosition ) % 2 == 1 )		//left jump possible
								myMove.tile[ 1 ][ k ] = j - 2;
							else
								myMove.tile[ 1 ][ k ] = j + 2;

						}
						else	//50%
						{

							if( canJump( i, j, myColor, &gamePosition ) > 1 )		//right jump possible
								myMove.tile[ 1 ][ k ] = j + 2;
							else
								myMove.tile[ 1 ][ k ] = j - 2;

						}

						if( k + 1 == MAXIMUM_MOVE_SIZE )	//maximum tiles reached
							break;

						myMove.tile[ 0 ][ k + 1 ] = -1;		//maximum tiles not reached

						i = myMove.tile[ 0 ][ k ];		//we will try to jump from this point in the next loop
						j = myMove.tile[ 1 ][ k ];


						k++;
					}
					break;
				}
			}
		}

	}
}






