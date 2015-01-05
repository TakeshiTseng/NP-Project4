#include <stdio.h>
#include <sys/types.h>  
#include <sys/socket.h>  

#include <netinet/in.h>  

#include <stdlib.h>  
#include <string.h>  
#include <netdb.h>  



unsigned short  portbase = 0;   /* port base, for non-root servers  */  

/*------------------------------------------------------------------------ 
 * passivesock - allocate & bind a server socket using TCP or UDP 
 *------------------------------------------------------------------------ 
 */
int passivesock(const char *service, const char *transport, int qlen)  
    /*
     * Arguments:
     *      service   - service associated with the desired port 
     *      transport - transport protocol to use ("tcp" or "udp") 
     *      qlen      - maximum server request queue length 
     */
{
    struct servent  *pse;   /* pointer to service information entry */  
    struct protoent *ppe;   /* pointer to protocol information entry*/  
    struct sockaddr_in sin; /* an Internet endpoint address     */  
    int s, type;    /* socket descriptor and socket type    */  

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;

    /* Map service name to port number */  
    if ( pse = getservbyname(service, transport) )  
        sin.sin_port = htons(ntohs((unsigned short)pse->s_port)  
                + portbase);  
    else if ((sin.sin_port=htons((unsigned short)atoi(service))) == 0)  
        perror("can't get service entry\n");  

    /* Map protocol name to protocol
     * number */  
    if ( (ppe = getprotobyname(transport)) == 0)  
        perror("can't get protocol entry\n");  

    /* Use protocol to choose a
     * socket type */  
    if (strcmp(transport, "udp") == 0)  
        type = SOCK_DGRAM;  
    else  
        type = SOCK_STREAM;  

    /* Allocate a socket */  
    s = socket(PF_INET, type, ppe->p_proto);  
    if (s < 0) {
        perror("can't create socket\n");  
    }

    /* Bind the
     * socket */
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)  
        perror("can't bind\n");  
    if (type == SOCK_STREAM && listen(s, qlen) < 0)  
        perror("can't listen\n");  
    return s;
}
