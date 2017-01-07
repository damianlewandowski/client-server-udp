#include <sys/types.h>              /* definitions for required types */
#include <stdio.h>                  /* printf(), perror() */
#include <stdlib.h>
#include <unistd.h>                 /* fork() */
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>             /* socket() */
#include <netinet/in.h>
#include <arpa/inet.h>


/* Server handles only these mime types */
struct {
    char *mime_type;
    char *ext;
} mime_types[] = {
    {"gif", "image/gif" },
    {"jpg", "image/jpeg"},
    {"jpeg","image/jpeg"},
    {"png", "image/png" },
    {"zip", "image/zip" },
    {"gz",  "image/gz"  },
    {"tar", "image/tar" },
    {"htm", "text/html" },
    {"html","text/html" },
    {0, 0}
};


int main(int argc, char **argv) {

    if(argc > 1) {
        printf("Usage:\t./simple_http_server\nQuitting...\n");
        return -1;
    }

    char buffer[2048];
    int server_fd, client_fd;
    /* Server's and client's address */
    struct sockaddr_in server_address, client_address;
    /* Fill memory with a constant 0 byte */
    memset(&server_address, 0, sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));

    /* IPv4 address, port: 9080, all incoming addresses */
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9080);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    /* TCP socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) {
        perror("socket");
        return -1;
    }

    /* Bind socket to a given address and port */
    if(bind(server_fd, (const struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("bind");
        return -1;
    }


    if(listen(server_fd, SOMAXCONN) < 0) {
        perror("listen");
        return -1;
    }

    socklen_t client_address_length = sizeof(client_address);
    while(1) {
        /* Accept new connection */
        client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_length);
        if(client_fd < 0) {
            perror("accept");
            return -1;
        }
        /* Receive request */
        if(recv(server_fd, buffer, sizeof(buffer), 0) < 0) {
            perror("recv");
            return -1;
        }

        printf("Got new connection.\nHe wrote: %s\n", buffer);
    }

    return 0;
}