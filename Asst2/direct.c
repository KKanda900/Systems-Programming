#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>


const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int main()
{
    DIR *direct = opendir(".");
    struct dirent *dp;
 
    while ((dp = readdir(direct)))
    {
        if (dp->d_type == DT_DIR)
        {
            continue;
        }
        else
        {
            if (dp->d_type == DT_REG)
            {
                int fd = open(dp->d_name, O_RDONLY);
                int size = lseek(fd, 0, SEEK_END);
                if(strcmp(get_filename_ext(dp->d_name), "txt") == 0){
                    printf("Success\n");
                    printf("%s\n", dp->d_name);
                    break;
                }
                close(fd);
            }
        }
    }
 
    closedir(direct);
 
    return 0; 
}