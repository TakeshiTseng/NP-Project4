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

void sock_req(int sock_fd, sock4pkt_t* pkt) {
    byte_t buffer[1024];
    bzero(buffer, 1024);
    read(sock_fd, buffer, 1024);
    pkt->vn = buffer[0] ;
    pkt->cd = buffer[1] ;
    pkt->dst_port = buffer[2] << 8 | buffer[3] ;
    pkt->dst_ip = buffer[4] << 24 | buffer[5] << 16 | buffer[6] << 8 | buffer[7] ;

    if(buffer[8] != '\0') {
        int len = strlen(&(buffer[8]));
        pkt->user_id = malloc(len + 1);
        bzero(pkt->user_id, len+1);
        strcpy(pkt->user_id, &(buffer[8]));
        if(buffer[4] == 0 && buffer[5] == 0 && buffer[6] == 0) {
            int do_len = strlen(&(buffer[8 + len + 1]));
            pkt->domain_name = malloc(do_len+1);
            bzero(pkt->domain_name, do_len+1);
            strcpy(pkt->domain_name, &(buffer[8 + len + 1]));
            printf("DOMAIN_NAME: %s\n", pkt->domain_name);
        }

    }
}

void sock_reply(int sock_fd, sock4pkt_t pkt, int vaild) {

    byte_t package[8];

    package[0] = 0;
    package[1] = vaild==1?90:91 ; // 90 or 91
    package[2] = pkt.dst_port / 256;
    package[3] = pkt.dst_port  % 256;
    package[4] = pkt.dst_ip  >> 24;
    package[5] = (pkt.dst_ip >> 16) & 0xFF;
    package[6] = (pkt.dst_ip >> 8)  & 0xFF;
    package[7] = pkt.dst_ip & 0xFF;

    write(sock_fd, package, 8);

}

void handleSIGCHLD() {
    int stat;

    /*Kills all the zombie processes*/
    while(waitpid(-1, &stat, WNOHANG) > 0);
    // while(wait3(&stat, WNOHANG, (struct rusage*)0)>=0);
}

void create_server_sock(int port) {
    struct sockaddr_in sock_server;
    sock_server.sin_family = AF_INET;
    sock_server.sin_addr.s_addr = INADDR_ANY;
    sock_server.sin_port = htons(port);
    memset((coid*)&(sock_server.sin_zero), 0, 8);

    return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

int main(int argc, const char *argv[])
{

    signal(SIGCHLD, handleSIGCHLD);
    int c;
    srand(time(NULL));
    int port = 2000 + rand() % 100;
    printf("Port : %d\n", port);

    int sc_fd = create_server_sock(port);

    if(sc_fd < 0) {
        perror("Create sock server error");
        return -1;
    }

    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);
    printf("Accepting.....\n");

    while(1 == 1){
        int client_sock = accept(sc_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);
        int pid = fork();

        if(pid == 0){
            unsigned char buffer[8192];
            bzero(buffer, 8192);
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

                    fd_set rfds, afds;
                    int nfds = sock>client_sock?sock+1:client_sock+1;
                    FD_ZERO(&afds);
                    FD_SET(sock, &afds);
                    FD_SET(client_sock, &afds);
                    while(1 == 1) {

                        memcpy(&rfds, &afds, sizeof(rfds));

                        if(select(nfds, &rfds, NULL, NULL, NULL) < 0) {
                            perror("select error");
                            fflush(stdout);
                            //close(sock);
                            close(client_sock);
                            break;
                        }
                        bzero(buffer, 8192);
                        if(FD_ISSET(sock, &rfds)) {
                            int len = read(sock, buffer, 8192);
                            if(len > 0){
                                printf("Data read from sock: %d\n", len);
                                fflush(stdout);
                                write(client_sock, buffer, len);
                            }
                        } else if(FD_ISSET(client_sock, &rfds)) {
                            int len = read(client_sock, buffer, 8192);
                            if(len > 0){
                                printf("Data read from client sock: %d\n", len);
                                fflush(stdout);
                                write(sock, buffer, len);
                            }
                        }

                    }

                    close(sock);
                }
                close(client_sock);

                exit(0);
            } else if(pkt.cd == 2) {
                // Bind mode! Let's rock!!!

                int bind_sc_fd; // TODO
                struct sockaddr_in server;
                int sock = socket(AF_INET , SOCK_STREAM , 0);
                server.sin_addr.s_addr = pkt.dst_ip;
                server.sin_family = AF_INET;
                server.sin_port = htons(pkt.dst_port);
                if(connect(sock, (struct sockaddr *)&server , sizeof(server)) < 0) {
                    perror("[bind]connect error");
                    sock_reply(client_sock, pkt, 0);
                } else {
                    pkt.dst_ip = 0;
                    pkt.dst_port = 0; // TODO
                    sock_reply(client_sock, pkt, 1);

                    printf("Wait for bind client.....\n");
                    int bind_client_sock = accept(bind_sc_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);
                    printf("OK, Bind!\n");
                    sock_reply(client_sock, pkt, 1);

                    fd_set rfds, afds;
                    int nfds = sock>bind_client_sock?sock+1:bind_client_sock+1;
                    FD_ZERO(&afds);
                    FD_SET(sock, &afds);
                    FD_SET(bind_client_sock, &afds);


                    close(bind_client_sock);
                }
                close(bind_sc_fd);
            }
        } else {
            close(client_sock);
        }
    }
    close(sc_fd);
    return 0;
}
