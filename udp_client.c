#include <sys/types.h>      /* useful definitions */
#include <stdio.h>          /* printf(), perror() */
#include <sys/socket.h>     /* socket(), bind(), listen(), accept() */
#include <netinet/in.h>     /* struct sockaddr_in */
#include <arpa/inet.h>      /* htons */
#include <string.h>         /* memset() */


int main(int argc, char **argv) {

    int client_fd;
    char buf[8192];
    struct sockaddr_in server_address, client_address;

    /* IPv4, port:8977 */
    memset(&client_address, 0, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(8977);
    client_address.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Fill memory with constant 0 byte */
    memset(&server_address, 0, sizeof(server_address));

    /* Server's address */
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8976);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Create UDP IPv4 socket */
    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_fd < 0) {
        perror("socket");
        return -1;
    }

    /* Bind client socket */
    if(bind(client_fd, (const struct sockaddr *)&client_address, sizeof(client_address)) < 0) {
        perror("bind");
        return -1;
    }

    int success;
    while(1) {
        /* Clear buffer */
        memset(buf, 0, sizeof(buf));

        printf("[YOU] --> ");
        /* Take input from the user */
        fgets(buf, sizeof(buf), stdin);
        /* Send message to the server */
        success = sendto(client_fd, buf, sizeof(buf), 0, (const struct sockaddr *) &server_address, sizeof(server_address));
        if(success < 0) {
            perror("sendto");
            return -1;
        }
        /* Receive message */
        if (recvfrom(client_fd, buf, sizeof(buf), 0, (struct sockaddr *) &client_address,
                     (socklen_t *)&client_address) < 0) {
            perror("recvfrom");
            return -1;
        }

        printf("[Server] --> %s", buf);

    }


    return 0;
}