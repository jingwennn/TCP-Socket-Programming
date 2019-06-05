#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

typedef struct playernode{
  int num_play;
  int index;
  int left_index;
  int right_index;
  char left_port[256];
  char right_port[256];
  char left_player[256];
  char right_player[256];
}playernode;

typedef struct potato{
  int num_hop;
  int current_hop;
  int path[512];
  int end_game;
}potato_info;
