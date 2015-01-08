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
    unsigned int dst_ip;
    char user_id[512];
    char domain_name[512];
};
typedef struct sock4pkt sock4pkt_t;

void sock_reply(int sock_fd, sock4pkt_t pkt, int vaild);
int create_server_sock(int port, struct sockaddr_in* sock_server);
void sock_req(int sock_fd, sock4pkt_t* pkt);

void connect_mode(int client_sock, sock4pkt_t pkt);
void bind_mode(int client_sock, sock4pkt_t pkt);

void exchange_socket_data(int sock_fd1, int sock_fd2);
#endif
