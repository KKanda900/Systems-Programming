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

typedef struct directory_names
{
    char data[200000];
    pthread_t id[100000];
    struct directory_names *next;
} directory_names;

typedef struct args_struct
{
    char data[200000];
    pthread_t id[100000];
    struct args_struct *next;
} args_struct;

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
	/* if (front == NULL){
		return;
	} */

	/* directory_names *ptr = front; */
	directory_names*tmp;
    while (front != NULL) 
	{
		tmp = front;
        front = front->next;
		free(tmp); 
	}

	/* free(ptr); */
}

void *file_handler(void *file)
{
    directory_names *f = (directory_names *)file;
    printf("File: %s\n", f->data);
    return NULL;
}

void *directory_handler(void *direct)
{
    directory_names *directory = (directory_names *)direct;

    char *path = directory->data;

    DIR *dir = opendir(path);
    struct dirent *dp;

    if (dir == NULL)
    {
        perror(directory->data);
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
            char localname[strlen(directory->data) + strlen(dp->d_name) + 2];

            strcpy(localname, path);
            strcat(localname, "/");
            strcat(localname, dp->d_name);
            addNode(directory, localname);

            if (DEBUG == 0)
            {
                printf("%d\n", dp->d_type);
            }

            if (dp->d_type == DT_DIR)
            {
                printf("Node Directory added: %s\n", localname);
                pthread_create(&directory->id[ids++], NULL, &directory_handler, (void *)directory);
            }
            else if (dp->d_type == DT_REG)
            {
                pthread_create(&directory->id[ids++], NULL, &file_handler, (void *)directory);
            }
        }

        while (j < ids)
        {
            pthread_join(directory->id[j++], NULL);
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

    if (DEBUG == 0)
    {
        printf("%f\n", log10(2));
    }

    directory_names *directory = malloc(sizeof(directory_names));
    addNode(directory, argv[1]);

    directory_handler((void *)directory);

    free_list(directory);
    

    return 0;
}