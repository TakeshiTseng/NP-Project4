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
            unsigned char buffer[8192];
            bzero(buffer, 8192);
            int len = read(client_sock, buffer, 8192);
            printf("Header lenght: %d\n", len);
            unsigned char VN = buffer[0] ;
            unsigned char CD = buffer[1] ;
            unsigned int DST_PORT = buffer[2] << 8 | buffer[3] ;
            unsigned int DST_IP = buffer[4] << 24 | buffer[5] << 16 | buffer[6] << 8 | buffer[7] ;
            char* USER_ID = (char*)buffer + 8 ;
            char* DOMAIN_NAME;


            printf("VN: %u\n", VN);
            printf("CD: %u\n", CD);
            printf("DST_PORT: %u\n", DST_PORT);
            printf("DST_IP: %u\n", DST_IP);
            printf("%u %u %u %u\n", buffer[4], buffer[5],  buffer[6], buffer[7]);
            printf("USER_ID: %s\n", USER_ID);
            if(buffer[4] == 0 && buffer[5] == 0 && buffer[6] == 0) {
                DOMAIN_NAME = USER_ID + strlen(USER_ID) + 1;
                printf("DOMAIN_NAME: %s\n", DOMAIN_NAME);
            }


            
            // connect to http server
            struct sockaddr_in server;
            int sock = socket(AF_INET , SOCK_STREAM , 0);
            char ip_addr[17]; //xxx.xxx.xxx.xxx
            sprintf(ip_addr, "%u.%u.%u.%u", buffer[4], buffer[5],  buffer[6], buffer[7]);
            server.sin_addr.s_addr = inet_addr(ip_addr);
            server.sin_family = AF_INET;
            server.sin_port = htons(DST_PORT);
            int res = 90;
            if(connect(sock, (struct sockaddr *)&server , sizeof(server)) < 0) {
                res = 91;
            }

            printf("Make request:\n");
            unsigned char package[8];
            package[0] = 0;
            package[1] = (unsigned char) res ; // 90 or 91
            package[2] = DST_PORT / 256;
            package[3] = DST_PORT % 256;
            package[4] = DST_IP >> 24;
            package[5] = (DST_IP >> 16) & 0xFF;
            package[6] = (DST_IP >> 8)  & 0xFF;
            package[7] = DST_IP & 0xFF;

            for(int c=0; c<8; c++) {
                printf("package[%d] = %u\n", c, package[c]);
            }
            write(client_sock, package, 8);

            if(res == 90){
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
                        len = read(sock, buffer, 8192);
                        if(len > 0){
                            printf("Data read from sock: %d\n", len);
                            fflush(stdout);
                            write(client_sock, buffer, len);
                        }
                        
                    } else if(FD_ISSET(client_sock, &rfds)) {
                        len = read(client_sock, buffer, 8192);
                        if(len > 0){
                            printf("Data read from client sock: %d\n", len);
                            fflush(stdout);
                            write(sock, buffer, len);
                        }
                        
                    }

                    //FD_ZERO(&rfds);

                }

                close(sock);
            }
            close(client_sock);

            exit(0);
        } else {
            close(client_sock);
        }
    }
    close(sc_fd);
    return 0;
}
