#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

/*
 * Currently: Working on file handling
 */

void *directory_handling(void *d_path)
{
    // do something

    DIR *dir = opendir(d_path);
    struct dirent *dp;
    char localname[200000];

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
            }
            else if (dp->d_type == DT_REG) 
            {
                printf("File Path: %s\n", localname); 
            }
            else
            {
                continue;
            }
        }
    }
    else
    {
        printf("%s\n", (char *)d_path);
        perror(d_path);
        exit(1);
    }

    printf("Ending directory handler\n");
    closedir(dir);

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