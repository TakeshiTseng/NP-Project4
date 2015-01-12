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
            unsigned int src_ip = client_addr.sin_addr.s_addr;
            printf("Permit Src = %d.%d.%d.%d(%d), ", src_ip&0xff, src_ip>>8&0xff, src_ip>>16&0xff, src_ip>>24, client_addr.sin_port);
            printf("Dst = %d.%d.%d.%d(%d)\n", pkt.dst_ip>>24, pkt.dst_ip>>16&0xff, pkt.dst_ip>>8&0xff, pkt.dst_ip&0xff, pkt.dst_port);
            if(pkt.cd == 1){
                // connect mode
                // connect to http server
                struct sockaddr_in server;
                int sock = socket(AF_INET , SOCK_STREAM , 0);
                server.sin_addr.s_addr = htonl(pkt.dst_ip);
                server.sin_family = AF_INET;
                server.sin_port = htons(pkt.dst_port);


                if(firewall_check(pkt.dst_ip) == 1){
                    if(connect(sock, (struct sockaddr *)&server , sizeof(server)) < 0) {
                        sock_reply(client_sock, pkt, 0);
                        printf("SOCKS_CONNECT DENY ....\n");
                    } else {
                        sock_reply(client_sock, pkt, 1);
                        printf("SOCKS_CONNECT GRANTED ....\n");
                        exchange_socket_data(sock, client_sock);
                    }
                    
                } else {
                    sock_reply(client_sock, pkt, 0);
                    printf("SOCKS_CONNECT DENY ....\n");
                }

                return 0;
            } else if(pkt.cd == 2) {
                // Bind mode! Let's rock!!!
                struct sockaddr_in bind_server;

                port = 50000 + rand() % 1000;
                int bind_sc_fd = create_server_sock(port, &bind_server);

                while(bind_sc_fd < 0) {
                    sleep(1);
                    port = 50000 + rand() % 1000;
                    printf("[Bind] Rebind to port : %d\n", port);
                    bind_sc_fd = create_server_sock(port, &bind_server);
                }


                if(bind_sc_fd < 0) {
                    perror("[Bind] Create error\n");
                    sock_reply(client_sock, pkt, 0);
                    return 0;
                }

                pkt.dst_ip = 0;
                pkt.dst_port = port;

                sock_reply(client_sock, pkt, 1);
                printf("SOCKS_BIND GRANTED ....\n");
                int sock = accept(bind_sc_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);
                sock_reply(client_sock, pkt, 1);
                printf("SOCKS_BIND GRANTED ....\n");
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
