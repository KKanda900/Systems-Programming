#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tokenizer.h"

void makeStringToken(char *input, char*storage)
{
    int j;
    for (j = 0; j < strlen(input); j++)
    {
        if (isspace(input[j]))
        {
            printf("%s\n", storage);
            j++;
            continue;
        }

        storage[j] = input[j];

        if (input[j] == '\0')
        {
            break;
        }
    }
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
    int length = 0;
    if (argv[1] != NULL)
    {
        length = strlen(argv[1]);
        input = argv[1];
        //printf("The input string is: %s\n", input);
    }

    char*storage = malloc((length+1)*sizeof(char));
    makeStringToken(input, storage);

    free(storage);
    return EXIT_SUCCESS;
}