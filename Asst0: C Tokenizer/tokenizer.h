typedef enum toktype_
{
    WORD,
    DECIMAL_INTEGER,
    OCTAL_INTEGER,
    HEXADECIMAL_INTEGER,
    C_OPERATOR,
    BAD_TOKEN
} TokenTypes;

typedef struct _Tok_ {
	char* data; // This should store the input you give in argv[1] into a individual string without any delimiters
	TokenTypes names;
} Token;

char* getRidOfDelims(char *input, char*storage);

