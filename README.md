# TCP-Socket-Programming
Develop a pair of programs ("player" and "ringmaster") interacting with each other to model a game. TCP sockets are used as the mechanism for all the communication. 
  
The game is called hot potato, in which there are some number of players who quickly toss a potato from one player to another until, at a random point, the game ends and the player holding the potato is “it”. 
In this game, players form a ring to pass the potato around, and each player has a left heighbor and a right neighbor. 
Also, there will be a ringmaster process that will start each game, report the results, and shut down the game.
  
Each player will establish three network socket connections for communication with the player to the left, the player to the right, the ringmaster. The potato can arrive on any of these three channels. Commands and important information may also be received from the ringmaster. The ringmaster will have N network socket connections. At the end of the game, the ringmaster will receive the potato from the player who is “it”.
