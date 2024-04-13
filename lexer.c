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
    TOKEN_PUNCT_END_OF_LINE,
    TOKEN_OP_AT,
    TOKEN_COMMA,
	TOKEN_IDENTIFIER,
	TOKEN_LITERAL_INT,
    TOKEN_LITERAL_FLOAT,
    TOKEN_LITERAL_CHAR,
    TOKEN_LITERAL_STRING,
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


// Recognizing keywords using strcmp
// Will now return the corresponding kewyord TokenType that has been
// recognized in case of a match, or TOKEN_UNKNOWN otherwise.
TokenType is_keyword_strcmp(const char *lexeme) {
    const struct {
        const char *lexeme;
        TokenType token_type;
    } keywords[] = {
        {"chord", TOKEN_KEYWORD_CHORD},
        {"play", TOKEN_KEYWORD_PLAY},
        {"loop", TOKEN_KEYWORD_LOOP},
        {NULL, TOKEN_UNKNOWN}
    };

    for (int i = 0; keywords[i].lexeme != NULL; i++) {
        if (strcmp(lexeme, keywords[i].lexeme) == 0) {
            return keywords[i].token_type;
        }
    }
    return TOKEN_UNKNOWN;
}


// Recognizing operators, in the same style as keywords
TokenType is_operator_strcmp(const char *lexeme) {
    const struct {
        const char *lexeme;
        TokenType token_type;
    } operators[] = {
        {"@", TOKEN_OP_AT},
        {NULL, TOKEN_UNKNOWN}
    };
	
    for (int i = 0; operators[i].lexeme != NULL; i++) {
        if (strcmp(lexeme, operators[i].lexeme) == 0) {
            return operators[i].token_type;
        }
    }
    return TOKEN_UNKNOWN;
}


// Recognizing punctuation, in the same style as operators
TokenType is_punctuation_strcmp(const char *lexeme) {
    const struct {
        const char *lexeme;
        TokenType token_type;
    } punctuations[] = {
        {"(", TOKEN_PUNCT_LEFT_PAREN},
        {")", TOKEN_PUNCT_RIGHT_PAREN},
        {",", TOKEN_COMMA},
        {"{", TOKEN_PUNCT_LEFT_CURLY},
        {"}", TOKEN_PUNCT_RIGHT_CURLY},
        {"[", TOKEN_PUNCT_LEFT_SQUARE},
        {"]", TOKEN_PUNCT_RIGHT_SQUARE},
        {";", TOKEN_PUNCT_END_OF_LINE},
        {NULL, TOKEN_UNKNOWN} // Sentinel value
    };

    for (int i = 0; punctuations[i].lexeme != NULL; i++) {
        if (strcmp(lexeme, punctuations[i].lexeme) == 0) {
            return punctuations[i].token_type;
        }
    }
    return TOKEN_UNKNOWN;
}


// Recognizing (unsigned) integer literals using RegEx
bool is_integer_literal_regex(const char *lexeme) {
    const char *pattern = "^(0|[1-9][0-9]*)$";
    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (result != 0) {
        return false;
    }
    result = regexec(&regex, lexeme, 0, NULL, 0);
    regfree(&regex);
    if (result == 0) {
        return true;
    }
    return false;
}


// Recognizing float literals using RegEx
bool is_float_literal_regex(const char *lexeme) {
    const char *pattern = "^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$";
    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (result != 0) {
        return false;
    }
    result = regexec(&regex, lexeme, 0, NULL, 0);
    regfree(&regex);
    if (result == 0) {
        return true;
    }
    return false;
}


// Recognizing char literals using RegEx
bool is_char_literal_regex(const char *lexeme) {
    const char *pattern = "^'(\\.|[^\\'])'$";
    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (result != 0) {
        return false;
    }
    result = regexec(&regex, lexeme, 0, NULL, 0);
    regfree(&regex);
    if (result == 0) {
        return true;
    }
    return false;
}


// Recognizing string literals using RegEx
bool is_string_literal_regex(const char *lexeme) {
    const char *pattern = "^\"(\\.|[^\\\"])*\"$";
    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (result != 0) {
        return false;
    }
    result = regexec(&regex, lexeme, 0, NULL, 0);
    regfree(&regex);
    if (result == 0) {
        return true;
    }
    return false;
}


