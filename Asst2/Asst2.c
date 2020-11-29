#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <math.h>
#include <dirent.h>
#include <errno.h>

// Linked List for word tokens in a file.
typedef struct Tokens
{
    char* word; // be sure to use malloc when initializing word
    double freq;
    struct Tokens* next;
} Tokens;

// Linked List for files.
typedef struct Files
{
    struct Tokens* tokens;
    char* fileName; // be sure to use malloc when initializing fileName
    FILE* fp;
    int tokenCount;
    struct Files* next;
} Files;

typedef struct Threads
{
    pthread_t threadID;
    struct Threads* next;
} Threads;

pthread_mutex_t fileListLock;
Files* filesHead;
Files* lastFile;

void filesToString (Files* f);
void tokensToString (Tokens* t);
void freeThreads(Threads* head);
void freeFiles (Files* head);
void freeTokens (Tokens* head);
Threads* appendThread(Threads* last);
Files* appendFile(FILE* f, char* fileName);

void* directory_handler(void* direct);

void* tokenizer (void* arg);
char* getBuff (char* oldBuff, int count);
char* getToken (FILE* f);
void insertToken (Files* f, char* word);

Files* mergesortFiles(Files* head);
Files* mergeFiles(Files* left, Files* right);
void getProb(Files* head);

Tokens* computeMean(Files* f1, Files* f2);
double computeKLD(Tokens* mean, Tokens* t);

// Print out the token list
void tokensToString (Tokens* t)
{
    while (t != NULL)
    {
        printf("\t%s %.2f ->\n", t->word, t->freq);
        t = t->next;
    }
    printf("\tend of list\n");
}

// Print out the file list
void filesToString (Files* f)
{
    while (f != NULL)
    {
        printf("%s %d ->\n", f->fileName, f->tokenCount);
        tokensToString(f->tokens);
        f = f->next;
    }
    printf("end of list\n");
}

// Free all Threads node starting from the given head.
void freeThreads (Threads* head)
{
    if (head == NULL) return;

    Threads* next = head->next;
    free(head);
    freeThreads(next);
}

// Free all Files node starting from the given head.
void freeFiles (Files* head)
{
    if (head == NULL) return;

    freeTokens(head->tokens);
    free(head->fileName);
    Files* next = head->next;
    free(head);
    freeFiles(next);
}

// Free all Tokens node starting from the given head.
void freeTokens (Tokens* head)
{
    if (head == NULL) return;

    free(head->word);
    Tokens* next = head->next;
    free(head);
    freeTokens(next);
}

// create new node at the end of thread list
Threads* appendThread(Threads* last)
{
    Threads* newNode = NULL;
    if (last == NULL) // first node
    {
        newNode = (Threads*) malloc(sizeof(Threads));
    }
    else if (last->next != NULL)
    {
        printf("Not pointing to the end of list, check the code\n");
        return NULL;
    }
    else
    {
        last->next = (Threads*) malloc(sizeof(Threads));
        newNode = last->next;
    }
    newNode->next = NULL;
    return newNode;
}

// create new node at the end of file list
Files* appendFile(FILE* f, char* fileName)
{
    if (lastFile->fileName == NULL) // This is a new head of Files
    {
        lastFile->fileName = malloc(strlen(fileName) + 1);
        strcpy(lastFile->fileName, fileName);
        lastFile->fp = f;
    }
    else
    {
        while (lastFile->next != NULL) // find the end of list
        {
            lastFile = lastFile->next;
        }
        lastFile->next = (Files *) malloc(sizeof(Files));
        lastFile = lastFile->next;

        lastFile->tokens = NULL;
        lastFile->fileName = malloc(strlen(fileName) + 1);
        strcpy(lastFile->fileName, fileName);
        lastFile->fp = f;
        lastFile->tokenCount = 0;
        lastFile->next = NULL;
    }
    return lastFile;
}

// move string to a bigger buffer
char* getBuff (char* oldBuff, int count)
{
    char* newBuff = malloc(1024 * count);
    strcpy(newBuff, oldBuff);
    free(oldBuff);
    return newBuff;
}

// omit non alpha/hyphen char and turn alpha into lower case
char* getToken (FILE* f)
{
    int validCount = 0;
    int buffCount = 1;
    char* buff = malloc(1024);
    buff[0] = '\0';

    int c = fgetc(f);
    char ch = '\0';
    while (c != EOF && (!isspace(c) || validCount == 0))
    {
        ch = (char)c;
        if (isalpha(ch) || ch == '-')
        {
            validCount++;
            if (strlen(buff) == buffCount * 1024 - 1)
            {
                buffCount++;
                buff = getBuff(buff, buffCount);
            }
            if (isupper(ch)) ch += 32; // to lower case
            strncat(buff, &ch, 1);
        }
        c = fgetc(f);
    }
    char* string = malloc(validCount + 1);
    strcpy(string, buff);
    free(buff);
    return string;
}

