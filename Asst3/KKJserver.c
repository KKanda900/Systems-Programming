#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <fcntl.h>

#define BACKLOG 5

struct connection
{
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int fd;
};

int kkjserver(char *port);
void *echo(struct connection *arg);

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Please provide a port number: %s [port]\n", argv[0]);
        exit(1);
    }

    (void)kkjserver(argv[1]);
    return 0;
}

int kkjserver(char *portnum)
{
    struct addrinfo hint, *address_list, *addr;
    struct connection *con;
    int error, sfd;
    pthread_t tid;

    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    error = getaddrinfo(NULL, portnum, &hint, &address_list);
    if (error != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    for (addr = address_list; addr != NULL; addr = addr->ai_next)
    {
        sfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        if (sfd == -1)
        {
            continue;
        }

        if ((bind(sfd, addr->ai_addr, addr->ai_addrlen) == 0) && (listen(sfd, BACKLOG) == 0))
        {
            break;
        }

        close(sfd);
    }

    if (addr == NULL)
    {
        printf("Couldn't bind to port\n");
        return -1;
    }

    freeaddrinfo(address_list);

    printf("Waiting for connection\n");

    con = malloc(sizeof(struct connection));
    con->addr_len = sizeof(struct sockaddr_storage);

    con->fd = accept(sfd, (struct sockaddr *)&con->addr, &con->addr_len);

    if (con->fd == -1)
    {
        perror("accept");
    }

    /* fcntl(sfd, F_SETFL, O_NONBLOCK); */
    int n;

    printf("Connected\n");
    char server_message[20000];
    strcpy(server_message, "Knock knock");
    strcat(server_message, "\0");
    n = write(con->fd, server_message, strlen(server_message));
    if (n < 0)
    {
        perror("Write error");
    }

    /* if ((send(con->fd, server_message, sizeof(server_message), 0)) == -1)
        {
            fprintf(stderr, "Failed to send message\n");
            close(con->fd);
            break;
        } */

    echo(con);

    close(sfd);

    return 0;
}

void *echo(struct connection *arg)
{
    char host[100], port[10];
    struct connection *c = arg;
    int error, nread;

    error = getnameinfo((struct sockaddr *)&c->addr, c->addr_len, host, 100, port, 10, NI_NUMERICSERV);

    if (error != 0)
    {
        fprintf(stderr, "getnameinfo: %s", gai_strerror(error));
        close(c->fd);
        return NULL;
    }

    char client_message[1024] = "\n\0";
    int n;

    while (1)
    {

        //bzero(client_message, 1024);

        nread = read(c->fd, client_message, 1024);
        if (nread < 0)
        {
            perror("Error on read");
        }

        printf("Client message: %s\n", client_message);

        bzero(client_message, 1024);

        /* n = 0;
        while((client_message[n++] = getchar()) != '\0'); */

        // This gets the server response but we dont want the server response fgets(client_message, 1024, stdin);

        n = 0;

        if (strncmp(client_message, "whos there?", 12) != 0)
        {
            strncpy(client_message, "orange", 7);
            n = write(c->fd, client_message, strlen(client_message));
            if (n < 0)
            {
                perror("Write error");
            }
        }

        if (strncmp(client_message, "orange who?", 12) != 0)
        {
            strncpy(client_message, "orange you glad I didn't say banana", 36);
            n = write(c->fd, client_message, strlen(client_message));
            if (n < 0)
            {
                perror("Write error");
            }
        }

        /* strncpy(client_message, "message failed", 15);
        n = write(c->fd, client_message, strlen(client_message));
        if (n < 0)
        {
            perror("Write error");
        } */

        printf("Server sending: %s\n", client_message);
    }

    close(c->fd);
    free(c);
    return NULL;
}