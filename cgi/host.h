#ifndef __HOST_H__
#define __HOST_H__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>


struct host {
    // xxx.xxx.xxx.xxx
    char hostname[21];
    int port;
    char filename[128];
    int server_fd;
    FILE* host_file;
    int is_connect;
    char sock_server[21];
    int sock_port;
    struct sockaddr_in server;
};

typedef struct host host_t;

void create_host(host_t** host, char* hostname, int port, char* filename);

#endif
