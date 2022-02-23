#include "helper.h"

int get_sock(const char * addr, char * port, int opt){
  struct addrinfo * list = init_addr(addr, port);
  int sock_rec = init_sock(list, opt);
  return sock_rec;
}



struct addrinfo * init_addr(const char * addr, char * port){
  //own address info
  struct addrinfo own, *list;

  //clear out own addrinfo
  memset(&own, 0, sizeof(own));

  //IPv4/IPv6 address both
  own.ai_family = AF_UNSPEC;

  //Connected socket
  own.ai_socktype = SOCK_STREAM;

  //own IP
  if(addr == NULL){
    own.ai_flags = AI_PASSIVE;
  }

  if(getaddrinfo(addr, port, &own, &list) != 0){
     return NULL;
  }
  return list;
}

int init_sock(struct addrinfo * list, int opt){
  int yes = 1;
  int sock_rec;

  struct addrinfo * ptr;
  //traverse list and bind to a socket
  for(ptr = list; ptr != NULL; ptr = ptr->ai_next){

    //try to create socket
    if((sock_rec = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1){
      perror("Socket cannot be init\n");
      exit(1);
    }

    //set socket properties
    if (setsockopt(sock_rec, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("Cannot set socket\n");
      exit(1);
    }

    if(opt == 0){
      //bind to the socket
      if (bind(sock_rec, ptr->ai_addr, ptr->ai_addrlen) == -1) {
	close(sock_rec);
	perror("Socket: bind\n");
	continue;
      }
    }
    else{
      if (connect(sock_rec, ptr->ai_addr, ptr->ai_addrlen) == -1) {
      close(sock_rec);
      //printf("%d\n", sock_rec);
      perror("Socket: connect : \n");
      continue;
    }
    
    }

    //succesful
    break;
  }

  freeaddrinfo(list); // all done with this structure

  return sock_rec;
}

int rec_data(int sock, void * data, int len){
  int n;
  if ((n = recv(sock, data, len, 0)) == -1) {
    perror("Error:recv");
    exit(1);
  }
  return n;
}

void send_data(int sock, void * data, int len){
   if (send(sock, data, len, 0) == -1){
      perror("Error:send");
      exit(1);
    }
}  

int sock_accept(int sock){
  struct sockaddr_storage socket_addr;
  socklen_t socket_len = sizeof(socket_addr);
  int n;
  if((n = accept(sock, (struct sockaddr *)&socket_addr, &socket_len)) == -1){
    perror("Accept");
    exit(1);
  }
  return n;
}

void send_pot_player(int sock, potato * p, int * players){
   send_data(sock, p, sizeof(*p));
   send_data(sock, players, p->len * sizeof(*players));
}

int rec_pot_player(int sock, potato * p, int * players){
    int read_p = rec_data(sock, p, sizeof(*p));
    int read_pl = rec_data(sock, players, p->len * sizeof(*players));
    //printf("read %d %d bits\n", read_p, read_pl);
    
    if(read_p == 0 || read_pl == 0){
      return 0;
    }
    return 1;
}

void init_select(int fdmax, fd_set * read_fds){
  if (select(fdmax+1, read_fds, NULL, NULL, NULL) == -1) {
      perror("select");
      exit(1);
  }
}

int rand_select(int n){  
   int player_sel = rand() % n;
   //printf("selected in helper : %d", player_sel);
   return player_sel;
}

void send_potato(potato * p, int * players, int sock_left, int sock_right, int own_id, int no){
   int player_sel = rand_select(2);
   if(player_sel == 0){
     int right = (own_id + 1) % no;
      print_send_potato(right);
      send_pot_player(sock_right, p, players);
   }
   else{
      int left = own_id - 1;
      if(left < 0){
	left = no;
      }
      print_send_potato(left);
      send_pot_player(sock_left, p, players);
   }
}

int send_pot(potato * p, int * players, int own_id, int sock_left, int sock_right, int sock_rec, int no){
      append_player(p, players, own_id);

      //testing
      // print_hops(p, players);
      //printf("\n NO OF HOPS %d\n", p->hops);
      //end, hops ended
      if(p->hops <= 0){
	print_player_end();
	send_pot_player(sock_rec, p, players);
	return 1;
      }
 
      send_potato(p, players, sock_left, sock_right, own_id, no);
      return 0;
}

void close_player(int sock_rec, int sock_left, int sock_right, int sock_send){
  close(sock_rec);
  close(sock_left);
  close(sock_right);
  close(sock_send);    
}

void close_rm(int sock_rec, int sockets[], int no_players){
  close(sock_rec);
  for(int i = 0; i < no_players; i++){
    close(sockets[i]);
  }
}

void free_rm(potato * p, int * players, char * players_addr[], int no_players){
  free(p);
  free(players);
  for(int i = 0; i < no_players; i++){
    free(players_addr[i]);
  }
}

