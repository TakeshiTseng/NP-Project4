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
#include "socks.h"

void handleSIGCHLD() {
    int stat;

    /*Kills all the zombie processes*/
    while(waitpid(-1, &stat, WNOHANG) > 0);
    // while(wait3(&stat, WNOHANG, (struct rusage*)0)>=0);
}

int main(int argc, const char *argv[])
{

    signal(SIGCHLD, handleSIGCHLD);
    int c;
    srand(time(NULL));
    int port = 8000 + rand() % 100;
    printf("Port : %d\n", port);

    struct sockaddr_in sock_server;
    int sc_fd = create_server_sock(port, &sock_server);

    if(sc_fd < 0) {
        return -1;
    }

    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);
    printf("Accepting.....\n");

    while(1 == 1){
        int client_sock = accept(sc_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);
        if(client_sock < 0) {
            perror("accept error");
            break;
        }
        int pid = fork();

        if(pid == 0){
            sock4pkt_t pkt;
            sock_req(client_sock, &pkt);


            printf("VN: %u\n", pkt.vn);
            printf("CD: %u\n", pkt.cd);
            printf("DST_PORT: %u\n", pkt.dst_port);
            printf("DST_IP: %u\n", pkt.dst_ip);

            if(pkt.cd == 1){
                // connect mode
                // connect to http server
                struct sockaddr_in server;
                int sock = socket(AF_INET , SOCK_STREAM , 0);
                server.sin_addr.s_addr = htonl(pkt.dst_ip);
                server.sin_family = AF_INET;
                server.sin_port = htons(pkt.dst_port);
                if(connect(sock, (struct sockaddr *)&server , sizeof(server)) < 0) {
                    sock_reply(client_sock, pkt, 0);
                } else {
                    sock_reply(client_sock, pkt, 1);
                    exchange_socket_data(sock, client_sock);
                }

                return 0;
            } else if(pkt.cd == 2) {
                // Bind mode! Let's rock!!!
                printf("[Bind] Bind mode\n");
                // for bind server
                struct sockaddr_in bind_server;
                printf("[Bind] Createing server sock\n");


                port = 9000 + rand() % 100;
                int bind_sc_fd = create_server_sock(port, &bind_server);

                while(bind_sc_fd < 0) {
                    printf("Rebind port\n");
                    sleep(1);
                    port = 9000 + rand() % 100;
                    bind_sc_fd = create_server_sock(port, &bind_server);
                }

                printf("[Bind] Bind server port: %d\n", bind_server.sin_port);

                if(bind_sc_fd < 0) {
                    perror("[Bind] Create error\n");
                    sock_reply(client_sock, pkt, 0);
                    return 0;
                }

                pkt.dst_ip = 0;
                pkt.dst_port = port;

                sock_reply(client_sock, pkt, 1);

                printf("[Bind] accepting.....\n");
                int sock = accept(bind_sc_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);
                printf("[Bind] OK, Accept!\n");
                sock_reply(client_sock, pkt, 1);

                exchange_socket_data(sock, client_sock);
                close(bind_sc_fd);
                return 0;
            }
        } else {
            // parent doesn't need client sock, close it.
            close(client_sock);
        }

    }
    close(sc_fd);
    return 0;
}
