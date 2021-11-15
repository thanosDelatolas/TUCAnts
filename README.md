# TUCAnts  
An agent to play TUCAnts (a game similar to checkers). 
## Requirements:
	Any Linux distribution
## Compilation: 

* `make`   - to build client and server
* `make client` - to build just the client
* `make server` - to build just the server
* `make guiServer` - to build just the guiServer
## Execution: 
* `./guiServer`
* `./server [-p port] [-g number_of_games] [-s (swap color after   each game)]`
* `./client [-i ip] [-p port]`
## Implementation 
 * Alpha–beta pruning
 * Quiescence search
 * Transposition table
 * MTD(f)
