#include "host.h"
#include <stdlib.h>
#include <string.h>

void create_host(host_t** host, char* hostname, int port, char* filename) {
    *host = malloc(sizeof(host_t));
    strcpy((*host)->hostname, hostname);
    (*host)->port = port;
    strcpy((*host)->filename, filename);
    (*host)->is_connect = 0;
}
