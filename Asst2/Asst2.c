#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <math.h>

// Linked List for word tokens in a file.
typedef struct Tokens
{
    char* word;
    double freq;
    struct Tokens* next;
} Tokens;

// Linked List for files.
typedef struct Files
{
    struct Tokens* tokens;
    char* fileName;
    int tokenCount;
    struct Files* next;
} Files;

void filesToString (Files* f);
void tokensToString (Tokens* t);
void freeFiles (Files* head);
void freeTokens (Tokens* head);

void tokenizer (FILE* f, char* filename, Files* ptr);
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

// Free all Files node from the given head.
void freeFiles (Files* head)
{
    if (head == NULL) return;

    freeTokens(head->tokens);
    free(head->fileName);
    Files* next = head->next;
    free(head);
    freeFiles(next);
}

// Free all Tokens node from the given head.
void freeTokens (Tokens* head)
{
    if (head == NULL) return;

    free(head->word);
    Tokens* next = head->next;
    free(head);
    freeTokens(next);
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
    while (c != EOF && !isspace(c))
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
        if (prev == NULL) // new front
        {
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
// Also save the filename for later use
void tokenizer (FILE* f, char* filename, Files* ptr)
{
    // move below out to directory handling
    if (ptr->fileName == NULL) // This is a new head of Files
    {
        ptr->fileName = malloc(strlen(filename) + 1);
        strcpy(ptr->fileName, filename);
    }
    else
    {
        while (ptr->next != NULL) // find the end of list
        {
            ptr = ptr->next;
        }
        ptr->next = (Files *) malloc(sizeof(Files));
        ptr = ptr->next;

        ptr->tokens = NULL;
        ptr->fileName = malloc(strlen(filename) + 1);
        strcpy(ptr->fileName, filename);
        ptr->tokenCount = 0;
        ptr->next = NULL;
    }
    // move above out to directory handling

    char* cleanedToken = getToken(f);
    while (strlen(cleanedToken) != 0)
    {
        ptr->tokenCount++;
        insertToken(ptr, cleanedToken);
        char* oldToken = cleanedToken; // just free cleanedToken gives warning for some reason
        free(oldToken);
        cleanedToken = getToken(f);
    }
    free(cleanedToken); // free the last cleaned token
    // return ptr; // for faster file insertion
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

int main (int argc, char** argv) {
/*    printf("%d\n", argc);
    if (argc < 2) {
        printf("Provide a directory.\n");
        return EXIT_FAILURE;
    }

    int dirnlen = (int)strlen(argv[1]);
    char* dir = (char*)malloc(dirnlen+1); //+1 for '\0'
    strcpy(dir, argv[1]);*/

    Files* filesHead = (Files*) malloc(sizeof(Files));
    filesHead->tokens = NULL;
    filesHead->next = NULL;
    filesHead->fileName = NULL;
    filesHead->tokenCount = 0;

    // scan through dir
    // found somefile.txt
    FILE* fp1 = fopen("test1.txt", "r");
    tokenizer(fp1, "./test1.txt", filesHead);
    fclose(fp1);
    FILE* fp2 = fopen("test0.txt", "r");
    tokenizer(fp2, "./test0.txt", filesHead);
    fclose(fp2);
    FILE* fp3 = fopen("test2.txt", "r");
    tokenizer(fp3, "./test2.txt", filesHead);
    fclose(fp3);

    // start from here
    if (filesHead->next == NULL)
    {
        printf("Only one file found, stopped\n");
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
