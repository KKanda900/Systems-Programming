#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// DO NOT LEAVE OUT BRACKETS WHEN USING THESE MARCO!!

// Add char at current index to currToken.data and change Token name to t.
#define addCharChangeTo(t) strncat(currToken.data, &c, sizeof(char)); index++; currToken.names = t

// Print out the only char as Token t.
#define printAs(t) currToken.names = t; currToken = tokenPrinter(currToken); break

/*
 * Here is a enum we use to define an easier way to identify the token type for all 48 token,
 * includes a Modulus token "%" which is not in RefCard.
 */

typedef enum TokenType
{
    // Full name of types in definition of concludeToken
    NEW, // Initial type
    WORD, DEC_INT, OCT_INT, HEX_INT, FLOAT,
    MINUS, STRUCT_PTR, DECRE, MINUS_EQ,
    NEGATE, NEQ_TEST,
    GREATER, SHIFT_R, GREATER_EQ, SHIFT_R_EQ,
    LESS, SHIFT_L, LESS_EQ, SHIFT_L_EQ,
    XOR, XOR_EQ,
    OR, LOG_OR, BIT_OR,
    ADD, INCRE, ADD_EQ,
    DIV, DIV_EQ,
    AND, LOG_AND, BIT_AND,
    ASSIGN, EQ,
    MULTI, MULTI_EQ,
    MOD, MOD_EQ,
    PAREN_L, PAREN_R,
    BRAC_L, BRAC_R,
    STRUCT_MEM, COMMA, ONES_COMP,
    COND_T, COND_F
} TokenType;

/*
 * This struct type is used to hold the token that we are going to print and the identifying token type that
 * we will print out in the tokenScanner function.
 *
 * It contains two fields: the char* to store the data of the token and the enum Tokentype to identify the token
 */

typedef struct Token {
	char* data;
	TokenType names; 
} Token;

/*
 * These are the reserved keywords specified in C.
 */

const char* reserved[32] =
        {"auto", "break", "case", "char",
         "const", "continue", "default", "do",
         "int", "long", "register", "return",
         "short", "signed", "sizeof", "static",
         "struct", "switch", "typedef", "union",
         "unsigned", "void", "volatile", "while",
         "double", "else", "enum", "extern",
         "float", "for", "goto", "if"};

/*
 * Here are some function prototypes used in our tokenizer implementation
 */

Token tokenPrinter(Token t);
int isopt(char c);
void tokenScanner(int inputlen, char*input, Token currToken);

/*
 * The isopt function will return 1 (true) if there was an operator that was found or return 0 (false) if there wasn't
 * any of the corresponding operators found.
 *
 * The corresponding operators that it is comparing the character you provided with is
 * "!%&()*+,-./:<=>?[]^|~".
 *
 * The isopt function takes a character as a argument.
 */

int isopt(char c)
{
    char* operators = "!%&()*+,-./:<=>?[]^|~";
    for (int i = 0; i < 21; i++)
    {
        if (c == operators[i]) return 1;
    }
    return 0;
}

/*
 * The tokenPrinter function determines the Token's type name from the given enum Token type name and
 * prints out the Token the string generates in the form:
 * <token type>: "<token name>"
 *
 * The tokenPrinter function takes in the struct Token which holds the data of the token its going to print
 * out and the name of the Token type.
 *
 * If the Token is a WORD it will go into a separate statement where it will determine if it's a special
 * keyword and print out the corresponding word or keyword. (Extra credit part 1)
 * Otherwise the function at the end will print the Token type and the the Token data.
 */

