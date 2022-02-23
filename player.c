#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include "helper.h"

#define HOST_LEN 100
#define PORT 8081

int main(int argc, char *argv[]){
  if(argc != 3){
    perror("Invalid arguments\nUsage: player <server_name> <port>");
    exit(1);
  }

  fd_set master; // master file descriptor list
  fd_set read_fds; // temp file descriptor list for select()
  int fdmax; // maximum file descriptor number
  FD_ZERO(&master); // clear the master and temp sets
  FD_ZERO(&read_fds);
  

  //GET own ip address
  char own_host[HOST_LEN];
  gethostname(own_host, HOST_LEN);
  struct hostent * host_info = gethostbyname(own_host);
  char * ip = inet_ntoa(*((struct in_addr*)host_info->h_addr_list[0]));  
  
  //bind to own socket
  //get port number
  int sock_send = socket(PF_INET , SOCK_STREAM , 0);

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(0);
  sin.sin_addr.s_addr = inet_addr(ip);

  int yes = 1;
  if (setsockopt(sock_send, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("Cannot set socket\n");
      exit(1);
  }

  bind(sock_send, (struct sockaddr *)&sin, sizeof(sin));
  int len = sizeof(sin);
  if (getsockname(sock_send, (struct sockaddr *)&sin, &len) == -1){
    perror("getsockname");
  }
 
  int own_port = ntohs(sin.sin_port);
  if(listen(sock_send, 100) == -1){
    perror("listen error:");
    return -1;
  }
  
  //connect to server
  int sock_rec = get_sock(argv[1], argv[2], 1);
  FD_SET(sock_rec, &master);
  fdmax = sock_rec;

  //send own ip address, port
  send_data(sock_rec, ip, HOST_LEN);
  send_data(sock_rec, &own_port, sizeof(own_port)); 
  
  //get data from ringmaster
  int own_id, left_port, right_port, no_players, hops;
  char left_ip[HOST_LEN], right_ip[HOST_LEN];

  //own data
  int own_len = rec_data(sock_rec, &own_id, sizeof(own_id));
  rec_data(sock_rec, &no_players, sizeof(no_players));
  rec_data(sock_rec, &hops, sizeof(hops)); 

  char own_addr[6];
  sprintf(own_addr, "%d", own_port);
  own_addr[own_len + 1] = '\0';

  print_connect_init(own_id, no_players - 1);

  //right player
  int right_len = rec_data(sock_rec, &right_port, sizeof(right_port)); 
  rec_data(sock_rec, right_ip, HOST_LEN);

  char right_addr[6];
  sprintf(right_addr, "%d", right_port);
  right_addr[right_len + 1] = '\0';
  
  //Connect to right player
  int sock_right = get_sock(right_ip, right_addr, 1);
  int sock_left = sock_accept(sock_send);

  FD_SET(sock_left, &master);
  if(sock_left > fdmax){
    fdmax = sock_left;
  }
  FD_SET(sock_right, &master);
  if(sock_right > fdmax){
    fdmax = sock_right;
  }

  //Special case
  if(hops == 0){
    close_player(sock_rec, sock_left, sock_right, sock_send);
    exit(0);
  }

  //set random time
  srand((unsigned int)time(NULL));

  //init potato, players
  potato *p = potato_init(0, 0);
  int * players = malloc(no_players * sizeof(*players));
  int read_p = 0;

  //recieve potato from rm
  //send potato for first time
  while(1){
    read_fds = master; 
    init_select(fdmax, &read_fds);

    //recieve potato
    if(FD_ISSET(sock_rec,&read_fds)){
      read_p = rec_pot_player(sock_rec, p, players);
    }
    //send potato
    if(read_p != 0){
      send_pot(p, players, own_id, sock_left, sock_right, sock_rec, no_players - 1);
    }
    break;
  }
  
  int read_left_p = 0, read_right_p = 0, read_s = 0;
  char rec_end[4];
  while(1){
     read_fds = master;
     init_select(fdmax, &read_fds);

     
     //recieve end from ringmaster
     if(FD_ISSET(sock_rec, &read_fds)){
       read_s = rec_data(sock_rec, rec_end, 4 * sizeof(*rec_end));
       if(read_s != 0){
	 break;
       }
     }
  
   //left player
     if(FD_ISSET(sock_left,&read_fds)){
       read_left_p = rec_pot_player(sock_left, p, players);
       if(read_left_p != 0){
	 int out = send_pot(p, players, own_id, sock_left, sock_right, sock_rec, no_players - 1);
	if(out == 1){
	  break;
	}
       }
     }
     //right player
     else if(FD_ISSET(sock_right,&read_fds)){
       read_right_p = rec_pot_player(sock_right, p, players);
       if(read_right_p != 0){
	 int out = send_pot(p, players, own_id, sock_left, sock_right, sock_rec, no_players - 1);
	 if(out == 1){
	   break;
	 }
       }
     }

  }

  //free allocd 
  free(p);
  free(players);

  //close sockets;
  close_player(sock_rec, sock_left, sock_right, sock_send);
  
}
