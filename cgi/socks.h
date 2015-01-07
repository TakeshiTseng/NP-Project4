#ifndef __SOCKS_H__
#define __SOCKS_H__

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef unsigned char byte_t;
struct sock4pkt {
    int vn;
    int cd;
    int dst_port;
    int dst_ip;
    char* user_id;
    char* domain_name;
};
typedef struct sock4pkt sock4pkt_t;

void send_sock(int sock_fd, sock4pkt_t pkt);
int create_server_sock(int port, struct sockaddr_in* sock_server);
void get_sock(int sock_fd, sock4pkt_t* pkt);

void connect_mode(int client_sock, sock4pkt_t pkt);
void bind_mode(int client_sock, sock4pkt_t pkt);

void exchange_socket_data(int sock_fd1, int sock_fd2);
#endif