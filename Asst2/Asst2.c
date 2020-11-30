#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <math.h>
#include <dirent.h>
#include <errno.h>

/*
 * This struct type is used to hold the tokens for the files.
 * 
 * It contains three fields: 
 *  1. char*word -> the word in the file
 *  2. double freq -> the frequency of the word
 *  3. struct Tokens*next -> the next token for the file
 */
typedef struct Tokens
{
    char* word; // be sure to use malloc when initializing word
    double freq;
    struct Tokens* next;
} Tokens;

/*
 * This struct type is used to contain the contents of each file.
 * 
 * It contains five fields:
 *  1. The tokens for the file
 *  2. The name of the file
 *  3. The FILE pointer of the file
 *  4. The number of tokens.
 *  5. The next file in the list.
 */
typedef struct Files
{
    struct Tokens* tokens;
    char* fileName; // be sure to use malloc when initializing fileName
    FILE* fp;
    int tokenCount;
    struct Files* next;
} Files;

/*
 * This struct type is used to contain the threads that will be produced in the directory handler.
 * 
 * It contains two fields:
 *  1. The thread id
 *  2. The next thread in the list.
 */
typedef struct Threads
{
    pthread_t threadID;
    struct Threads* next;
} Threads;

/*
 * This struct type is used to contain the directories and files for that directory.
 * 
 * It contains three fields:
 *  1. The mutex lock
 *  2. The directory name
 *  3. The front of the files list
 */
typedef struct DirHandleArgs
{
    pthread_mutex_t* lock;
    char* dirName;
    Files* filesHead;
} DirHandleArgs;

/* 
 * These function prototypes are used for debug output.
 */ 
void filesToString (Files* f);
void tokensToString (Tokens* t);

/*
 * These function prototypes are used for deallocating linked lists created.
 */
void freeThreads(Threads* head);
void freeFiles (Files* head);
void freeTokens (Tokens* head);

/*
 * These function prototype are used for handling directories that are encountered.
 */
void* directory_handler(void* direct);
Threads* appendThread (Threads* last);
Files* appendFile (FILE* f, char* fileName, Files* lastFile);

/* 
 * These function prototypes are used for tokenizing files.
 */
void* tokenizer (void* arg);
char* getBuff (char* oldBuff, int count);
char* getToken (FILE* f);
void insertToken (Files* f, char* word);

/* 
 * These function prototypes are used for comparing files.
 */
Files* mergesortFiles (Files* head);
Files* mergeFiles (Files* left, Files* right);
void getProb (Files* head);

/* 
 * These function prototypes are used when getting the mean and KLD.
 */
Tokens* computeMean (Files* f1, Files* f2);
double computeKLD (Tokens* mean, Tokens* t);

/* 
 * The tokensToString function will take the strut of tokens and print out the token list.
 */
void tokensToString (Tokens* t)
{
    while (t != NULL)
    {
        printf("\t%s %.2f ->\n", t->word, t->freq);
        t = t->next;
    }
    printf("\tend of list\n");
}

/* 
 * filesToString function will take Files struct as an argument and print the files list and their tokens
 * using tokensToString as a helper function.
 */
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

/* 
 * freeThreads function will take the list of threads that were created (Threads struct) and free the threads
 * in the list starting from head.
 */
void freeThreads (Threads* head)
{
    if (head == NULL) return;

    Threads* next = head->next;
    free(head);
    freeThreads(next);
}

/* 
 * freeFiles function will take the list of Files that were created and free the File pointer starting from the head.
 */
void freeFiles (Files* head)
{
    if (head == NULL) return;

    freeTokens(head->tokens);
    free(head->fileName);
    Files* next = head->next;
    free(head);
    freeFiles(next);
}

/*  
 * freeTokens function will take the list of tokens that were created and free the Tokens pointer
 * starting from the head.
 */
void freeTokens (Tokens* head)
{
    if (head == NULL) return;

    free(head->word);
    Tokens* next = head->next;
    free(head);
    freeTokens(next);
}

/* 
 * appendThread function will take the list of threads that were created starting from the last one and
 * append a new thread to it when called.
 */
Threads* appendThread (Threads* last)
{
    Threads* newNode = NULL;
    if (last == NULL) // first node
    {
        newNode = (Threads*) malloc(sizeof(Threads));
    }
    else
    {
        while (last->next != NULL) last = last->next;
        last->next = (Threads*) malloc(sizeof(Threads));
        newNode = last->next;
    }
    newNode->next = NULL;
    return newNode;
}

/* 
 * appendFile will take a File pointer and the name of the file and append the file to the end of that list.
 */
Files* appendFile (FILE* f, char* fileName, Files* lastFile)
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

/*
 * getBuff takes a old buffer and the count that you want for the new buffer and moves the string to a bigger buffer.
 */
char* getBuff (char* oldBuff, int count)
{
    char* newBuff = malloc(1024 * count);
    strcpy(newBuff, oldBuff);
    free(oldBuff);
    return newBuff;
}

/* 
 * getToken will take a File pointer then omit non alpha/hyphen char and turn alpha into lower case,
 * then return the cleaned token.
 */
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

/*
 * insertToken will take the Files list and the word (token) and insert it into the tokens list.
 */
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

