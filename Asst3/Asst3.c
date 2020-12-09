#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#define BACKLOG 5

/*
 * This struct type is used to hold the attributes for a client.
 * 
 * It contains three fields:
 *  1. The address of the client.
 *  2. The address length of the client's address.
 *  3. The file descriptor of the client.
 */
struct connection
{
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int fd;
};

/*
 * kkjserver function is where the server will start and accept incoming clients then start the joke process. It takes a char pointer 
 * to a port number that where the server will start from. It is an int function that returns an error if there is an error that happens
 * upon start but not in the infinite loop. 
 * 
 * In the infinite loop the server will constantly run and accept clients one at a time. Upon a client connecting it first sends the inital
 * knock knock to initiate the joke. Then reads for the next expected response (who's there?) and sends the next question (setup.). After it writes the setup, 
 * the server waits for the next expected response (setup, who?) then sends the punchline. After the client recieves the punch line, the server is expecting 
 * a message of annoyance, disgust, or surprise the server closes connection with the current client its working with and starts the next client in queue.
 * 
 * When the server reads for the expected response and the expected response turns out incorrect, the server will return an error message
 * based on the KKJ protocal.  
 */
int kkjserver(char *portnum)
{
    struct addrinfo hint, *address_list, *addr;
    struct connection *con;
    int error, sfd;

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

        if ((bind(sfd, addr->ai_addr, addr->ai_addrlen) == 0) &&
            (listen(sfd, BACKLOG) == 0))
        {
            break;
        }

        close(sfd);
    }

    if (addr == NULL)
    {
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo(address_list);

    char message1[1024];
    char buf[1024];
    char client[1024]; int pipe;

    printf("Waiting for connection\n");
    for (;;)
    {
        con = malloc(sizeof(struct connection));
        con->addr_len = sizeof(struct sockaddr_storage);
        con->fd = accept(sfd, (struct sockaddr *)&con->addr, &con->addr_len);

        if (con->fd == -1)
        {
            perror("accept");
            continue;
        }

        char host[100], port[10];
        int error, nread;

        error = getnameinfo((struct sockaddr *)&con->addr, con->addr_len, host, 100, port, 10, NI_NUMERICSERV);

        if (error != 0)
        {
            fprintf(stderr, "getnameinfo: %s", gai_strerror(error));
            close(con->fd);
            return -1;
        }

        printf("[%s:%s] connection\n", host, port);

        // This is the read that should first happen
        // sends the client the start of the knock knock
        int i = 0;
        while (i != 1)
        {
            strcpy(message1, "REG|13|Knock, knock.|");
            strcat(message1, "\0");
            write(con->fd, message1, strlen(message1));
            i++;
        }

        // first read, should read the Who's there?
        pipe = 0;
        bzero(buf, 1024);
        bzero(client, 1024);
        while ((nread = read(con->fd, buf, 1)) > 0)
        {
            if (client == NULL)
            {
                strcpy(client, buf);
            }
            else
            {
                strcat(client, buf);
            }

            if (strcmp(buf, "|") == 0)
            {
                pipe++;
                if (pipe == 3)
                {
                    printf("Client asked: %s\n", client);
                    break;
                }
            }
        }

        write(con->fd, "REG|7|orange.|", strlen("REG|7|orange.|"));

        bzero(buf, 0);
        bzero(client, 0);


        // second iteration should read "orange who?"
        pipe = 0;
        bzero(buf, 1024);
        bzero(client, 1024);
        while ((nread = read(con->fd, buf, 1)) > 0)
        {
            if (client == NULL)
            {
                strcpy(client, buf);
            }
            else
            {
                strcat(client, buf);
            }

            if (strcmp(buf, "|") == 0)
            {
                pipe++;
                if (pipe == 3)
                {
                    printf("Client asked: %s\n", client);
                    break;
                }
            }
        }

        write(con->fd, "REG|36|orange you glad I didn't say banana.|", strlen("REG|36|orange you glad I didn't say banana.|"));
        bzero(buf, 0);
        bzero(client, 0);


        // third iteration should read the ending a/d/#
        pipe = 0;
        bzero(buf, 1024);
        bzero(client, 1024);
        read(con->fd, client, 1024); // assuming the message is any a/d/s

        printf("Connection Closed\n");
        close(con->fd);
        free(con);


    }

    // never reach here
    return 0;
}

/*
 * The main function will first check if the proper usage was given to start the server. The server should start with the usage:
 *                              ./KKJserver [port]
 * Once given the proper usage it will start the KKJserver and should not end once its started. The only reason it should end
 * is if on the server end, the KKJserver was ended with Ctrl + C or Ctrl + Z.
 */
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

