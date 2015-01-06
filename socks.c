#include "socks.h"



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

int create_server_sock(int port, struct sockaddr_in* sock_server) {

    int sc_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(sc_fd < 0) {
        perror("Create sock server error");
        return -1;
    }

    bzero((char *) sock_server, sizeof(struct sockaddr_in));
    (*sock_server).sin_family = AF_INET;
    (*sock_server).sin_addr.s_addr = INADDR_ANY;
    (*sock_server).sin_port = htons(port);

    if(bind(sc_fd, (struct sockaddr *) sock_server, sizeof(struct sockaddr_in)) < 0) {
        perror("Bind error");
        return -1;
    }

    if(listen(sc_fd, 10) < 0) {
        perror("Listen error");
        return -1;
    }

    return sc_fd;
}

void exchange_socket_data(int sock_fd1, int sock_fd2) {


    char buffer[8192];
    fd_set rfds, afds;
    int nfds = sock_fd2>sock_fd1?sock_fd2+1:sock_fd1+1;

    FD_ZERO(&afds);
    FD_SET(sock_fd2, &afds);
    FD_SET(sock_fd1, &afds);
    while(1 == 1) {

        memcpy(&rfds, &afds, sizeof(rfds));

        if(select(nfds, &rfds, NULL, NULL, NULL) < 0) {
            perror("[Conn]select error");
            fflush(stdout);
            close(sock_fd1);
            close(sock_fd2);
            break;
        }
        bzero(buffer, 8192);

        if(FD_ISSET(sock_fd1, &rfds)) {
            int len = read(sock_fd1, buffer, 8192);
            if(len > 0){
                printf("Data read from sock_fd1: %d\n", len);
                fflush(stdout);
                write(1, buffer, len>100?100:len);
                write(1, "\n", 1);
                write(sock_fd2, buffer, len);
            }
        }

        if(FD_ISSET(sock_fd2, &rfds)) {
            int len = read(sock_fd2, buffer, 8192);
            if(len > 0){
                printf("Data read from sock_fd2: %d\n", len);
                fflush(stdout);
                write(1, buffer, len>100?100:len);
                write(1, "\n", 1);
                write(sock_fd1, buffer, len);
            }
        } 


    }

}
