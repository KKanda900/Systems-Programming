#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Linked List for word tokens in a file.
typedef struct Tokens
{
    char* word;
    int freq;
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

// Print out the token list
void tokensToString (Tokens* t)
{
    while (t != NULL)
    {
        printf("\t%s %d ->\n", t->word, t->freq);
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

            if (isupper(ch))
            {
                ch += 32;
            }
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
    // lock
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
    // unlock

    char* cleanedToken = getToken(f);
    while (strlen(cleanedToken) != 0)
    {
        ptr->tokenCount++;
        insertToken(ptr, cleanedToken);
        char* oldToken = cleanedToken;
        free(oldToken);
        cleanedToken = getToken(f);
    }
    free(cleanedToken);
    // return ptr;
}

int main (int argc, char** argv) {
/*    printf("%d\n", argc);
    if (argc < 2) {
        printf("Provide a directory.\n");
        return EXIT_SUCCESS;
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
    FILE* fp1 = fopen("test0.txt", "r");
    tokenizer(fp1, "./test0.txt", filesHead); // I can let this return the last files node to reduce time
    fclose(fp1);
    FILE* fp2 = fopen("test1.txt", "r");
    tokenizer(fp2, "./test1.txt", filesHead); // I can let this return the last files node to reduce time
    fclose(fp2);

    filesToString(filesHead);

    freeFiles(filesHead);

    return EXIT_SUCCESS;
}