/*
 * tokenizer function is our file handler which will be used as new threads created from in the directory
 * handler function. It takes a void pointer which is type casted into the Files list and creates tokens
 * based on the list.
 * 
 * Functionally it takes a node from the shared file list, constructed for the given file, then generate its token list.
 * The tokenizer contains helper methods that help when tokenizing the Files like:
 *  1. getToken -> get the token
 *  2. insertToken -> insert the token in the specific files token list
 */
void* tokenizer (void* arg)
{
    Files* ptr = (Files*) arg;
    char* cleanedToken = getToken(ptr->fp);
    while (strlen(cleanedToken) != 0)
    {
        ptr->tokenCount++;
        insertToken(ptr, cleanedToken);
        char* oldToken = cleanedToken;
        free(oldToken);
        cleanedToken = getToken(ptr->fp);
    }
    free(cleanedToken); // free the last cleaned token
    fclose(ptr->fp);
    return NULL;
}

/*
 * mergeFiles takes two files list and merges the two sorted files list.
 * Both of left and right lists are NOT NULL and this is ensured by the caller.
 */
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

/*
 * mergesortFiles function takes the list of Files and sorts the list using merge sort.
 */
Files* mergesortFiles(Files* head)
{
    if (head == NULL || head->next == NULL) return head; // no or only 1 node in the list

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

/*
 * getProb takes a Files list and turns the frequency of the tokens for every files into probability.
 */
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

/*
 * computeMean takes two Files list f1 and f2 and merges the tokens list in them into one token list
 * with mean of frequency.
 */
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

/* 
 * computeKLD takes the mean tokens and an one of the tokens list t that used to compute this mean.
 * 
 * When you find the token in T also in mean you multiply the discrete probability of its occurrence in the list
 * by the log 10 of its discrete probability in the list divided by the probability computed for it in means lists.
 */
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

/*
 * directory_handler takes a void pointer which is type casted into struct DirHandleArgs that carries arguments used,
 * and open the dirName when its found or returns gracefully if that directory isn't found.
 * 
 * Functionally it opens the directory that it is given with opendir() and iterates through the contents
 * of that directory using readdir(). When it finds a subdirectory it opens a new pthread using a copy
 * of the file handler and the nextDir string to open another directory. Otherwise it opens another pthread using
 * a copy of the file handler (tokenizer) which it passes a Files list to open every file.
 * 
 * Before creating a new thread it locks the mutex and add an entry to the files list to avoid multiple
 * directory_handler editing files list at the same time.
 * 
 * After nothing is found at the very end, it joins all threads created by itself and exits.
 */
void* directory_handler(void* direct)
{
    DirHandleArgs* args = (DirHandleArgs*) direct;
    DIR *dir = opendir(args->dirName);

    if (dir == NULL)
    {
        perror(args->dirName);
        free(args->dirName);
        free(args);
        pthread_exit(NULL);
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
                char* nextDir = (char*) malloc(strlen(args->dirName) + strlen(dp->d_name) + 2);
                strcpy(nextDir, args->dirName);
                strcat(nextDir, dp->d_name);
                strcat(nextDir, "/");

                tptr = appendThread(tptr);
                if (dirHead == NULL) dirHead = tptr;
                DirHandleArgs* newargs = (DirHandleArgs*) malloc(sizeof(DirHandleArgs));
                newargs->lock = args->lock;
                newargs->dirName = nextDir;
                newargs->filesHead = args->filesHead;
                pthread_create(&(tptr->threadID), NULL, directory_handler, newargs);
            }
            else
            {
                char* nextFile = (char*) malloc(strlen(args->dirName) + strlen(dp->d_name) + 1);
                strcpy(nextFile, args->dirName);
                strcat(nextFile, dp->d_name);

                FILE* fp = fopen(nextFile, "r");
                if (fp == NULL)
                {
                    perror(nextFile);
                    free(nextFile);
                    continue;
                }

                pthread_mutex_lock(args->lock);
                Files* newFile = appendFile(fp, nextFile, args->filesHead);
                pthread_mutex_unlock(args->lock);

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
        free(args->dirName);
        free(args);
        closedir(dir);

        pthread_exit(NULL);
    }
}

/*
 * The main function will take the path name where argv[1] contains the root directory that we will start traversing
 * to conduct the file analysis.
 * 
 * It initializes the root directory and Files list to start appending to when the analysis starts and at the very end
 * it compares the files using the probability that was found from it.
 * 
 * The main function will return EXIT_SUCCESS if the job is done or EXIT_FAILURE if main isn't given two arguments
 * at start or less than two files was read.
 */
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

    pthread_mutex_t fileListLock;
    pthread_mutex_init(&fileListLock, NULL);
    pthread_t init;

    Files* filesHead = (Files*) malloc(sizeof(Files));
    filesHead->tokens = NULL;
    filesHead->next = NULL;
    filesHead->fileName = NULL;
    filesHead->fp = NULL;
    filesHead->tokenCount = 0;

    DirHandleArgs* args = (DirHandleArgs*) malloc(sizeof(DirHandleArgs));
    args->lock = &fileListLock;
    args->dirName = dir;
    args->filesHead = filesHead;
    pthread_create(&init, NULL, directory_handler, args);

    pthread_join(init, NULL);

    // math starts from here
    if (filesHead->next == NULL)
    {
        printf("No or only one file found, stopped\n");
        freeFiles(filesHead);
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
