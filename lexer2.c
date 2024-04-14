#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

// Update the TokenType enumeration
// We order tokens following the priority list we have defined
// Keywords > Operators, Punctuations > Identifiers > Literals > Unknown
// This will be important later on.
typedef enum {
    TOKEN_KEYWORD_PLAY,
	TOKEN_KEYWORD_LOOP,
	TOKEN_KEYWORD_CHORD,
    TOKEN_PUNCT_LEFT_PAREN,
    TOKEN_PUNCT_RIGHT_PAREN,
    TOKEN_PUNCT_LEFT_CURLY,
    TOKEN_PUNCT_RIGHT_CURLY,
    TOKEN_PUNCT_LEFT_SQUARE,
    TOKEN_PUNCT_RIGHT_SQUARE,
    TOKEN_OP_AT,
    TOKEN_COMMA,
	TOKEN_IDENTIFIER,
	TOKEN_LITERAL_INT,
    TOKEN_LITERAL_FLOAT,
    TOKEN_UNKNOWN
} TokenType;


// Our Token object
typedef struct {
    TokenType type;
    char *lexeme;
	int line_number;
} Token;


// Constructor for the Token struct
Token *create_token(TokenType type, const char *lexeme, int line_number) {
    Token *token = (Token *)malloc(sizeof(Token));
    token->type = type;
    token->lexeme = strdup(lexeme);
	token->line_number = line_number;
    return token;
}


// Destructor for the Token struct
void free_token(Token *token) {
    free(token->lexeme);
    free(token);
}


// Read source code
char* read_source_code(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);  
    long file_size = ftell(file);
    rewind(file);
    char *source_code = (char *)malloc((file_size + 1) * sizeof(char));
    if (source_code == NULL) {
        fprintf(stderr, "Error allocating memory for source code\n");
        exit(EXIT_FAILURE);
    }
    size_t read_size = fread(source_code, sizeof(char), file_size, file);
    source_code[read_size] = '\0';
    fclose(file);
    return source_code;
}


// regex matcher
void regex_matcher(char ** ptr, Token *** token_stream, size_t *token_count, int *current_line_number, regex_t regex[]){
        // Match regular expressions
        regmatch_t match;
        int matched = 0;
        for (int i = 0; i < 14; i++) {
            if (regexec(&regex[i], *ptr, 1, &match, 0) == 0) {
                // Create token based on match
                char lexeme[256] = {0};
                strncpy(lexeme, *ptr + match.rm_so, match.rm_eo - match.rm_so);
                lexeme[match.rm_eo - match.rm_so] = '\0';
                TokenType type;
                matched = 1;
                switch (i) {
                    case 0: type = TOKEN_KEYWORD_PLAY; break;
                    case 1: type = TOKEN_KEYWORD_LOOP; break;
                    case 2: type = TOKEN_KEYWORD_CHORD; break;
                    case 3: type = TOKEN_PUNCT_LEFT_PAREN; break;
                    case 4: type = TOKEN_PUNCT_RIGHT_PAREN; break;
                    case 5: type = TOKEN_PUNCT_LEFT_CURLY; break;
                    case 6: type = TOKEN_PUNCT_RIGHT_CURLY; break;
                    case 7: type = TOKEN_PUNCT_LEFT_SQUARE; break;
                    case 8: type = TOKEN_PUNCT_RIGHT_SQUARE; break;
                    case 9: type = TOKEN_OP_AT; break;
                    case 10: type = TOKEN_COMMA; break;
                    case 11: type = TOKEN_LITERAL_FLOAT; break;
                    case 12: type = TOKEN_LITERAL_INT; break;
                    case 13: type = TOKEN_IDENTIFIER; break;
                    default: type = TOKEN_UNKNOWN;
                }
                /*if(type == TOKEN_UNKNOWN){
                  printf("unknown token\n");
                  (*ptr++);break;}*/
                Token *token = create_token(type, lexeme, *current_line_number);
                //printf("%s\n",lexeme);
                (*token_stream) = realloc(*token_stream, (*token_count + 1) * sizeof(Token *));
                (*token_stream)[(*token_count)++] = token;
                (*ptr) = &(*ptr)[match.rm_eo];
                break;
            }
            //else {printf("in the else\n");}
        }
        if (**ptr == '\n') {
            current_line_number++;
        }
        //printf("%s\n", *ptr);
        if (matched == 0){(*ptr) = (*ptr) + 1;}
}

int main(int argc, char ** argv){
    const char *filename = argv[1];
    char *source_code = read_source_code(filename);
    char *source_code_original = source_code;
    if (source_code == NULL) {
        fprintf(stderr, "Error reading source code from '%s'\n", filename);
        return 1;
    }

	// Create tokens stream
    Token **token_stream = NULL;
    size_t token_count = 0;
	
	// Prepare maximal munch tracking variables
    size_t source_code_length = strlen(source_code);
    size_t current_position = 0;
	  int current_line_number = 1;



/*const char *patterns[] = {
        "^play",
        "^loop",
        "^chord",
        "^\\(",
        "^\\)",
        "^\\{",
        "^\\}",
        "^\\[",
        "^\\]",
        "^@",
        "^,",
        "^[a-zA-Z_][a-zA-Z0-9_]*"
        "^[0-9]+",
        "^[0-9]+\\.[0-9]+(e[+-]?[0-9]+)?",
    };*/



const char *patterns[] = {
        "^play",
        "^loop",
        "^chord",
        "^\\(",
        "^\\)",
        "^\\{",
        "^\\}",
        "^\\[",
        "^\\]",
        "^@",
        "^,",
        "^[0-9]+\\.[0-9]+(e[+-]?[0-9]+)?",
        "^[0-9]+",
        "^[a-zA-Z_][a-zA-Z0-9_]*"
    };


// Number of tokens
    size_t num_patterns = sizeof(patterns) / sizeof(patterns[0]);

    // Compile regular expressions
    regex_t regex[num_patterns];
    for (size_t i = 0; i < num_patterns; ++i) {
        if (regcomp(&regex[i], patterns[i], REG_EXTENDED) != 0) {
            fprintf(stderr, "Failed to compile regex pattern: %s\n", patterns[i]);
            exit(EXIT_FAILURE);
        }
    }
    
    printf("start matching\n");
    while(strlen(source_code) > 1){
        regex_matcher(&source_code, &token_stream, &token_count, &current_line_number, regex);
        //printf("%ld\n", strlen(source_code));
    }
	
    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        fprintf(stderr, "Error opening output file\n");
        return 1;
    }

    // Write token information to the file
    //printf("tokenS!!!!\n");
    for (size_t i = 0; i < token_count; i++) {

        //printf("%s", token_stream[i]->lexeme);
        fprintf(output_file, "Token { type: %d, lexeme: '%s', line: '%d'}\n", 
            token_stream[i]->type, token_stream[i]->lexeme, token_stream[i]->line_number);
    }

    // Close the output file
    fclose(output_file);
	
	// Free tokens stream
    
    for (size_t i = 0; i < token_count; i++) {
        free_token(token_stream[i]);
    }
    free(token_stream);
	
	// Free source code
    free(source_code_original);

    return 0;
}
