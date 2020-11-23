#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

typedef struct words
{
    char word[50];
    int freq;
    struct words *next;
} Words;

Words *addWord(char word[], Words *wf, struct dirent *dp)
{
    Words *newWF = malloc(sizeof(Words));
    strcpy(newWF->word, word);
    newWF->freq = freqCount(newWF->word, dp->d_name);
    //printf("%s | %d\n", newWF->word, newWF->freq);
    newWF->next = NULL;

    if (wf == NULL)
    {
        wf = newWF;
        return wf;
    }

    newWF->next = wf->next;
    wf->next = newWF;
    return wf;
}

int freqCount(char word1[], char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    char word[50];
    int counter = 0;

    while (fscanf(fp, "%s", word) != EOF)
    {
        if (strcmp(word, word1) == 0)
        {
            counter++;
        }
    }

    fclose(fp);
    return counter;
}

void printList(Words *wf)
{
    while (wf != NULL)
    {
        printf("Word - %s | Frequency - %d\n", wf->word, wf->freq);
        wf = wf->next;
    }
}

void free_list(Words *front)
{
    if (front == NULL)
    {
        return;
    }

    Words *ptr = front;
    while (ptr->next != NULL)
    {
        Words *tmp = ptr->next;
        free(ptr);
        ptr = tmp;
    }

    free(ptr);
}

const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "";
    return dot + 1;
}

int main()
{
    DIR *direct = opendir(".");
    struct dirent *dp;
    struct stat sb;

    while ((dp = readdir(direct)))
    {
        if (dp->d_type == DT_DIR)
        {
            continue;
        }
        else
        {
            if (stat(dp->d_name, &sb) != -1)
            {
                if (sb.st_mode & S_IXUSR) // binary files
                {
                    //skip
                }
                else
                { 
                    int fd = open(dp->d_name, O_RDONLY);
                    int size = lseek(fd, 0, SEEK_END);
                    FILE *fp = fopen(dp->d_name, "r");
                    char word[1024];
                    Words *wf = malloc(sizeof(Words));
                    //int word_count = 0;

                    char*tokens;

                    while (fgets(word, 1024, fp) != NULL)
                    {
                        tokens = strtok(word, ".!,?;:-_[]{}'""''* ");
                        while(tokens != NULL){
                            wf = addWord(tokens, wf, dp);
                            printf("%s\n", tokens); 
                            tokens = strtok(NULL, ".!,?;:-_[]{}'""''*   <>#");
                        }
                    }

                    printList(wf);
                    free_list(wf); 
                    close(fd);
                }
            }
        }
    }
    closedir(direct);

    return 0;
}

/* int main(int argc, char **argv)
{

    FILE *fp = fopen("example.txt", "r");
    char word[50];
    Words *wf = malloc(sizeof(Words));

    while (fscanf(fp, "%s", word) != EOF)
    {
        wf = addWord(word, wf);
    }

    fclose(fp);

    printList(wf);


    free_list(wf);

    return 0;
} */