#ifndef HELLO_WORLD_H
#define HELLO_WORLD_H

typedef enum toktype_
{
    WORD,
    DECIMAL_INTEGER,
    OCTAL_INTEGER,
    HEXADECIMAL_INTEGER,
    C_OPERATOR
} TokenTypes;

typedef struct _TokenNode_ {
	char* data;
	struct _TokenNode_ * next;
	TokenTypes names;
} TokenNode;

void makeStringToken(char *input, char*storage);

#endif
