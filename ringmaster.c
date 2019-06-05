#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include "potato.h"

int main(int argc, char* argv[])
{
  if(argc!=4){
    return -1;
  }
  char *port=argv[1];
  int num_play=atoi(argv[2]);
  int num_hop=atoi(argv[3]);
  int client_connection_fd_list[num_play];
  char host_list[num_play][256];
  playernode player [num_play];
  if(num_play<=1){
    return -1;
  }

  if(num_hop<0 || num_hop>512){
    return -1;
  }
  printf("Potato Ringmaster\n");
  printf("Players = %d\n",num_play);
  printf("Hops = %d\n",num_hop);
  int status;
  int socket_fd;
  struct addrinfo host_info;
  char *hostname = NULL;
  struct addrinfo *host_info_list;
  fd_set active_fd_set, read_fd_set;
  FD_ZERO (&active_fd_set);

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    return -1;
  } //if

  socket_fd = socket(host_info_list->ai_family,
		     host_info_list->ai_socktype,
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    return -1;
  } //if

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    //printf("Error: cannot bind socket\n");
    return -1;
  } //if

  status = listen(socket_fd, 100);
  if (status == -1) {
    //printf("Error: cannot listen on socket\n");
    return -1;
  } //if

  for (int i=0;i<num_play;i++){
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd=-1;
    while(client_connection_fd==-1)
      client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
      printf("Error: cannot accept connection on socket\n");
      return -1;
    } //if
    printf("Player %d is ready to play\n",i);

    char buffer[512]={'\0'};
    recv(client_connection_fd, buffer, sizeof(buffer), 0);
    client_connection_fd_list[i]=client_connection_fd;
    FD_SET(client_connection_fd,&active_fd_set);

    strcpy(host_list[i],buffer);
  }


  
  for (int i=0;i<num_play;i++){
    int base=10000;
    player[i].num_play=num_play;
    player[i].index=i;

    if(i==0){
      player[i].left_index=num_play-1;
      player[i].right_index=1;
      strcpy(player[i].left_player,host_list[num_play-1]);
      strcpy(player[i].right_player,host_list[i+1]);
      sprintf(player[i].left_port,"%d",base+num_play-1);
      sprintf(player[i].right_port,"%d",base);
    }
    else if(i==num_play-1){
      player[i].left_index=num_play-2;
      player[i].right_index=0;
      strcpy(player[i].left_player,host_list[i-1]);
      strcpy(player[i].right_player,host_list[0]);
      sprintf(player[i].left_port,"%d",base+i-1);
      sprintf(player[i].right_port,"%d",base+i);
    }
    else{
      player[i].left_index=i-1;
      player[i].right_index=i+1;
      strcpy(player[i].left_player,host_list[i-1]);
      strcpy(player[i].right_player,host_list[i+1]);
      sprintf(player[i].left_port,"%d",base+i-1);
      sprintf(player[i].right_port,"%d",base+i);
    }

  }

  for (int i=0;i<num_play;i++){
    size_t size=sizeof(playernode);
    char message [size];
    memcpy(message, &(player[i]), size);
    send(client_connection_fd_list[i],message,size, 0);
  }
  
  // send the potato to the first client
  if (num_hop!=0){
    potato_info potato;
    potato.num_hop=num_hop-1;
    potato.current_hop=0;
    if (num_hop==0)
      potato.end_game=1;
    else
      potato.end_game=0;

    srand(time(NULL));
    int first = rand()%num_play;

    size_t size_potato=sizeof(potato_info);
    char message[size_potato];
    memcpy(message, &(potato), size_potato);
    send(client_connection_fd_list[first], message, size_potato, 0);
    printf("Ready to start the game, sending potato to player %d\n",first);

    char buffer_potato[sizeof(potato_info)]={'\0'};
    int stop_sign=0;
    while(!stop_sign){
      read_fd_set=active_fd_set;
      if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0){
	//printf ("Error: select\n");
	return -1;
      }
      for (int fd=0;fd<FD_SETSIZE;fd++){
	if(FD_ISSET(fd, &read_fd_set)){
	  recv(fd, buffer_potato, sizeof(buffer_potato), 0);
	  stop_sign=1;
	}
      }
    }

    for (int i=0;i<num_play;i++){
      send(client_connection_fd_list[i],buffer_potato,sizeof(buffer_potato),0);
    }

    potato_info final_potato;
    memcpy(&final_potato, buffer_potato, sizeof(potato_info));
    if(num_hop!=0){
      printf("Trace of potato:\n");
      for (int i=0;i<num_hop;i++){
	printf("%d",final_potato.path[i]);
	if(i!=num_hop-1)
	  printf(",");
      }
      printf("\n");
    }
  }
  else{
    potato_info potato;
    potato.num_hop=0;
    potato.end_game=1;
    size_t size_potato=sizeof(potato_info);
    char message[size_potato];
    memcpy(message, &(potato), size_potato);

    for (int i=0;i<num_play;i++){
      send(client_connection_fd_list[i],message,sizeof(message),0);
    }
  }
  freeaddrinfo(host_info_list);
  close(socket_fd);
  return 0;
}
