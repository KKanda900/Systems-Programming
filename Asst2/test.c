#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

/*
 * Currently: Working on file handling
 */

typedef struct thread_list
{
    unsigned long int data;
    struct thread_list *next;
} thread_list;

typedef struct thread_arg {
    char* pathname; 
    pthread_mutex_t * mutex_ptr;
} thread_arg;

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
        //pthread_exit(NULL);
        ptr = tmp;
    }

    // then we free the rest
    free(ptr);
}

void *file_handling(void *p)
{
    /* thread_arg * args = (thread_arg *) p;

    pthread_mutex_lock(args->mutex_ptr); */

    int fd = open((char*)p, O_RDONLY);
    printf("File: %s\n", (char*)p);
    
    /* pthread_mutex_unlock(args->mutex_ptr); */
    return NULL;
}

void *directory_handling(void *d_path)
{
    // do something

    DIR *dir = opendir(d_path);
    struct dirent *dp;
    char localname[200000];
    thread_list *front = malloc(sizeof(thread_list));
    //pthread_mutex_t lock;

    if (dir != NULL)
    {
        while ((dp = readdir(dir)) != 0)
        {
            if ((strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0))
                continue;
            strcpy(localname, d_path);
            strcat(localname, "/");
            strcat(localname, dp->d_name); 
            if (dp->d_type == DT_DIR)
            {
                printf("Directory Path: %s\n", localname);
                
                /* directory_handling((void*)localname); */
                
                pthread_t ptid;
                addNode(front, ptid);
                pthread_create(&ptid, NULL, &directory_handling, (void*)localname);
                
            }
            else if (dp->d_type == DT_REG)
            {

                /* file_handling((void*)localname); */

                /* thread_arg t1;
                t1.pathname = localname;
                t1.mutex_ptr = &lock; */

                pthread_t ptid;
                addNode(front, ptid);
                pthread_create(&ptid, NULL, &file_handling, (void*)localname);
                
            }
        }
    }
    else 
    {
        perror(d_path);
        /* printf("Error\n"); */
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

    /* pthread_exit(NULL); */

    return 0;
}