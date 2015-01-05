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

typedef unsigned char byte;

struct header {
	byte vn[1];
	byte cd[1];
	byte dst_port[2];
	byte dst_ip[4];
};

typedef struct header header_t;

int main(int argc, const char *argv[])
{
	
    char port_str[5] = "2007";
	int sc_fd = passivesock(port_str, "tcp", 5);
	struct sockaddr_in client_addr;
	int addrlen = sizeof(client_addr);
	printf("accepting.....\n");

	while(1 == 1){
		int client_sock = accept(sc_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);
		header_t hd;
		read(client_sock, &hd, sizeof(header_t));
		printf("vn %d\n", hd.vn[0]);
		printf("cd %d\n", hd.cd[0]);
		int dst_port = (int)hd.dst_port[0] + (int)hd.dst_port[1];
		printf("dst_port %d\n", dst_port);
		int dst_ip = (int)hd.dst_ip[0] + (int)hd.dst_ip[1] + (int)hd.dst_ip[2] + (int)hd.dst_ip[3];
		printf("dst_ip %d\n", dst_ip);

		byte buff[1];
		while(read(client_sock, buff, sizeof(byte)) != 0) {
			printf("data: %d\n", buff);
		}

		close(client_sock);
	}
	
	return 0;
}
