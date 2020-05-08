#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "mlist.h"

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

char * agentName = "DEVOU_AGENT!";

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

					random_move();

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

	iterativeDeepening(temp_pos, agent_move);

	free(temp_pos);
	
	return agent_move;
}

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
    //scores of each player *90 beacause score is important!
    return heuristic + pos -> score[myColor]*90 - pos -> score[getOtherSide(myColor)]*90;

}
/*
*It is an extension of the evaluation function to defer evaluation until the position is stable enough to be evaluated
* enough to be evaluated means no jump available
*/
int quiescence_search(Position* pos){
		int i, j;
		for( i = 0; i < BOARD_ROWS; i++ ){
			for( j = 0; j < BOARD_COLUMNS; j++ ){
				if( pos->board[ i ][ j ] == pos->turn ){
					if( canJump( i, j, pos->turn, pos )){
						return TRUE;
					}
				}
			}
		}
		return FALSE;


}

int iterativeDeepening(Position* pos, Move* agent_move){
	return -1;
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






