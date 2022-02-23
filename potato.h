#include<stdlib.h>
#include<stdio.h>

struct potato{
   int hops;
   int index;
   int len;
};
typedef struct potato potato;

potato * potato_init(int hops, int len);
void resize_players(int * players, int len);
void append_player(potato * p, int * players, int id);
void print_send_potato(int id);
void print_init_potato(int id);
void print_rm_init(int players, int hops);
void print_ready(int id);
void print_hops(potato * p, int * players);
void print_player_end();
void print_connect_init(int id, int tot);
