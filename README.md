# TUCAnts  

Requirements:

	* any Linux distribution

Compilation: 

* make [all]  - to build client and server
* make client - to build just the client
* make server - to build just the server
* make guiServer - to build just the guiServer

Execution: 

./guiServer
./server [-p port] [-g number_of_games] [-s (swap color after 
						   each game)]
./client [-i ip] [-p port]

Changes in last version:

	-Client sends the desired move to server and server 
	 executes doMove. Server sends new position to clients
	 after every move made.

