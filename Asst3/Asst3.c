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
 * verifyMessage checks if the respond message is valid.
 *
 * The function takes two arguments: the client input and the message# (0-5).
 * The function returns NULL if the message is a valid REG, returns ERR if it is valid ERR, returns ERROR if it is an
 * incorrect error message, returns the corresponding error code if it is an incorrect regular message.
 */
char* verifyMessage (char* clientInput, int messageNum)
{
    // getting a substring
    if (strlen(clientInput) == 0)
    {
        perror("Checking empty string");
        return NULL;
    }

    char* errorCode = (char*) malloc(8);
    bzero(errorCode, 8);
    errorCode[0] = 'M';
    errorCode[1] = (char) (messageNum + 48);
    errorCode[2] = 'F';
    errorCode[3] = 'T';

    int inputlen = 0;
    int sep;
    if (clientInput[0] == 'E') sep = 2;
    else if (clientInput[0] == 'R') sep = 3;
    else return errorCode;

    int sepnum = 0;
    for (int i = 0; i < strlen(clientInput) && sepnum < sep; i++)
    {
        if (clientInput[i] == '|')
        {
            sepnum++;
            inputlen = i + 1;
        }
    }

    // move the part of string from clientInput to input
    char input[1024];
    bzero(input, 1024);
    strncpy(input, clientInput, inputlen);
    char temp[1024];
    strcpy(temp, clientInput);
    strncpy(clientInput, temp+inputlen, 1024);

    if (strncmp(input, "ERR|", 4) == 0) // receiving error code
    {
        bzero(errorCode, 8);
        if (inputlen != 9 || input[4] != 'M' || input[5] < 48 || input[5] > 53 ||
                strncmp(input+6, "CT|", 3) != 0 || strncmp(input+6, "LN|", 3) != 0 || strncmp(input+6, "FT|", 3) != 0)
            strcpy(errorCode, "ERROR\0");
        else
            strcpy(errorCode, "ERR\0");

        return errorCode;
    }

    if (strncmp(input, "REG|", 4) != 0) return errorCode;

    // check format of respond
    int part = 1;
    int msglen = 0;
    char* msg;
    for (int i = 4; i < inputlen; i++)
    {
        if (input[i] == '|')
        {
            part++;
            if (part == 2)
            {
                msglen = i;
                msg = input + i + 1;
            }
            if (part == 3)
            {
                msglen = i - msglen - 1;
                break;
            }
        }
        if (part == 1 && !isdigit(input[i])) return errorCode; // not digit in length field
    }

    if (part < 3 || inputlen < 7) return errorCode; // no enough '|' or missing field

    if (strtol(input+4, NULL, 10) != msglen) // length not correct
    {
        errorCode[2] = 'L';
        errorCode[3] = 'N';
        return errorCode;
    }

    // below is checking for message format
    errorCode[2] = 'C';
    errorCode[3] = 'T';
    if (msglen == 0) return errorCode; // content not correct in any case

    if (messageNum == 1)
    {
        if (strncmp(msg, "Who's there?", 12) != 0) return errorCode;
    }
    else if (messageNum == 3)
    {
        if (strncmp(msg, "Orange, who?", 12) != 0) return errorCode;
    }
    else if (messageNum == 5)
    {
        if (!ispunct(msg[msglen-1])) return errorCode;
    }

    return NULL;
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
    char client[1024];
    char* errorCode;
    char errorMessage[16];

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

        bzero(client, 1024);

        // This is the read that should first happen
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
        if (read(con->fd, client, 1023) <= 0)
        {
            printf("Connection Closed\n");
            close(con->fd);
            free(con);
            continue;
        }
        errorCode = verifyMessage(client, 1);
        if (errorCode != NULL)
        {
            if (strcmp(errorCode, "ERROR") == 0) printf("Unrecognized error message respond received: %s", client);
            else if (strcmp(errorCode, "ERR") == 0) printf("Error message received: %s", client);
            else
            {
                strcpy(errorMessage, "ERR|");
                strcat(errorMessage, errorCode);
                strcat(errorMessage, "|");
                write(con->fd, errorMessage, strlen(errorMessage));
            }
            free(errorCode);
            printf("Connection Closed\n");
            close(con->fd);
            free(con);
            continue;
        }

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
        if (read(con->fd, client, 1023) <= 0)
        {
            printf("Connection Closed\n");
            close(con->fd);
            free(con);
            continue;
        }
        errorCode = verifyMessage(client, 3);
        if (errorCode != NULL)
        {
            if (strcmp(errorCode, "ERROR") == 0) printf("Unrecognized error message respond received: %s", client);
            else if (strcmp(errorCode, "ERR") == 0) printf("Error message received: %s", client);
            else
            {
                strcpy(errorMessage, "ERR|");
                strcat(errorMessage, errorCode);
                strcat(errorMessage, "|");
                write(con->fd, errorMessage, strlen(errorMessage));
                printf("Error msg sent: %s", errorMessage);
            }
            free(errorCode);
            printf("Connection Closed\n");
            close(con->fd);
            free(con);
            continue;
        }

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
        if (read(con->fd, client, 1023) <= 0)
        {
            printf("Connection Closed\n");
            close(con->fd);
            free(con);
            continue;
        }
        errorCode = verifyMessage(client, 5);
        if (errorCode != NULL)
        {
            if (strcmp(errorCode, "ERROR") == 0) printf("Unrecognized error message respond received: %s", client);
            else if (strcmp(errorCode, "ERR") == 0) printf("Error message received: %s", client);
            else
            {
                strcpy(errorMessage, "ERR|");
                strcat(errorMessage, errorCode);
                strcat(errorMessage, "|");
                write(con->fd, errorMessage, strlen(errorMessage));
                printf("Error msg sent: %s", errorMessage);
            }
            free(errorCode);
        }

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