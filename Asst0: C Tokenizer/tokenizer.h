typedef enum toktype_
{
    WORD,
    DECIMAL_INTEGER,
    OCTAL_INTEGER,
    HEXADECIMAL_INTEGER,
    C_OPERATOR
} TokenTypes;

typedef struct _Tok_ {
	char* data;
    int pointer;
	TokenTypes names;
} Token;

char* getRidOfDelims(char *input, char*storage);

