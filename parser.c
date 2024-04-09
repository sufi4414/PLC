#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "parser.h"

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

TokenType parse_token_type(int type) {
    switch (type) {
        case 0: return TOKEN_KEYWORD_PLAY;
        case 1: return TOKEN_KEYWORD_LOOP;
        case 2: return TOKEN_KEYWORD_CHORD;
        case 3: return TOKEN_PUNCT_LEFT_PAREN;
        case 4: return TOKEN_PUNCT_RIGHT_PAREN;
        case 5: return TOKEN_PUNCT_LEFT_CURLY;
        case 6: return TOKEN_PUNCT_RIGHT_CURLY;
        case 7: return TOKEN_PUNCT_LEFT_SQUARE;
        case 8: return TOKEN_PUNCT_RIGHT_SQUARE;
        case 9: return TOKEN_PUNCT_END_OF_LINE;
        case 10: return TOKEN_OP_AT;
        case 11: return TOKEN_COMMA;
        case 12: return TOKEN_IDENTIFIER;
        case 13: return TOKEN_LITERAL_INT;
        case 14: return TOKEN_LITERAL_FLOAT;
        case 15: return TOKEN_LITERAL_CHAR;
        case 16: return TOKEN_LITERAL_STRING;
        default: return TOKEN_UNKNOWN;
    }
}


/*
some grammar rules

chord Cmaj [c3@saw, e3@saw, g3@saw]
play(Cmaj, 2.25, 3)

program -> statementlist
statementlist -> statement statementlist | ε
statement -> playstatement | chordstatement

playstatement -> TOKEN_KEYWORD_PLAY TOKEN_PUNCT_LEFT_PAREN playargs TOKEN_PUNCT_RIGHT_PAREN
playargs -> TOKEN_IDENTIFIER TOKEN_COMMA number TOKEN_COMMA number
number -> TOKEN_LITERAL_FLOAT | TOKEN_LITERAL_INT


chordstatement -> TOKEN_KEYWORD_CHORD TOKEN_IDENTIFIER TOKEN_PUNCT_LEFT_SQUARE chordnotes TOKEN_PUNCT_RIGHT_SQUARE
chordnotes -> note chordtail
chordtail -> TOKEN_COMMA note chordtail | ε
note -> TOKEN_IDENTIFIER TOKEN_OP_AT TOKEN_IDENTIFIER

*/

typedef struct {
    TokenType type;
    char *lexeme;
    int line_number;
} Token;

typedef struct {
    Token *tokens;
    size_t count;
    size_t current;
} Parser;

void error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(EXIT_FAILURE);
}

Token advance(Parser *parser) {
    if (parser->current < parser->count) {
        return parser->tokens[parser->current++];
    }
    error("Unexpected end of tokenstream");
}

Token peek(Parser *parser) {
    if (parser->current < parser->count) {
        return parser->tokens[parser->current];
    }
    error("Unexpected end of tokenstream");
}

void consume(Parser *parser, TokenType type, const char *message) {
    Token token = peek(parser);
    printf("Consuming: %s\n", token.lexeme);
    if (token.type == type) {
        advance(parser);
    } else {
        error(message);
    }
}


void parse_number(Parser *parser) {
    Token token = peek(parser);
    if (token.type == TOKEN_LITERAL_FLOAT) {
        consume(parser, TOKEN_LITERAL_FLOAT, "Expected float number");
    } else if (token.type == TOKEN_LITERAL_INT) {
        consume(parser, TOKEN_LITERAL_INT, "Expected integer number");
    } else {
        error("Expected number");
    }
}


void parse_note(Parser *parser) {
    consume(parser, TOKEN_IDENTIFIER, "Expected identifier");
    consume(parser, TOKEN_OP_AT, "Expected '@'");
    consume(parser, TOKEN_IDENTIFIER, "Expected identifier");
}

void parse_chord(Parser *parser) {
    consume(parser , TOKEN_IDENTIFIER, "Expected chord identifier");
}

void parse_playargs(Parser *parser) {

    consume(parser, TOKEN_IDENTIFIER, "Expected identifier");

    Token token = peek(parser);
    if (token.type == TOKEN_OP_AT) {
        consume(parser, TOKEN_OP_AT, "Expected '@'");
        consume(parser, TOKEN_IDENTIFIER, "Expected waveform identifier after '@'");
    }

    consume(parser, TOKEN_COMMA, "Expected ',' after note or chord");

    parse_number(parser); 
    consume(parser, TOKEN_COMMA, "Expected ',' after first number");
    parse_number(parser);
}



void parse_playstatement(Parser *parser) {
    consume(parser, TOKEN_KEYWORD_PLAY, "Expected 'play'");
    consume(parser, TOKEN_PUNCT_LEFT_PAREN, "Expected '('");
    parse_playargs(parser);
    consume(parser, TOKEN_PUNCT_RIGHT_PAREN, "Expected ')'");
}

void parse_chordnotes(Parser *parser) {
    parse_note(parser);
    while (peek(parser).type == TOKEN_COMMA) {
        advance(parser); // Consume the comma
        parse_note(parser);
    }
}

void parse_chordstatement(Parser *parser) {
    consume(parser, TOKEN_KEYWORD_CHORD, "Expected 'chord'");
    consume(parser, TOKEN_IDENTIFIER, "Expected identifier");
    consume(parser, TOKEN_PUNCT_LEFT_SQUARE, "Expected '['");
    parse_chordnotes(parser);
    consume(parser, TOKEN_PUNCT_RIGHT_SQUARE, "Expected ']'");
}

void parse_statement(Parser *parser) {
    switch (peek(parser).type) {
        case TOKEN_KEYWORD_PLAY:
            parse_playstatement(parser);
            break;
        case TOKEN_KEYWORD_CHORD:
            parse_chordstatement(parser);
            break;
        default:
            error("Expected 'play' or 'chord'");
    }
}

void parse_statementlist(Parser *parser) {
    while (parser->current < parser->count) {
        parse_statement(parser);
    }
}

void parse_program(Parser *parser) {
    parse_statementlist(parser);
}

Token new_token(TokenType type, const char* lexeme, int line_number) {
    Token token;
    token.type = type;
    token.lexeme = strdup(lexeme);
    token.line_number = line_number;
    return token;
}

// Function to free token resources
void free_token_resources(Token *tokens, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        free(tokens[i].lexeme);
    }
}

Token *read_tokens_from_file(const char *filename, size_t *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file\n");
        exit(EXIT_FAILURE);
    }

    Token *tokens = NULL;
    *count = 0;

    int type;
    char lexeme[256];
    int line_number;
    while (fscanf(file, " Token { type: %d, lexeme: '%255[^']', line: '%d'}", &type, lexeme, &line_number) == 3) {
        tokens = realloc(tokens, (*count + 1) * sizeof(Token));
        tokens[*count] = new_token(parse_token_type(type), lexeme, line_number);
      //  printf("Read token: %s (type %d)\n", tokens[*count].lexeme, tokens[*count].type);
        (*count)++;
    }

    fclose(file);
    return tokens;
}


int main() {

    size_t num_tokens;
    Token *tokens = read_tokens_from_file("output.txt", &num_tokens);

    // Initialize the parser with the token stream
    Parser parser;
    parser.tokens = tokens;
    parser.count = num_tokens;
    parser.current = 0;

    // Now you can parse the program
    parse_program(&parser);

    // Free the allocated resources for tokens
    free_token_resources(tokens, num_tokens);

    return 0;
}