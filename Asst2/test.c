#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * Currently: Working on file handling
 */

#define DEBUG printf("hello\n")

typedef struct thread_list
{
    unsigned long int data;
    pthread_mutex_t mtx;
    struct thread_list *next;
} thread_list;

thread_list *addNode(thread_list *front, unsigned long data)
{
    thread_list *newNode = malloc(sizeof(thread_list)); //malloc space for the newNode that you are creating
    newNode->data = data;
    newNode->next = NULL;

    // if front is null, which is most likely when your first adding to front then
    // make newNode the front
    if (front == NULL)
    {
        front = newNode;
        //free(newNode);
        return front;
    }

    // Otherwise front is not null and you can keep adding to the front node
    (*newNode).next = front->next;
    front->next = newNode;

    // In the end return front
    return front;
}

void free_list(thread_list *front)
{
    // Like for other linked list functions, check if the front is null or not
    if (front == NULL)
    {
        return;
    }

    //Set a pointer so we can iterate nicely
    thread_list *ptr = front;
    while (ptr->next != NULL)
    {
        // This would be like any traversal but we are freeing each memory of the pointers
        thread_list *tmp = ptr->next;
        pthread_join(front->data, NULL);
        free(ptr); // like here
        ptr = tmp;
    }

    // then we free the rest
    free(ptr);
}

void *file_handling(void *f_path)
{
    /* int fd = open((char *)f_path, O_RDONLY);

     if (DEBUG == 0)
        printf("file handler\n"); 

    if (fd < 0)
    {
        //printf("file handler\n");
        perror(f_path);
        exit(1);
    }

    printf("filepath: %s\n", (char *)f_path); */

    FILE *fp = fopen((char *)f_path, "r");

    if (fp == NULL)
    {
        exit(1);
    }

    printf("filepath: %s\n", (char *)f_path);

    fclose(fp);

    return NULL;
}

void *directory_handling(void *d_path)
{
    // do something

    DIR *dir = opendir(d_path);
    struct dirent *dp;
    struct stat buff;
    thread_list *front = malloc(sizeof(thread_list));
    /* void *handler; */

    if (dir != NULL)
    {
        while ((dp = readdir(dir)) != NULL)
        {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
                continue;
            char localname[20000];
            strcpy(localname, d_path);
            strcat(localname, "/");
            strcat(localname, dp->d_name);
            if (stat(localname, &buff) == 0)
            {
                if (S_ISDIR(buff.st_mode) > 0)
                {
                    //pthread_t ptid;
                    //addNode(front, ptid);
                    //pthread_create(&front->data, NULL, &directory_handling, (void *)localname);
                    printf("Directory name: %s\n", localname);
                    directory_handling((void*)localname);
                }
                if (S_ISREG(buff.st_mode) > 0)
                {
                    //pthread_t ptid;
                    //addNode(front, ptid);
                    //pthread_create(&front->data, NULL, &file_handling, (void *)localname);
                    file_handling((void*)localname);
                }
                else
                {
                    continue;
                }
            }
        }
    }
    else if (dir == NULL)
    {
        DEBUG;
        perror(d_path);
        exit(1);
    }

    closedir(dir);

    free_list(front);

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Expecting two arguments\n");
        return -1;
    }

    directory_handling((void *)argv[1]);

    pthread_exit(NULL);

    return 0;
}
