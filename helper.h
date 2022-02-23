#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include<time.h>
#include "potato.h"

int get_sock(const char * addr, char * port, int opt);
struct addrinfo * init_addr(const char * addr, char * port);
int init_sock(struct addrinfo * list, int opt);
int rec_data(int sock, void * data, int len);
void send_data(int sock, void * data, int len);
int sock_accept(int sock);
void send_pot_player(int sock, potato * p, int * players);
int rec_pot_player(int sock, potato * p, int * players);
void init_select(int fdmax, fd_set * read_fds);
int rand_select(int n);
void send_potato(potato * p, int * players, int sock_left, int sock_right, int own_id, int no);
int send_pot(potato * p, int * players, int own_id, int sock_left, int sock_right, int sock_rec, int no);
void close_player(int sock_rec, int sock_left, int sock_right, int sock_send);
void close_rm(int sock_rec, int sockets[], int no_players);
void free_rm(potato * p, int * players, char * players_addr[], int no_players);
