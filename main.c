#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include "passivesock.h"


void handleSIGCHLD() {
    int stat;

    /*Kills all the zombie processes*/
    while(waitpid(-1, &stat, WNOHANG) > 0);
    // while(wait3(&stat, WNOHANG, (struct rusage*)0)>=0);
}

int main(int argc, const char *argv[])
{

    signal(SIGCHLD, handleSIGCHLD);

    srand(time(NULL));
    int port = 2000 + rand() % 100;
    printf("Port : %d\n", port);
    char port_str[5];
    sprintf(port_str, "%d", port);
    int sc_fd = passivesock(port_str, "tcp", 5);
    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);
    printf("accepting.....\n");

    while(1 == 1){
        int client_sock = accept(sc_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);
        int pid = fork();

        if(pid == 0){
            unsigned char vn[1];
            read(client_sock, vn, 1);
            printf("vn %d\n", vn[0]);

            unsigned char cd[1];
            read(client_sock, cd, 1);
            printf("cd %d\n", cd[0]);

            unsigned char dst_port[2];
            read(client_sock, dst_port, 2);
            int port = ((int)dst_port[0]) << 8 | (int)dst_port[1];
            printf("dst_port %d\n", port);

            unsigned char dst_ip[4];
            read(client_sock, dst_ip, 4);
            int ip = ((int)dst_ip[0] << 24) | ((int)dst_ip[1] << 16) | ((int)dst_ip[2] << 8) | (int)dst_ip[3];
            printf("dst_ip %d\n", ip);
            printf("dst_ip %d.%d.%d.%d\n", dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);


            printf("data:\n");
            char buff[1024];
            bzero(buff, 1024);
            while(read(client_sock, buff, 1024) != 0) {
                printf("%s", buff);
                bzero(buff, 1024);
            }

            close(client_sock);
            return 0;
        }
    }
    close(sc_fd);
    return 0;
}
