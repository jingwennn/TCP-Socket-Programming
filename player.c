#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include "potato.h"

int main(int argc, char* argv[])
{
  if(argc!=3){
    //printf("Error: wrong arguments.\n");
    return -1;
  }
  char host[512] = {0};
  if(gethostname(host,sizeof(host)) < 0){
    //printf("Error: gethostname\n");
    return -1;
  }

  struct hostent *hp;
  /*
    if ((hp=gethostbyname(host)) == NULL){
        printf("Error: gethostbyname\n");
        return -1;
    }   
    strcpy(host, hp->h_name);
  */
  srand(time(NULL));
  char *hostname=argv[1];
  char *port=argv[2];
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  fd_set active_fd_set, read_fd_set;
  FD_ZERO (&active_fd_set);

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    //printf("Error: cannot get address info for host\n");
    return -1;
  } //if
  //create an endpoint for communication
  socket_fd = socket(host_info_list->ai_family,
		     host_info_list->ai_socktype,
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    printf("Error: cannot create socket\n");
    return -1;
  } //if
  status=-1;
  while(status==-1)
    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if(status==-1){
    printf("Error0: cannot connect\n");
    return -1;
  }
 
  FD_SET(socket_fd,&active_fd_set);

  send(socket_fd, host, strlen(host), 0);
  //printf("socket_fd:%d\n",socket_fd);
  //printf("tag1\n");

  char buffer[sizeof(playernode)]={'\0'};
  recv(socket_fd, buffer, sizeof(buffer), 0);
  playernode player;
  memcpy(&player, buffer, sizeof(playernode));
  printf("Connected as player %d out of %d total players\n",player.index, player.num_play);
  

  
  int socket_fd_left;
  int socket_fd_right;
  struct addrinfo host_info_left;
  struct addrinfo host_info_right;
  struct addrinfo *host_info_list_left;
  struct addrinfo *host_info_list_right;

  memset(&host_info_left, 0, sizeof(host_info_left));
  memset(&host_info_right, 0, sizeof(host_info_right));
  host_info_right.ai_family   = AF_UNSPEC;
  host_info_right.ai_socktype = SOCK_STREAM;
  host_info_right.ai_flags    = AI_PASSIVE;
  host_info_left.ai_family   = AF_UNSPEC;
  host_info_left.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(player.right_player, player.right_port, &host_info_right, &host_info_list_right);
  if (status != 0) {
    //printf("Error: cannot get address info for host\n");
    return -1;
  } //if

  socket_fd_right = socket(host_info_list_right->ai_family,
			   host_info_list_right->ai_socktype,
			   host_info_list_right->ai_protocol);
  if (socket_fd_right == -1) {
    //printf("Error: cannot create socket\n");
    return -1;
  } //if
  int yes = 1;
  status = setsockopt(socket_fd_right, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd_right, host_info_list_right->ai_addr, host_info_list_right->ai_addrlen);
  if (status == -1) {
    printf("Error: cannot bind socket\n");
    return -1;
  } //if
  status = listen(socket_fd_right, 100);
  if (status == -1) {
    printf("Error: cannot listen on socket\n");
    return -1;
  } //if
  // as a client
  status = getaddrinfo(player.left_player, player.left_port, &host_info_left, &host_info_list_left);
  if (status != 0) {
    printf("Error: cannot get address info for host\n");
    return -1;
  } //if


  //create an endpoint for communication
  socket_fd_left = socket(host_info_list_left->ai_family,
			  host_info_list_left->ai_socktype,
			  host_info_list_left->ai_protocol);
  if (socket_fd_left == -1) {
    //printf("Error: cannot create socket\n");
    return -1;
  } //if

  int client_connection_fd=-1;
  status=-1;
  client_connection_fd=-1;
    while(client_connection_fd == -1 || status == -1){
    status = connect(socket_fd_left, host_info_list_left->ai_addr, host_info_list_left->ai_addrlen);
    //if(status==-1)
    //printf("fail\n");
    //else
    //printf("success connect\n");
    
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    if(client_connection_fd==-1)
      client_connection_fd = accept(socket_fd_right, (struct sockaddr *)&socket_addr, &socket_addr_len);
    //printf("%d\n",client_connection_fd);
     }
  //printf("tag1\n");
  //return 0;
  FD_SET(socket_fd_left,&active_fd_set);
  FD_SET(client_connection_fd,&active_fd_set);
  int stop_sign=0;
  while(!stop_sign){
    //receive the potato
    read_fd_set=active_fd_set;
    if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0){
      printf ("Error: select\n");
      return -1;
    }
    for (int fd=0;fd<FD_SETSIZE;fd++){
      if(FD_ISSET(fd, &read_fd_set)){
	char buffer_potato[sizeof(potato_info)]={'\0'};
	int databytes=recv(fd, buffer_potato, sizeof(buffer_potato), 0);
	if(databytes <= 0){
	  close(fd);
	  FD_CLR(fd,&active_fd_set);
	  exit(EXIT_SUCCESS);
	}
	potato_info current_potato;
	memcpy(&current_potato, buffer_potato, sizeof(potato_info));
	/*
	printf("current hop: %d\n",current_potato.current_hop);
	printf("hop num: %d\n",current_potato.num_hop);
	if(fd==socket_fd){
	  printf("from ringmaster.\n");
	}
	*/
	if(current_potato.end_game==1){
	  stop_sign=1;
	  break;
	}

	current_potato.path[current_potato.current_hop]=player.index;
	current_potato.current_hop+=1;
	int fd_next;
	if(current_potato.current_hop==current_potato.num_hop+1){
	  fd_next=socket_fd;
	  current_potato.end_game=1;
	  printf("I'm it\n");
	  stop_sign=1;

	}
	else{
	  int next = rand()%2;
	  if (next==0){
	    fd_next=socket_fd_left;
	    printf("Sending potato to %d\n",player.left_index);
	  }
	  else{
	    fd_next=client_connection_fd;
	    printf("Sending potato to %d\n",player.right_index);
	  }

	}

	size_t size=sizeof(potato_info);
	char next_potato [size];
	memcpy(next_potato, &current_potato, size);
	send(fd_next,next_potato,size, 0);

      }
    }
  }

  freeaddrinfo(host_info_list);
  close(socket_fd);
  freeaddrinfo(host_info_list_left);
  close(socket_fd_left);
  freeaddrinfo(host_info_list_right);
  close(socket_fd_right);
  return 0;
}