Token tokenPrinter(Token t) {
    int printed = 0;
    char* typename;
    switch (t.names)
    {
        case WORD: typename = "word"; break;
        case DEC_INT: typename = "decimal integer"; break;
        case OCT_INT: typename = "octal integer"; break;
        case HEX_INT: typename = "hexadecimal integer"; break;
        case FLOAT: typename = "float"; break;

        case MINUS: typename = "minus/subtract operator"; break; // -
        case STRUCT_PTR: typename = "structure pointer"; break; // ->
        case DECRE: typename = "decrement"; break; // --
        case MINUS_EQ: typename = "minus equals"; break; // -=

        case NEGATE: typename = "negate"; break; // !
        case NEQ_TEST: typename = "inequality test"; break; // !=

        case GREATER: typename = "greater than test"; break; // >
        case SHIFT_R: typename = "shift right"; break; // >>
        case GREATER_EQ: typename = "greater than or equal test"; break; // >=
        case SHIFT_R_EQ: typename = "shift right equals"; break; // >>=

        case LESS: typename = "less than test"; break;// <
        case SHIFT_L: typename = "shift left"; break; // <<
        case LESS_EQ: typename = "less than or equal"; break; // <=
        case SHIFT_L_EQ: typename = "shift left equals"; break; // <<=

        case XOR: typename = "bitwise XOR"; break; // ^
        case XOR_EQ: typename = "bitwise XOR equals"; break; // ^=

        case OR: typename = "bitwise OR"; break; // |
        case LOG_OR: typename = "logical OR"; break; // ||
        case BIT_OR: typename = "bitwise OR equals"; break; // |=

        case ADD: typename = "addition"; break; // +
        case INCRE: typename = "increment"; break; // ++
        case ADD_EQ: typename = "plus equals"; break; // +=

        case DIV: typename = "division"; break; // /
        case DIV_EQ: typename = "divide equals"; break; // /=

        case AND: typename = "AND/address operator"; break; // &
        case LOG_AND: typename = "logical AND"; break; // &&
        case BIT_AND: typename = "bitwise AND"; break; // &=

        case ASSIGN: typename = "assignment"; break; // =
        case EQ: typename = "equality test"; break; // ==

        case MULTI: typename = "multiply/dereference operator"; break; // *
        case MULTI_EQ: typename = "times equals"; break; // *=

        case MOD: typename = "modulus"; break; // % (not on ref card)
        case MOD_EQ: typename = "mod equals"; break; // %=

        case PAREN_L: typename = "left parenthesis"; break; // (
        case PAREN_R: typename = "right parenthesis"; break; // )

        case BRAC_L: typename = "left bracket"; break; // [
        case BRAC_R: typename = "right bracket"; break; // ]

        case STRUCT_MEM: typename = "structure member"; break; // .
        case COMMA: typename = "comma"; break; // ,
        case ONES_COMP: typename = "1s complement"; break; // ~
        case COND_T: typename = "conditional true"; break; // ?
        case COND_F: typename = "conditional false"; break; // :
        default: // Should never fall into this case
            printf("Not suppose to conclude a new token.\n");
            return (Token){0};
    }
    if (t.names == WORD)
    {
        for (int i = 0; i < 32; i++)
        {
            if (strcmp(t.data, reserved[i]) == 0)
            {
                printf("%s: \"%s\"\n", reserved[i], t.data);
                printed = 1;
                break;
            }
        }
    }

    if (printed == 0) printf("%s: \"%s\"\n", typename, t.data); // Or already printed as a special keyword
    free(t.data);
    return (Token){0};
}


/*
 * The tokenScanner function is a void function that will perform the actual tokenization where
 * it will determine the token type and start concatenating the token name into currToken.data
 * and determine the token type which will go into currToken.name which will be used to generate the token
 * in the tokenPrinter function.
 *
 * The arguments tokenScanner takes is the size of the input string you provide in argv[1], the input string
 * which is argv[1], and the struct Token variable which will be used to store any information about the token.
 *
 * At the end it will also check if there is one last token left in the input string and print that out
 * after the preceding tokens are printed out.
 */

