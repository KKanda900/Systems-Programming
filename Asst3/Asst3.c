#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>

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
 * writeError sends out error message to the client, and close the connection.
 *
 * The function takes two arguments: the struct connection that set in kkjserver(), and the error message.
 */
void writeError(struct connection* con, char* errorMsg)
{
    write(con->fd, errorMsg, strlen(errorMsg));
    printf("Connection Closed\n");
    close(con->fd);
    free(con);
}

/*
 * readMessage reads and checks if the message is valid.
 *
 * This function takes two arguments: the struct connection that set in kkjserver(), and the message#.
 * The function sends out the corresponding error code to the client and returns 0 if the message is not valid,
 * 1 if it's the expected message in expected format.
 */
int readMessage(struct connection* con, int messageNum)
{
    int pipeCount = 0;
    int pipeNum =3;
    int msglen = 0;

    char buff[1] = {'\0'};
    char client[1024];
    bzero(client, 1024);

    char errorMessage[16];
    bzero(errorMessage, 16);
    strcpy(errorMessage, "ERR|M");
    char msgnum = (char) (messageNum + 48);
    strncat(errorMessage, &msgnum, 1);

    int readlen = 0;
    while (pipeCount < pipeNum)
    {
        if (read(con->fd, buff, 1) <= 0)
        {
            printf("Error on read\n");
            return 0;
        }
        strncat(client, buff, 1);
        readlen++;
        if (pipeCount == 0 && pipeNum != 2 && client[0] == 'E') pipeNum = 2;
        if (buff[0] == '|')
        {
            pipeCount++;
            if (pipeCount == 1) // check first part of the message
            {
                if ((pipeNum == 2 && strcmp(client, "ERR|") != 0) ||
                    (pipeNum == 3 && strcmp(client, "REG|") != 0))
                {
                    strcat(errorMessage, "FT|");
                    writeError(con, errorMessage);
                    return 0;
                }
                bzero(client, readlen);
                readlen = 0;
            }
            if (pipeCount == 2) // try read length of message
            {
                if (pipeNum == 2) break;

                for (int i = 0; i < strlen(client); i++)
                {
                    if (strlen(client) == 1 || (!isdigit(client[i]) && client[i] != '|'))
                    {
                        strcat(errorMessage, "FT|");
                        writeError(con, errorMessage);
                        return 0;
                    }
                }
                msglen = (int) strtol(client, NULL, 10);
                bzero(client, readlen);
                readlen = 0;
            }
            if (pipeCount == 3) // check if message length match
            {
                if (readlen - 1 != msglen)
                {
                    strcat(errorMessage, "LN|");
                    writeError(con, errorMessage);
                    return 0;
                }
            }
        }
    }

    if (pipeNum == 2) // error msg handling
    {
        if (strlen(client) != 5 || client[0] != 'M' || (client[1] < 48 || client[1] > 53) ||
            (strncmp(client+2, "CT|", 3) != 0 && strncmp(client+2, "LN|", 3) != 0 && strncmp(client+2, "FT|", 3) != 0))
            printf("Unrecognized error message received: ERR|%s\n", client);
        else printf("Error message received: ERR|%s\n", client);
        printf("Connection Closed\n");
        close(con->fd);
        free(con);
        return 0;
    }
    if ((messageNum == 1 && strcmp(client, "Who's there?|") != 0) ||
        (messageNum == 3 && strcmp(client, "Orange, who?|") != 0) ||
        (messageNum == 5 && !ispunct(client[readlen-2])))
    {
        strcat(errorMessage, "CT|");
        writeError(con, errorMessage);
        return 0;
    }

    return 1;
}

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
void kkjserver(char *portnum)
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
        return;
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
        freeaddrinfo(address_list);
        return;
    }

    freeaddrinfo(address_list);

    char message[1024];
    printf("Waiting for connection\n");

    while (1)
    {
        con = malloc(sizeof(struct connection));
        con->addr_len = sizeof(struct sockaddr_storage);
        con->fd = accept(sfd, (struct sockaddr *)&con->addr, &con->addr_len);

        if (con->fd == -1)
        {
            perror("accept error");
            free(con);
            continue;
        }

        char host[100], port[10];

        error = getnameinfo((struct sockaddr *)&con->addr, con->addr_len, host, 100, port, 10, NI_NUMERICSERV);

        if (error != 0)
        {
            fprintf(stderr, "getnameinfo: %s", gai_strerror(error));
            close(con->fd);
            free(con);
            return;
        }

        printf("[%s:%s] connection\n", host, port);

        // This is the write that should first happen
        // sends the client the start of the knock knock
        strcpy(message, "REG|13|Knock, knock.|\0");
        if (write(con->fd, message, strlen(message)) <= 0)
        {
            printf("Connection Closed\n");
            close(con->fd);
            free(con);
            continue;
        }

        // first read, should read the Who's there?
        if (readMessage(con, 1) == 0) continue;

        // second write
        strcpy(message, "REG|7|Orange.|\0");
        if (write(con->fd, message, strlen(message)) <= 0)
        {
            printf("Connection Closed\n");
            close(con->fd);
            free(con);
            continue;
        }

        // second iteration should read "orange who?"
        if (readMessage(con, 3) == 0) continue;

        // third write
        strcpy(message, "REG|36|Orange you glad I didn't say banana.|\0");
        if (write(con->fd, message, strlen(message)) <= 0)
        {
            printf("Connection Closed\n");
            close(con->fd);
            free(con);
            continue;
        }

        // third iteration should read the ending a/d/s
        if (readMessage(con, 5) == 0) continue;

        printf("Connection Closed\n");
        close(con->fd);
        free(con);
    }
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

    kkjserver(argv[1]);
    return 0;
}