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

// Print out the token list
void tokensToString (Tokens* t)
{
    while (t != NULL)
    {
        printf("%s %d ->\n", t->word, t->freq);
        t = t->next;
    }
    printf("end of list\n");
}

// Print out the file list
void filesToString (Files* f)
{
    while (f != NULL)
    {
        printf("%s %d ->\n", f->fileName, f->tokenCount);
        f = f->next;
    }
    printf("end of list\n");
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
char* cleanToken (FILE* f)
{
    int validCount = 0;
    int buffCount = 1;
    char* buff = malloc(1024);

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
    return string;
}

// Insert the given token into the token list
void insertToken (Tokens* ptr, char* word)
{
    Tokens* prev = NULL;
    while (ptr != NULL && strcmp(word, ptr->word) > 0)
    {
        prev = ptr;
        ptr = ptr->next;
    }
    if (ptr == NULL) // end of list or root is empty
    {
        ptr = (Tokens*) malloc(sizeof(Tokens));
        ptr->word = malloc(strlen(word) + 1);
        strcpy(ptr->word, word);
        ptr->freq = 1;
        ptr->next = NULL;
        return;
    }

    if (strcmp(word, ptr->word) == 0) // token existed
    {
        ptr->freq++;
        return;
    }

    Tokens* curr = (Tokens*) malloc(sizeof(Tokens));
    curr->word = malloc(strlen(word) + 1);
    strcpy(curr->word, word);
    curr->freq = 1;
    prev->next = curr;
    curr->next = ptr;
}

// Take a opened FILE and a pointer pointing to any node of the shared file list, construct tokens for the given file
void tokenizer (FILE* f, Files* ptr)
{
    // lock
    while (ptr != NULL) // find the end of list
    {
        ptr = ptr->next;
    }
    ptr = (Files*) malloc(sizeof(Files));
    ptr->tokens = NULL;
    ptr->tokenCount = 0;
    ptr->next = NULL;
    // unlock

    char* cleanedToken = cleanToken(f);
    while (cleanedToken != NULL)
    {
        insertToken(ptr->tokens, cleanedToken);
        cleanedToken = cleanToken(f);
    }
}

int main (int argc, char** argv) {
    printf("%d\n", argc);
    if (argc < 2) {
        printf("Provide a directory.\n");
        return EXIT_SUCCESS;
    }

    int dirnlen = (int)strlen(argv[1]);
    char* dir = (char*)malloc(dirnlen+1); //+1 for '\0'
    strcpy(dir, argv[1]);

}