// Insert the given token into the token list
void insertToken (Files* f, char* word)
{
    Tokens* ptr = NULL;
    if (f->tokens == NULL) // first token
    {
        f->tokens = (Tokens*) malloc(sizeof(Tokens));
        ptr = f->tokens;
        ptr->next = NULL;
    }
    else
    {
        Tokens* prev = NULL;
        ptr = f->tokens;
        while (ptr != NULL && strcmp(word, ptr->word) > 0)
        {
            prev = ptr;
            ptr = ptr->next;
        }
        if (prev == NULL) // front
        {
            if (strcmp(word, ptr->word) == 0) // token existed
            {
                ptr->freq++;
                return;
            }
            f->tokens = (Tokens*) malloc(sizeof(Tokens));
            f->tokens->next = ptr;
            ptr = f->tokens;
        }
        else if (ptr == NULL) // end of list
        {
            prev->next = (Tokens*) malloc(sizeof(Tokens));
            ptr = prev->next;
            ptr->next = NULL;
        }
        else if (strcmp(word, ptr->word) == 0) // token existed
        {
            ptr->freq++;
            return;
        }
        else // insert in the middle
        {
            prev->next = (Tokens*) malloc(sizeof(Tokens));
            prev->next->next = ptr;
            ptr = prev->next;
        }
    }

    ptr->word = malloc(strlen(word) + 1);
    strcpy(ptr->word, word);
    ptr->freq = 1;
}

// Take a opened FILE and a pointer pointing to any node of the shared file list, construct tokens for the given file.
void* tokenizer (void* arg)
{
    Files* ptr = (Files*) arg;
    char* cleanedToken = getToken(ptr->fp);
    while (strlen(cleanedToken) != 0)
    {
        ptr->tokenCount++;
        insertToken(ptr, cleanedToken);
        char* oldToken = cleanedToken; // just free cleanedToken gives warning for some reason
        free(oldToken);
        cleanedToken = getToken(ptr->fp);
    }
    free(cleanedToken); // free the last cleaned token
    fclose(ptr->fp);
    return NULL;
}

// Merge two sorted Files lists. Both left & right are not NULL is ensured by caller
Files* mergeFiles(Files* left, Files* right)
{
    Files* ptr = NULL;
    Files* sortedHead = NULL;

    while (left != NULL && right != NULL)
    {
        if (left->tokenCount > right->tokenCount)
        {
            if (sortedHead == NULL) // first node
            {
                ptr = right;
                sortedHead = ptr;
            }
            else
            {
                ptr->next = right;
                ptr = ptr->next;
            }
            right = right->next;
        }
        else
        {
            if (sortedHead == NULL) // first node
            {
                ptr = left;
                sortedHead = ptr;
            }
            else
            {
                ptr->next = left;
                ptr = ptr->next;
            }
            left = left->next;
        }
    }

    if (right == NULL) ptr->next = left;
    else ptr->next = right;

    return sortedHead;
}

// Sorting Files list with merge sort
Files* mergesortFiles(Files* head)
{
    if (head == NULL || head->next == NULL) return head; // only 1 node in the list

    Files* middle = head;
    Files* end = head->next;
    while (end != NULL)
    {
        end = end->next;
        if (end != NULL)
        {
            end = end->next;
            middle = middle->next;
        }
    }
    Files* left = head;
    Files* right = middle->next;
    middle->next = NULL;
    return mergeFiles(mergesortFiles(left), mergesortFiles(right));
}

// Turn frequency of tokens into prob
void getProb(Files* head)
{
    while (head != NULL)
    {
        Tokens* ptr = head->tokens;
        while (ptr != NULL)
        {
            ptr->freq = ptr->freq/head->tokenCount;
            ptr = ptr->next;
        }
        head = head->next;
    }
}

// Merge two tokens lists and compute the mean
Tokens* computeMean(Files* f1, Files* f2)
{
    Tokens* t1 = f1->tokens;
    Tokens* t2 = f2->tokens;
    if (t1 == NULL && t2 == NULL) return NULL;

    Tokens* mean = (Tokens*) malloc(sizeof(Tokens));
    Tokens* ptr = mean;
    mean->word = NULL;
    mean->next = NULL;

    while (t1 != NULL || t2 != NULL)
    {
        if (ptr->word != NULL) // not the first node
        {
            ptr->next = (Tokens*) malloc(sizeof(Tokens));
            ptr = ptr->next;
            ptr->next = NULL;
        }

        if (t2 == NULL || (t1 != NULL && strcmp(t1->word, t2->word) < 0))
        {
            ptr->word = (char*) malloc(strlen(t1->word) + 1);
            strcpy(ptr->word, t1->word);
            ptr->freq = t1->freq/2;
            t1 = t1->next;
        }
        else if (t1 == NULL || (t2 != NULL && strcmp(t1->word, t2->word) > 0))
        {
            ptr->word = (char*) malloc(strlen(t2->word) + 1);
            strcpy(ptr->word, t2->word);
            ptr->freq = t2->freq/2;
            t2 = t2->next;
        }
        else
        {
            ptr->word = (char*) malloc(strlen(t1->word) + 1);
            strcpy(ptr->word, t1->word);
            ptr->freq = (t1->freq + t2->freq)/2;
            t1 = t1->next;
            t2 = t2->next;
        }
    }

    return mean;
}

