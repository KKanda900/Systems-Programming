#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tokenizer.h"

/* void identifyToken(Token storedString) {
    for(int j = 0; j < strlen(storedString.data); j++){

    }
} */

//This seperate the words given by the input so once it finds a space it will use the words that were already built in to storage to create the token in the get Token function
char *getRidOfDelims(char *input, char *storage)
{
    int j;
    for (j = 0; j < strlen(input); j++)
    {
        if (isspace(input[j]) || input[j] == '\t' || input[j] == '\v' || input[j] == '\n' || input[j] == '\r')
        {
            continue;
        }

        storage[j] = input[j];

        if (input[j] == '\0')
        {
            break;
        }
    }
    return storage;
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        printf("Provide enough arguments\n");
        return -1;
    }

    if (argv[1] == NULL)
    {
        printf("Provide a valid string to read from\n");
        return -1;
    }

    char *input = NULL;
    char *copyInput = NULL;
    int length = 0;
    if (argv[1] != NULL)
    {
        length = strlen(argv[1]);
        input = argv[1];
        //printf("The input string is: %s\n", input);
    }
    strcpy(copyInput, input);

    Token storedString; storedString.data = malloc((length + 1) * sizeof(char));
    storedString.data = getRidOfDelims(copyInput, storedString.data);
    printf("%s\n", storedString.data);

    free(storedString.data);
    return EXIT_SUCCESS;
}