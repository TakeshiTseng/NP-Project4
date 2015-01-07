#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "host.h"
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

int read_line(int fd, char* buffer) {
    int count = 0;
    char buf[1];
    while(read(fd, buf, 1) > 0) {
        if(buf[0] == '\r')continue;
        buffer[count++] = buf[0];
        if(buf[0] == '\n')break;
    }
    return count;
}

int all_clear(host_t** hosts, int len) {
    int c;
    for(c=0; c<len; c++) {
        if(hosts[c] != NULL) {
            return -1;
        }
    }
    return 1;
}

int main(int argc, const char *argv[])
{
    printf("Content-Type: text/html\n\n");
    fflush(stdout);
    setenv("QUERY_STRING", "h1=127.0.0.1&p1=2065&f1=t1.txt", 1);
    char* query = getenv("QUERY_STRING");

    int c;
    int attr_count;
    char** attrs;
    str_split(query, "&", &attrs, &attr_count);

    host_t* hosts[6];
    for(c=0; c<6; c++) {
        hosts[c] = NULL;
    }


    for(c=0; c<attr_count; c++) {
        if(strlen(attrs[c]) > 3) {
            int num_of_host = attrs[c][1] - '0';
            if(attrs[c][0] == 'h') {
                char hostname[17];
                bzero(hostname, 17);
                strcpy(hostname, &attrs[c][3]);
                create_host(&hosts[num_of_host], hostname, 0, "");
            } else if(attrs[c][0] == 'p') {
                hosts[num_of_host]->port = atoi(&attrs[c][3]);
            } else {
                strcpy(hosts[num_of_host]->filename, &attrs[c][3]);
                hosts[num_of_host]->host_file = fopen(hosts[num_of_host]->filename, "r");
            }
        }
    }

   // print table header
    //
    printf("<html>\n<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />\n<title>Network Programming Homework 3</title>\n</head>\n<body bgcolor=#336699>\n<font face=\"Courier New\" size=2 color=#FFFF99>\n<table width=\"800\" border=\"1\">\n<tr>\n");
    fflush(stdout);
    for(c=1; c<=5; c++) {
        if(hosts[c] != NULL) {
            printf("<td>%s</td>\n", hosts[c]->hostname);
            fflush(stdout);
        }
    }
    printf("</tr>\n<tr>\n");
    fflush(stdout);
    for(c=1; c<=5; c++) {
        if(hosts[c] != NULL) {
            printf("<td valign=\"top\" id=\"m%d\"></td>\n", c-1);
            fflush(stdout);
        }
    }
    printf("</tr>\n</table>\n");
    fflush(stdout);

    fd_set rfds, afds;
    int nfds;
    nfds = getdtablesize
    FD_ZERO(&afds);
    int exit_flags[6];

    for(c=1; c<=5; c++){
        exit_flags[c] = 1;
        if(hosts[c] != NULL) {

            struct sockaddr_in server;
            exit_flags[c] = 0;
            int sock;

            sock = socket(AF_INET , SOCK_STREAM , 0);
            if(sock == -1) {
                printf("Create sock error\n");
                fflush(stdout);
                return 500;
            }
            memset(&server, 0, sizeof(server));
            server.sin_addr.s_addr = inet_addr(hosts[c]->hostname);
            server.sin_family = AF_INET;
            server.sin_port = htons(hosts[c]->port);
            int flag = fcntl(sock, F_GETFL, 0);
            fcntl(sock, F_SETFL, flag | O_NONBLOCK);

            if(connect(sock, (struct sockaddr *)&server , sizeof(server)) < 0) {
                if(errno != EINPROGRESS) {
                    printf("Errno : %d\n", errno);
                    printf("Connect error\n");
                    fflush(stdout);
                    return 500;
                }
            }

            hosts[c]->server_fd = sock;
            hosts[c]->server = server;
            FD_SET(sock, &afds);
        }
    }

    while(1 == 1) {
        if(all_clear(hosts, 6) == 1) {
            break;
        }
        memcpy(&rfds, &afds, sizeof(rfds));

        if(select(nfds, &rfds, NULL, NULL, NULL) < 0) {
            printf("select error\n");
            fflush(stdout);
            return 500;
        }

        for(c=1; c<=5; c++) {
            if(hosts[c] != NULL && hosts[c]->is_connect == 0) {
                if(connect(hosts[c]->server_fd, (struct sockaddr *)&(hosts[c]->server) , sizeof(hosts[c]->server)) < 0) {
                    printf("Errono %d\n", errno);
                    if(errno == EISCONN) {
                        hosts[c]->is_connect = 1;
                    }
                }
            }
            if(hosts[c] != NULL && FD_ISSET(hosts[c]->server_fd, &rfds)) {
                char buffer[10001];
                bzero(buffer, 10001);
                if(read_line(hosts[c]->server_fd, buffer) > 0) {
                    if(strncmp(buffer, "% ", 2) == 0) {
                        char next_cmd[10001];
                        bzero(next_cmd, 10001);
                        fgets(next_cmd, 10001, hosts[c]->host_file);
                        write(hosts[c]->server_fd, next_cmd, strlen(next_cmd));
                        replace_to_html(next_cmd);
                        printf("<script>document.all['m%d'].innerHTML += \"%% <b>%s</b>\";</script>\n", c-1, next_cmd);
                        fflush(stdout);
                        if(strncmp(next_cmd, "exit", 4) == 0) {
                            exit_flags[c] = 1;
                        }
                    } else {
                        replace_to_html(buffer);
                        printf("<script>document.all['m%d'].innerHTML += \"%s\";</script>\n", c-1, buffer);
                    }
                    fflush(stdout);
                }
            }
            if(hosts[c] != NULL && exit_flags[c] == 1){
                FD_CLR(hosts[c]->server_fd, &afds);
                if(fcntl(hosts[c]->server_fd, F_GETFD) > 0) {
                    close(hosts[c]->server_fd);
                }
                hosts[c] = NULL;
            }
        }
    }

    return 0;
}
