/* Simple HTTP server */
#include <sys/types.h>              /* definitions for required types */
#include <stdio.h>                  /* printf(), perror() */
#include <stdlib.h>
#include <unistd.h>                 /* fork() */
#include <string.h>                 /* memset() */
#include <fcntl.h>                  /* manipulate file descriptors */
#include <signal.h>                 /* signal() */
#include <sys/socket.h>             /* socket(), bind(), listen(), accept() */
#include <netinet/in.h>             /* struct sockaddr_in */
#include <arpa/inet.h>

/* For handling CTRL+C signal */
int keep_running = 1;

/* Server handles only these mime types */
struct {
    char *ext;
    char *file_type;
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

/* Handle CTRL+Break signal */
void break_handler(int n) {
    printf("Closing server...");
    keep_running = 0;
}

/* Handles client's request and sends adequate response */
int handle_client(int client_fd) {
    char buffer[2048];                          /* Holds client's request and other stuff */
    char file_buffer[65536];                    /* For reading file's content */
    char *forbidden_message = "HTTP/1.1 403 Forbidden\n\n";
    char *file_type = NULL;                     /* Requested file type */
    size_t request_length, extension_length;
    int f;                                      /* File's descriptor requested by the client */
    ssize_t read_bytes;                         /* Amount of bytes of read message */

    /* Fill memory with a 0 constant byte */
    bzero(buffer, sizeof(buffer));
    /* Read request and save how many bytes were read */
    read_bytes = read(client_fd, buffer, sizeof(buffer));
    if(read_bytes < 0) {
        perror("recv");
        return -1;
    }

    /* Get rid of carriage return and newline from the request */
    int j;
    for(j = 0; j < read_bytes; j++) {
        if(buffer[j] == '\n' || buffer[j] == '\r') {
            buffer[j] = '-';
        }
    }

    /* Set 0 character in place of second space */
    for(j = 4; j < 2048; j++) {
        if(buffer[j] == ' ') {
            buffer[j] = 0;
            break;
        }
    }
    /* Check if it was GET request */
    if(strncmp("GET /", buffer, 5) != 0) {
        printf("This is not GET request.Closing connection...");
        if(close(client_fd) < 0) {
            perror("close");
        }

        return 1;
    }

    /* Check if it was GET request for index.html */
    if(strncmp("GET /\0", buffer, 6) == 0) {
        printf("User requested for index.html file.\n");
        strcpy(buffer, "GET /index.html");
    }

    /* Check if requested file's extension is
     * handled by the server */
    request_length = strlen(buffer);
    int i;
    for(i = 0; mime_types[i].ext != 0; i++) {
        /* Length of one of the server's possible extensions */
        extension_length = strlen(mime_types[i].ext);
        /* Check if the first number of bytes equal to extension_length is correct */
        if(strncmp(mime_types[i].ext, &buffer[request_length - extension_length],
                   extension_length) == 0) {
            file_type = mime_types[i].file_type;
            break;
        }
    }

    /* If file extension is not handled by server close connection
     * and continue accepting new ones*/
    if(file_type == NULL) {
        printf("File extension is not handled by this server.Closing connection...\n");
        if(close(client_fd) < 0) {
            perror("close");
        }

        return -1;
    }

    /* Try to open requested file */
    if((f = open(&buffer[5], O_RDONLY)) < 0) {
        printf("Sending forbidden message...\n");
        write(client_fd, forbidden_message, strlen(forbidden_message));
        if(close(client_fd) < 0) {
            perror("close");
        }

        return 1;
    }

    /* Read everything in 1 go */
    read_bytes = read(f, file_buffer, 65536);
    /* Prepare and send the response */
    sprintf(buffer, "HTTP/1.1 200 OK\nServer:Yeti-Server\nContent-Length:%zd\nConnection:close\nContent-Type: %s\n\n", read_bytes, file_type);
    write(client_fd, buffer, strlen(buffer));
    printf("Sending message...\n");
    /* Send requested file's content */
    write(client_fd, file_buffer, strlen(file_buffer));

    return 1;
}


int main(int argc, char **argv) {

    if(argc > 1) {
        printf("Usage:\t./simple_http_server\nQuitting...\n");
        return -1;
    }

    /* CTRL+Break */
    struct sigaction act;
    act.sa_handler = break_handler;
    sigaction(SIGINT, &act, NULL);

    int server_fd, client_fd;

    /* Server's and client's addresses */
    struct sockaddr_in server_address, client_address;
    /* Fill memory with a constant 0 byte */
    memset(&server_address, 0, sizeof(server_address));

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

    /* Listen for as many connections as possible */
    if(listen(server_fd, SOMAXCONN) < 0) {
        perror("listen");
        return -1;
    }

    socklen_t client_address_length = sizeof(client_address);
    pid_t pid;
    while(keep_running) {
        /* Accept new connection */
        client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_length);
        if(client_fd < 0) {
            perror("accept");
            return -1;
        }

        /* Child process handles client when server
         * continues accepting new connections */
        pid = fork();
        if(pid < 0) {
            perror("fork");
            return -1;
        }
        /* Child process */
        if(pid == 0) {
            close(server_fd);
            handle_client(client_fd);
            break;
        /* Parent process */
        } else {
            close(client_fd);
        }
    }

    /* Close server socket fd */
    if(close(server_fd) < 0) {
        perror("close");
        return -1;
    }


    return 0;
}