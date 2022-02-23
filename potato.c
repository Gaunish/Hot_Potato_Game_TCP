#include "potato.h"

potato * potato_init(int hop, int len){
  potato * p = malloc(sizeof(*p));
  p->hops = hop;
  p->index = 0;
  p->len = len;

  return p;
}

void resize_players(int * players, int len){
  players = (int *)realloc(players, 2 * len * sizeof(*players));
}

void dec_hop(potato * p){
  p->hops -= 1;
}

void inc_index(potato *p){
  p->index += 1;
}

void append_player(potato * p, int * players, int id){
  dec_hop(p);

  //append player
  players[p->index] = id;

  inc_index(p);
}

void print_send_potato(int id){
  printf("Sending potato to player %d\n", id); 
}

void print_connect_init(int id, int tot){
  printf("Connected as player %d out of %d total players\n", id, tot);
}

void print_init_potato(int id){
  printf("Ready to start the game, sending potato to player %d\n", id);
  printf("Trace of potato:\n");
}

void print_rm_init(int players, int hops){
  printf("Potato Ringmaster\n");
  printf("Players = %d\n", players);
  printf("Hops = %d\n", hops);
}

void print_ready(int id){
  printf("Player %d is ready to play\n", id);
}

void print_hops(potato * p, int * players){
  for(int i = 0; i < p->index; i++){
    printf("%d", players[i]);
    if(i < p->index - 1){
      printf(",");
    }
  }
  printf("\n");
}

void print_player_end(){
  printf("I’m it\n");
}
