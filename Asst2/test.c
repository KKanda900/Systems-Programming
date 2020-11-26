#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

/*
 * Currently: Working on file handling
 */

typedef struct thread_list
{
    unsigned long int data;
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
        free(ptr); // like here
        ptr = tmp;
    }

    // then we free the rest
    free(ptr);
}

void *directory_handling(void *d_path)
{
    // do something

    DIR *dir = opendir(d_path);
    struct dirent *dp;
    char localname[200000];
    thread_list *front = malloc(sizeof(thread_list));
    thread_list *front2 = malloc(sizeof(thread_list));
    void *handler;

    if (dir != NULL)
    {
        while ((dp = readdir(dir)) != 0)
        {
            if ((strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0))
                continue;
            if (dp->d_type == DT_DIR)
            {
                strcpy(localname, d_path);
                strcat(localname, "/");
                strcat(localname, dp->d_name);
                printf("%s\n", localname);
                pthread_t ptid;
                addNode(front, ptid);
                pthread_create(&ptid, NULL, &directory_handling, (void *)localname);
            }
            else if (dp->d_type == DT_REG)
            {
                strcpy(localname, d_path);
                strcat(localname, "/");
                strcat(localname, dp->d_name);
                printf("%s\n", localname);
                pthread_t ptid;
                addNode(front2, ptid);
                pthread_create(&ptid, NULL, &file_handling, (void *)localname);
            }
        }
    }
    else if (dir == NULL)
    {
        perror(d_path);
        exit(1);
    }

    closedir(dir);

    while (front != NULL)
    {
        pthread_join(front->data, &handler);
        front = front->next;
    }

    while (front2 != NULL)
    {
        pthread_join(front2->data, &handler);
        front2 = front2->next;
    }

    free_list(front);
    free_list(front2);

    return NULL;
}

void *file_handling(void *f_path)
{
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Expecting two arguments");
        return -1;
    }

    directory_handling((void *)argv[1]);

    return 0;
}