void tokenScanner(int inputlen, char*input, Token currToken) {
    int hasexponent = 0; int putdigit = 0; int index = 0;
    while (index < inputlen) {
        char c = input[index];
        switch (currToken.names) {
            case MINUS:
                switch (c)
                {
                    case '>': addCharChangeTo(STRUCT_PTR); break;
                    case '-': addCharChangeTo(DECRE); break;
                    case '=': addCharChangeTo(MINUS_EQ); break;
                    default: ;
                }
                currToken = tokenPrinter(currToken);
                break;
            case NEGATE:
                if (c=='=') {addCharChangeTo(NEQ_TEST);}
                currToken = tokenPrinter(currToken);
                break;
            case GREATER:
                switch (c)
                {
                    case '>': addCharChangeTo(SHIFT_R); break;
                    case '=': addCharChangeTo(GREATER_EQ);
                    default: currToken = tokenPrinter(currToken);
                }
                break;
            case SHIFT_R:
                if (c=='=') {addCharChangeTo(SHIFT_R_EQ);}
                currToken = tokenPrinter(currToken);
                break;
            case LESS:
                switch (c)
                {
                    case '<': addCharChangeTo(SHIFT_L); break;
                    case '=': addCharChangeTo(LESS_EQ);
                    default: currToken = tokenPrinter(currToken);
                }
                break;
            case SHIFT_L:
                if (c=='=') {addCharChangeTo(SHIFT_L_EQ);}
                currToken = tokenPrinter(currToken);
                break;
            case XOR:
                if (c=='=') {addCharChangeTo(XOR_EQ);}
                currToken = tokenPrinter(currToken);
                break;
            case OR:
                switch (c)
                {
                    case '|': addCharChangeTo(LOG_OR); break;
                    case '=': addCharChangeTo(BIT_OR); break;
                    default: ;
                }
                currToken = tokenPrinter(currToken);
                break;
            case ADD:
                switch (c)
                {
                    case '+': addCharChangeTo(INCRE); break;
                    case '=': addCharChangeTo(ADD_EQ); break;
                    default: ;
                }
                currToken = tokenPrinter(currToken);
                break;
            case DIV:
                if (c=='=') {addCharChangeTo(DIV_EQ);}
                else if(c=='/'){}
                currToken = tokenPrinter(currToken);
                break;
            case AND:
                switch (c)
                {
                    case '&': addCharChangeTo(LOG_AND); break;
                    case '=': addCharChangeTo(BIT_AND); break;
                    default: ;
                }
                currToken = tokenPrinter(currToken);
                break;
            case ASSIGN:
                if (c=='=') {addCharChangeTo(EQ);}
                currToken = tokenPrinter(currToken);
                break;
            case MULTI:
                if (c=='=') {addCharChangeTo(MULTI_EQ);}
                currToken = tokenPrinter(currToken);
                break;
            case MOD:
                if (c=='=') {addCharChangeTo(MOD_EQ);}
                currToken = tokenPrinter(currToken);
                break;
            case DEC_INT:
                if (strlen(currToken.data)==1 && currToken.data[0]=='0')
                {
                    if ((c=='x' || c=='X') && index+1 < inputlen && isxdigit(input[index+1]))
                    {
                        currToken.names = HEX_INT;
                        putdigit = 2;
                    }
                    else if (c>='0' && c<='7')
                    {
                        currToken.names = OCT_INT;
                        putdigit = 1;
                    }
                }
                if (isdigit(c)) putdigit = 1;
                else if (c=='.' && index+1 < inputlen && isdigit(input[index+1]))
                {
                    currToken.names =FLOAT;
                    putdigit = 2;
                }
                else if ((c=='e' || c=='E') && !hasexponent &&
                         index+2 < inputlen && (input[index+1]=='+' || input[index+1]=='-') &&
                         isdigit(input[index+2]))
                {
                    currToken.names =FLOAT;
                    putdigit = 3;
                    hasexponent = 1;
                }
                if (putdigit > 0)
                {
                    strncat(currToken.data, &input[index], putdigit*sizeof(char));
                    index += putdigit;
                    putdigit = 0;
                }
                else currToken = tokenPrinter(currToken);
                break;
            case OCT_INT:
                if (isdigit(c))
                {
                    strncat(currToken.data, &c, sizeof(char));
                    index++;
                    if (c > '7') currToken.names = DEC_INT;
                }
                else if (c=='.' && index+1 < inputlen && isdigit(input[index+1]))
                {
                    strncat(currToken.data, &input[index], 2*sizeof(char));
                    index++;
                    currToken.names = FLOAT;
                }
                else currToken = tokenPrinter(currToken);
                break;
            case HEX_INT:
                if (isxdigit(c))
                {
                    strncat(currToken.data, &c, sizeof(char));
                    index++;
                }
                else currToken = tokenPrinter(currToken);
                break;
            case FLOAT:
                if (isdigit(c))
                {
                    strncat(currToken.data, &c, sizeof(char));
                    index++;
                }
                else if ((c=='e' || c=='E') && !hasexponent &&
                    index+2 < inputlen && (input[index+1]=='+' || input[index+1]=='-') &&
                    isdigit(input[index+2]))
                {
                    hasexponent = 1;
                    strncat(currToken.data, &input[index], 3*sizeof(char));
                    index += 3;
                }
                else
                {
                    currToken = tokenPrinter(currToken);
                    hasexponent = 0;
                }
                break;
            case WORD:
                if (isopt(c) || isspace(c) || !isprint(c)) currToken = tokenPrinter(currToken);
                else
                {
                    strncat(currToken.data, &c, sizeof(char));
                    index++;
                }
                break;
            default:
                currToken.data = (char*) malloc((inputlen - index + 1)*sizeof(char));
                if (currToken.data == NULL) return exit(1);
                strncat(currToken.data, &c, sizeof(char));
                index++;
                if (isalpha(c)) currToken.names = WORD;
                else if (isdigit(c)) currToken.names = DEC_INT;
                else {
                    switch (c) {
                        case '(': printAs(PAREN_L);
                        case ')': printAs(PAREN_R);
                        case '[': printAs(BRAC_L);
                        case ']': printAs(BRAC_R);
                        case '.': printAs(STRUCT_MEM);
                        case ',': printAs(COMMA);
                        case '~': printAs(ONES_COMP);
                        case '?': printAs(COND_T);
                        case ':': printAs(COND_F);
                        case '-': currToken.names = MINUS; break;
                        case '!': currToken.names = NEGATE; break;
                        case '>': currToken.names = GREATER; break;
                        case '<': currToken.names = LESS; break;
                        case '^': currToken.names = XOR; break;
                        case '|': currToken.names = OR; break;
                        case '+': currToken.names = ADD; break;
                        case '/': currToken.names = DIV; break;
                        case '&': currToken.names = AND; break;
                        case '=': currToken.names = ASSIGN; break;
                        case '*': currToken.names = MULTI; break;
                        case '%': currToken.names = MOD; break;
                        default: free(currToken.data); // Treat as whitespace
                    }
                }
        }
    }
    if (currToken.names != NEW) tokenPrinter(currToken); // Print out the last token, if there's any
}


/*
The main function will take one string argument (in argv[1]) where argv[1]
contains the input string that is needed to be tokenized. 

It prints out the token using tokenScanner function from left to right where
each token that is generated will be on a seperate line.
*/

int main(int argc, char **argv) {

    if (argc != 2)
    {
        printf("Provide enough arguments\n");
        return -1;
    }

    if (strlen(argv[1]) == 0)
    {
        printf("Provide a valid string to read from\n");
        return -1;
    }

    int inputlen = (int)strlen(argv[1]);
    char* input = (char*) malloc((inputlen+1) * sizeof(char));
    if (input == NULL) return EXIT_FAILURE;
    strcpy(input, argv[1]);
    Token token;

    tokenScanner(inputlen, input, token);
    return EXIT_SUCCESS;
}
