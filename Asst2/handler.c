#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>

#define DEBUG 2

int ids = 0;
int j = 0;

char * temp;

typedef struct directory_names
{
    char data[200000];
    struct directory_names *next;
} directory_names;

typedef struct thread_args{
    char directory[200000];
    directory_names * directories;
    //add mutex pointer here
    pthread_t id[100000];
} thread_args;

typedef struct directory_copy
{
    char data[200000];
    struct args_struct *next;
} directory_copy;

directory_names *addNode(directory_names *front, char *data)
{
    directory_names *newNode = malloc(sizeof(directory_names));
    strcpy(front->data, data);
    newNode->next = NULL;

    if (front == NULL)
    { 
        front = newNode;
        free(newNode);
        return front;
    }

    (*newNode).next = front->next;
    front->next = newNode;

    free(newNode);
    return front;
}

void free_list(directory_names *front)
{
	directory_names*tmp;
    while (front != NULL) 
	{
		tmp = front;
        front = front->next;
		free(tmp); 
	}
}

void *file_handler(void *file)
{
    printf("File: %s\n", (char*)file);
    return NULL;
}

void *directory_handler(void *direct)
{
    thread_args* args = (thread_args*)direct;

    DIR *dir = opendir(args->directory);
    struct dirent *dp;

    if (dir == NULL)
    {
        perror(args->directory);
        exit(1);
    }
    else
    {
        while ((dp = readdir(dir)) != 0)
        {
            if ((strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0))
            {
                continue;
            }
            temp = malloc(strlen(args->directory) + strlen(dp->d_name) + 2);
            strcpy(temp, args->directory);
            strcat(temp, "/");
            strcat(temp, dp->d_name);


            if (dp->d_type == DT_DIR)
            {
                addNode(args->directories, temp);
                strcpy(args->directory, temp);
                printf("Node Directory added: %s\n", temp);




                pthread_create(&args->id[ids++], NULL, &directory_handler, (void *)args);
            }
            else if (dp->d_type == DT_REG)
            {
                pthread_create(&args->id[ids++], NULL, &file_handler, (void *)args->directories->data);
            }
        }

        while (j < ids)
        {
            pthread_join(args->id[j++], NULL);
        }

        closedir(dir);

        

        pthread_exit(NULL);

        
    }

    return NULL;
}

/*
 * Point to remember:
 * Directory can be either 
 */

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Expected two arguments\n");
        return -1;
    }


    thread_args* args = malloc(sizeof(thread_args));
    strcpy(args->directory, argv[1]);
    args->directories = malloc(sizeof(directory_names));
    addNode(args->directories, argv[1]);

    directory_handler((void *)args);

    free_list(args->directories);
    

    return 0;
}