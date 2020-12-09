#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("usage: ./client [address] [port]");
        return EXIT_FAILURE;
    }

    struct addrinfo hints, *address_list, *addr;
    int error;
    int sock;
    char buffer[1024];
    char client[1024];

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo(argv[1], argv[2], &hints, &address_list);
    if (error)
    {
        fprintf(stderr, "%s", gai_strerror(error));
        exit(EXIT_FAILURE);
    }

    for (addr = address_list; addr != NULL; addr = addr->ai_next)
    {
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock < 0)
            continue;
        if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0)
            break;
        close(sock);
    }

    if (addr == NULL)
    {
        fprintf(stderr, "Could not connect to %s:%s\n", argv[1], argv[2]);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(address_list);
    int n;
    int i = 0;

    for (i = 0; i < 3; ++i)
    {

        int pipe = 0;

        bzero(buffer, 1024);
        bzero(client, 1024);
        while ((n = read(sock, buffer, 1)) > 0)
        {
            strcat(client, buffer);

            if (strcmp(buffer, "|") == 0)
            {
                pipe++;
            }

            if (pipe == 3)
            {
                break;
            }
        }

        printf("Server said: %s\n", client);

        if (strcmp(client, "REG|13|Knock, knock.|") == 0)
        {
            write(sock, "REG|12|Who's there?|", strlen("REG|12|Who's there?|"));
        }
        else if (strcmp(client, "REG|7|orange.|") == 0)
        {
            write(sock, "REG|12|orange, who?|", strlen("REG|12|orange, who?|"));
        }
        else if (strcmp(client, "REG|36|orange you glad I didn't say banana.|") == 0)
        {
            write(sock, "woah that was stupid.", strlen("woah that was stupid."));
        }

        
        bzero(client, 0);
        bzero(buffer, 0);
    }

    close(sock);
    return EXIT_SUCCESS;
}