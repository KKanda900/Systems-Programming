#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("usage: ./client [address] [port]");
        return EXIT_FAILURE;
    }

    struct addrinfo hints, *address_list, *addr;
    int error;
    int sock;
    char buffer[1024] = "\n\0";

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo(argv[1], argv[2], &hints, &address_list);
    if (error) {
        fprintf(stderr, "%s", gai_strerror(error));
        exit(EXIT_FAILURE);
    }

    for (addr = address_list; addr != NULL; addr = addr->ai_next) {
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock < 0) continue;
        if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0) break;
        close(sock);
    }

    if (addr == NULL) {
        fprintf(stderr, "Could not connect to %s:%s\n", argv[1], argv[2]);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(address_list); 
    int n;

    for (int i = 0; i < 3; i++) {
        
        bzero(buffer, 1024);

        n = read(sock, buffer, 1024);
        if(n < 0){
            perror("error on read");
        }
        printf("Server: %s\n", buffer);
        
        bzero(buffer, 1024);
        fgets(buffer, 1024, stdin);
        n = write(sock, buffer, strlen(buffer));
        if (n < 0){
            perror("Error on write");
        }
        
    }

    close(sock);
    return EXIT_SUCCESS;
}