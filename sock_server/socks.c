#include "socks.h"

void sock_req(int sock_fd, sock4pkt_t* pkt) {
    byte_t buffer[1024];
    bzero(buffer, 1024);
    int len = read(sock_fd, buffer, 1024);
    pkt->vn = buffer[0] ;
    pkt->cd = buffer[1] ;
    pkt->dst_port = buffer[2] << 8 | buffer[3] ;
    pkt->dst_ip = buffer[4] << 24 | buffer[5] << 16 | buffer[6] << 8 | buffer[7] ;

    bzero(pkt->user_id, 512);
    strcpy(pkt->user_id, &(buffer[8]));

    if(buffer[4] == 0 && buffer[5] == 0 && buffer[6] == 0) {
        int pos_of_dn = len-2;
        while(buffer[pos_of_dn] != '\0') {
            pos_of_dn--;
        }
        pos_of_dn++;

        bzero(pkt->domain_name, 512);
        strcpy(pkt->domain_name, &(buffer[pos_of_dn]));

        pkt->dst_ip = htonl(get_ip_by_domain_name(pkt->domain_name));
    }

    printf("Pkt length: %d\n", len);
    printf("VN: %d, ", pkt->vn);
    printf("CD: %d, ", pkt->cd);
    printf("DST IP: %u.%u.%u.%u, ", buffer[4], buffer[5], buffer[6], buffer[7]);
    printf("DST PORT: %d, ", pkt->dst_port);
    printf("USERID: %s", pkt->user_id);
    if(buffer[4] == 0 && buffer[5] == 0 && buffer[6] == 0) {
        printf(", DOMAIN NAME: %s\n", pkt->domain_name);
    } else {
        printf("\n");
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


    char buffer[1048576];
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
        bzero(buffer, 1048576);

        if(FD_ISSET(sock_fd1, &rfds)) {
            int len = read(sock_fd1, buffer, 1048576);
            if(len > 0){
                write(sock_fd2, buffer, len);
            }
            if(len == 0) {
                close(sock_fd1);
                FD_CLR(sock_fd1, &afds);
            }
        }

        if(FD_ISSET(sock_fd2, &rfds)) {
            int len = read(sock_fd2, buffer, 1048576);
            if(len > 0){
                write(sock_fd1, buffer, len);
            }
            if(len == 0) {
                close(sock_fd2);
                FD_CLR(sock_fd2, &afds);
            }

        }


    }

}

int firewall_check(unsigned int dst_ip) {
    FILE* file = fopen("socks.conf", "r");
    char buf[1024];
    fscanf(file, "%s", buf);
    fclose(file);

    printf("Filewall %s\n", buf);

    if(strcmp(buf, "NCTU") == 0) {
        printf("%d, %d\n", dst_ip >> 24, dst_ip >> 16 & 0xff);
        if((dst_ip >> 24) == 140 && (dst_ip >> 16 & 0xff) == 113) {
            return 1;
        } else {
            return 0;
        }
    } else if(strcmp(buf, "NCHU") == 0) {
        if((dst_ip >> 24) == 140 && (dst_ip >> 16 & 0xff) == 114) {
            return 1;
        } else {
            return 0;
        }
    } else {
        // grant all
        return 1;
    }

}