double computeKLD(Tokens* mean, Tokens* t)
{
    double result = 0;
    while (t != NULL)
    {
        while (strcmp(mean->word, t->word) != 0)
        {
            mean = mean->next;
        }
        result += t->freq*log10(t->freq/mean->freq);
        t = t->next;
        mean = mean->next;
    }
    return result;
}

void* directory_handler(void* direct)
{
    char* address = (char*) direct;
    DIR *dir = opendir(address);

    if (dir == NULL)
    {
        printf("Cannot open %s (%d)\n", address, errno);
        perror(address);
        free(address);
        exit(1);
    }
    else
    {
        Threads* dirHead = NULL;
        Threads* tptr = NULL;
        struct dirent *dp;

        while ((dp = readdir(dir)) != NULL)
        {
            if ((strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)) continue;

            if (dp->d_type == DT_DIR)
            {
                char* nextDir = (char*) malloc(strlen(address) + strlen(dp->d_name) + 2);
                strcpy(nextDir, address);
                strcat(nextDir, dp->d_name);
                strcat(nextDir, "/");

                printf("Opening dir %s\n", nextDir);

                tptr = appendThread(tptr);
                if (dirHead == NULL) dirHead = tptr;
                pthread_create(&(tptr->threadID), NULL, directory_handler, nextDir);
            }
            else // or just else?
            {
                char* nextFile = (char*) malloc(strlen(address) + strlen(dp->d_name) + 1);
                strcpy(nextFile, address);
                strcat(nextFile, dp->d_name);

                printf("Opening file %s\n", nextFile);
                FILE* fp = fopen(nextFile, "r");
                if (fp == NULL)
                {
                    printf("Cannot open %s (%d)\n", nextFile, errno);
                    free(nextFile);
                    continue;
                }

                pthread_mutex_lock(&fileListLock);
                Files* newFile = appendFile(fp, nextFile);
                pthread_mutex_unlock(&fileListLock);

                tptr = appendThread(tptr);
                if (dirHead == NULL) dirHead = tptr;
                pthread_create(&(tptr->threadID), NULL, tokenizer, newFile);
                free(nextFile);
            }
        }

        Threads* ttj = dirHead;
        while (ttj != NULL)
        {
            pthread_join(ttj->threadID, NULL);
            ttj = ttj->next;
        }
        freeThreads(dirHead);
        free(address);
        closedir(dir);

        pthread_exit(NULL);
    }
}

int main (int argc, char** argv) {
    if (argc < 2)
    {
        printf("Provide a directory.\n");
        return EXIT_FAILURE;
    }

    int dirlen = (int) strlen(argv[1]);
    if (argv[1][dirlen - 1] != '/') dirlen++;

    char* dir = (char*) malloc(dirlen + 1); // +1 for '\0'
    strcpy(dir, argv[1]);
    if (dirlen != strlen(argv[1])) strcat(dir, "/");
    printf("%s\n", dir);

    pthread_mutex_init(&fileListLock, NULL);
    pthread_t init;

    filesHead = (Files*) malloc(sizeof(Files));
    filesHead->tokens = NULL;
    filesHead->next = NULL;
    filesHead->fileName = NULL;
    filesHead->fp = NULL;
    filesHead->tokenCount = 0;
    lastFile = filesHead;

    pthread_create(&init, NULL, directory_handler, dir);

    pthread_join(init, NULL);

    filesToString(filesHead);

    // math starts from here
    if (filesHead->next == NULL)
    {
        printf("No or only one file found, stopped\n");
        return EXIT_FAILURE;
    }

    filesHead = mergesortFiles(filesHead);
    getProb(filesHead);

    Files* f1 = filesHead;
    char* color_default = "\033[0m";
    char* color_red = "\033[0;31m";
    char* color_green = "\033[0;32m";
    char* color_yellow = "\033[0;33m";
    char* color_blue = "\033[0;34m";
    char* color_cyan = "\033[0;36m";
    char* color = NULL;
    while (f1->next != NULL)
    {
        Files* f2 = f1->next;
        while (f2 != NULL)
        {
            Tokens* mean = computeMean(f1, f2);
            double result = (computeKLD(mean, f1->tokens) + computeKLD(mean, f2->tokens))/2;
            freeTokens(mean);

            if (result > 0.3) color = color_default;
            else if (result > 0.25) color = color_blue;
            else if (result > 0.2) color = color_cyan;
            else if (result > 0.15) color = color_green;
            else if (result > 0.1) color = color_yellow;
            else color = color_red;

            printf("%s%.2f%s \"%s\" and \"%s\"\n", color, result, color_default, f1->fileName, f2->fileName);
            f2 = f2->next;
        }
        f1 = f1->next;
    }

    freeFiles(filesHead);

    return EXIT_SUCCESS;
}