// Recognizing identifiers using RegEx
bool is_identifier_regex(const char *lexeme) {
    const char *pattern = "^[a-zA-Z][A-Za-z0-9_]+$";
    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (result != 0) {
        return false;
    }
    result = regexec(&regex, lexeme, 0, NULL, 0);
    regfree(&regex);
    if (result == 0) {
        return true;
    }
    return false;

}

int main(int argc, char ** argv) {

    //const char *filename = "test.txt";
    const char *filename = argv[1];
    char *source_code = read_source_code(filename);
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
	
	// A possible maximal munch implementation
    while (current_position < source_code_length) {
		// Current lexeme candidate to be matched
        TokenType token_type = TOKEN_UNKNOWN;
        size_t longest_match = 0;
        char  * matched_lexeme = calloc(source_code_length+2, sizeof(char));
        char * candidate_lexeme = calloc(source_code_length+2, sizeof(char));
		
		// Left to right scan producing candidate lexemes and testing them
        for (size_t i = current_position; i < source_code_length; ++i) {
            memcpy(candidate_lexeme, source_code + current_position, i - current_position + 1);
            TokenType candidate_token_type = TOKEN_UNKNOWN;
            size_t candidate_match_length = i - current_position + 1;
			
            if ((candidate_token_type = is_keyword_strcmp(candidate_lexeme)) != TOKEN_UNKNOWN ||
                (candidate_token_type = is_operator_strcmp(candidate_lexeme)) != TOKEN_UNKNOWN ||
                (candidate_token_type = is_punctuation_strcmp(candidate_lexeme)) != TOKEN_UNKNOWN ||
                (candidate_token_type = is_identifier_regex(candidate_lexeme) ? TOKEN_IDENTIFIER : TOKEN_UNKNOWN) != TOKEN_UNKNOWN ||
                (candidate_token_type = is_integer_literal_regex(candidate_lexeme) ? TOKEN_LITERAL_INT : TOKEN_UNKNOWN) != TOKEN_UNKNOWN ||
                (candidate_token_type = is_float_literal_regex(candidate_lexeme) ? TOKEN_LITERAL_FLOAT : TOKEN_UNKNOWN) != TOKEN_UNKNOWN ||
                (candidate_token_type = is_char_literal_regex(candidate_lexeme) ? TOKEN_LITERAL_CHAR : TOKEN_UNKNOWN) != TOKEN_UNKNOWN ||
                (candidate_token_type = is_string_literal_regex(candidate_lexeme) ? TOKEN_LITERAL_STRING : TOKEN_UNKNOWN) != TOKEN_UNKNOWN) {
					token_type = candidate_token_type;
					longest_match = candidate_match_length;
					strcpy(matched_lexeme, candidate_lexeme);
            }
        }
		
		// After we have found our candidate lexeme, create token
        if (token_type != TOKEN_UNKNOWN) {
        Token *token = create_token(token_type, matched_lexeme, current_line_number);
            token_stream = (Token **)realloc(token_stream, (token_count + 1) * sizeof(Token *));
            token_stream[token_count] = token;
            token_count++;
            printf("Token { type: %d, lexeme: '%s', line: '%d'}\n", token->type, token->lexeme, token->line_number);
            current_position += longest_match;
        } else {
			// Update the line number when encountering a newline character
			if (source_code[current_position] == '\n') {
				current_line_number++;
			}
			// Show error message, unless we have a whitespace character or some sort
            if (source_code[current_position] != ' ' && source_code[current_position] != '\n' &&
                source_code[current_position] != '\t' && source_code[current_position] != '\r') {
                printf("Error: Unrecognized character '%c' at line %zu\n", source_code[current_position], (size_t)current_line_number);
            }
            ++current_position;
        }
    }

    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        fprintf(stderr, "Error opening output file\n");
        return 1;
    }

    // Write token information to the file
    for (size_t i = 0; i < token_count; i++) {
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
    free(source_code);

    return 0;
}
