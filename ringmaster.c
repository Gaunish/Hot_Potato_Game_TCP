#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>  
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include "helper.h"

#define HOST_LEN 100

int main(int argc, char ** argv){
  //Invalid arguments
  if(argc != 4){
    perror("Invalid arguments\nUsage: ringmaster <port_num> <num_players> <num_hops>");
    exit(1);
  }
  if(atoi(argv[2]) <= 1){
    perror("Invalid no of players");
    exit(1);
  }
  if(atoi(argv[3]) < 0 || atoi(argv[3]) > 512){
    perror("Invalid no of hops");
    exit(1);
  }

  //master file descriptor, temp fd
  fd_set master, readfd;
  //max fd no
  int maxfd = -1;
  //clear the fds
  FD_ZERO(&master);
  FD_ZERO(&readfd);

  //bind to own socket
  int sock_rec = get_sock(NULL, argv[1], 0);

  //Connect to given no of players
  //keep track of id of each player
  int id = 0;
  int no_players = atoi(argv[2]);

  //init potato
  int no_hops = atoi(argv[3]);
  potato * p = potato_init(no_hops, no_hops);
  int * players = malloc(no_hops * sizeof(*players));

  //print init
  print_rm_init(no_players, no_hops);
  
  //wait for incoming connections
  if(listen(sock_rec, no_players) == -1){
    return -1;
  }
 
  //init player related infos
  char * players_addr[no_players];
  for(int i = 0; i < no_players; i++){
    players_addr[i] = malloc(HOST_LEN * sizeof(*players_addr[i]));
  }
  int sockets[no_players], ports[no_players];
  
  //get connections
  while(id < no_players){
    //accept the socket
    int sock_send = sock_accept(sock_rec);
    if(sock_send == -1){
      continue;
    }
 
    //get connected socket fd
    sockets[id] = sock_send;
    
    //store connected ip address
    rec_data(sock_send, players_addr[id], HOST_LEN);
    rec_data(sock_send, &ports[id], sizeof(ports[id]));

    print_ready(id);

    //add to select list
    FD_SET(sockets[id], &master);
    if(sockets[id] > maxfd){
      maxfd = sockets[id];
    }
    
    id++;
  }

  //reset id 
  id = 0;
 
  //send connection infos
  while(id < no_players){
    //get left id
    int left = id - 1;
    if(id == 0){
      left = no_players - 1;
    }
    //get right id
    int right = (id + 1) % no_players;
  
    //send own id, no_of_players, hops
    send_data(sockets[id], &id, sizeof(id));
    send_data(sockets[id], &no_players, sizeof(no_players));
    send_data(sockets[id], &no_hops, sizeof(no_hops));


    //left player port, ip
    send_data(sockets[id], &ports[left], sizeof(ports[left]));
    send_data(sockets[id], players_addr[left], HOST_LEN);

    id++;
  }

  //special case
  if(no_hops == 0){
    close_rm(sock_rec, sockets, no_players);
    free_rm(p, players, players_addr, no_players);
    exit(0);
  }
  
  //select random player
  srand((unsigned int)time(NULL)); 
  int player_sel = rand_select(no_players);

  //print potato send
  print_init_potato(player_sel);
  
  //send potato
  send_pot_player(sockets[player_sel], p, players);

  //get finished potato when available
  int found = 0;
  while(1){
     readfd = master;
     init_select(maxfd, &readfd);

     //get finished game output
     for(int i = 0; i < no_players; i++){
       if(FD_ISSET(sockets[i], &readfd)){
	 int rec_q = rec_pot_player(sockets[i], p, players);
	 if(rec_q != 0){
	   found = 1;
	   break;
	 }
       }
     }
     if(found == 1){
       break;
     }
  }
  
  //print hops
  print_hops(p, players);

  //end player process
  char end[4] = "end";
  for(int i = 0; i < no_players; i++){
    send_data(sockets[i], end, 4 * sizeof(*end));
    }
    
  //close sockets
  close_rm(sock_rec, sockets, no_players);
  free_rm(p, players, players_addr, no_players);


  return 0;
}
