#include <sys/types.h>      /* useful definitions */
#include <stdio.h>          /* printf(), perror() */
#include <sys/socket.h>     /* socket(), bind(), listen(), accept() */
#include <netinet/in.h>     /* struct sockaddr_in */
#include <arpa/inet.h>      /* htons */
#include <string.h>         /* memset() */
#include <time.h>           /* struct tm */

int main(int argc, char **argv) {

    if(argc > 1) {
        perror("Usage:\t ./server.Quitting...\n");
        return -1;
    }

    int server_fd;                          /* Server socket descriptor */
    struct sockaddr_in server_address;      /* Server's address */
    struct sockaddr_in client_address;      /* Client's address */
    char buf[8192];                         /* Buffer for receiving messages */
    char *err_msg = "blad zapytania";

    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char *current_time = asctime(timeinfo);
    printf("%s", current_time);

    /* Fill memory with a constant 0 byte */
    memset(&server_address, 0, sizeof(server_address));

    /* IPv4, port: 8976, accept any address */
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8976);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Datagram socket */
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_fd < 0) {
        perror("socket");
        return -1;
    }

    /* Bind socket to a given server's address */
    if(bind(server_fd, (const struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("bind");
        return -1;
    }

    socklen_t client_addr_length;   /* Client's address length */
    int success;                    /* Success on sendto() */
    /* Server loop */
    while(1) {
        client_addr_length = sizeof(client_address);
        /* Receive message into the buf */
        if(recvfrom(server_fd, buf, sizeof(buf), 0, (struct sockaddr *)&client_address,
                    (socklen_t *)&client_addr_length) < 0) {
            perror("recvfrom");
            return -1;
        }

        /* If client sent "czas" message */
        if(strcmp("czas\n", buf) == 0) {
            success = sendto(server_fd, current_time, strlen(current_time), 0,
                   (const struct sockaddr *)&client_address, client_addr_length);
            if(success < 0) {
                perror("sendto");
                return -1;
            }
        } else {
            success = sendto(server_fd, err_msg, strlen(err_msg), 0,
                   (const struct sockaddr *)&client_address, client_addr_length);
            if(success < 0) {
                perror("sendto");
                return -1;
            }
        }

        printf("[Client] --> %s", buf);

        /* Clear the buffer */
        memset(buf, 0, sizeof(buf));
    }

}