#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

/*
some grammar rules

chord Cmaj [c3@saw, e3@saw, g3@saw]
play(Cmaj, 2.25, 3)

start -> statementlist
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

SymbolTable *create_symbol_table() {
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (table) {
        table->head = NULL;
    }
    return table;
}

void symbol_table_insert_chord(SymbolTable *table, Chord *chord) {
    SymbolNode *node = (SymbolNode *)malloc(sizeof(SymbolNode));
    if (node) {
        node->identifier = strdup(chord->name);
        node->chord = chord; // Assume the chord has been allocated and set up properly
        node->next = table->head;
        table->head = node;
    }
}

Chord *symbol_table_lookup_chord(SymbolTable *table, const char *identifier) {
    SymbolNode *current = table->head;
    while (current != NULL) {
        if (strcmp(current->identifier, identifier) == 0) {
            return current->chord;
        }
        current = current->next;
    }
    return NULL; // Not found
}

void free_symbol_table(SymbolTable *table) {
    SymbolNode *current = table->head;
    while (current != NULL) {
        SymbolNode *temp = current;
        current = current->next;
        free(temp->identifier);
        if (temp->chord) {
            for (int i = 0; i < temp->chord->noteCount; i++) {
                free(temp->chord->notes[i].name);
                free(temp->chord->notes[i].wave);
            }
            free(temp->chord->notes);
            free(temp->chord);
        }
        free(temp);
    }
}


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

Note *parse_chordnotes(Parser *parser, int *noteCount) {
    int count = 0;
    int capacity = 4;
    Note *notes = malloc(capacity * sizeof(Note));

    do {
        if (peek(parser).type == TOKEN_COMMA) {
            advance(parser); // Consume the comma
        }
        // consume(parser, TOKEN_IDENTIFIER, "Expected note identifier");
        Token noteName = advance(parser);
        consume(parser, TOKEN_OP_AT, " parsechordnotes Expected '@'");
        Token waveName = advance(parser);
        
        if (count >= capacity) {
            capacity *= 2;
            notes = realloc(notes, capacity * sizeof(Note));
        }

        notes[count].name = strdup(noteName.lexeme);
        notes[count].wave = strdup(waveName.lexeme);
        count++;
    } while (peek(parser).type == TOKEN_COMMA);

    *noteCount = count;
    return notes;
}

void parse_chordstatement(Parser *parser, SymbolTable *symbolTable) {
    consume(parser, TOKEN_KEYWORD_CHORD, "Expected 'chord'");
    Token chordName = advance(parser);
    consume(parser, TOKEN_PUNCT_LEFT_SQUARE, "Expected '['");

    int noteCount;
    Note *notes = parse_chordnotes(parser, &noteCount);

    consume(parser, TOKEN_PUNCT_RIGHT_SQUARE, "Expected ']'");

    Chord *chord = malloc(sizeof(Chord));
    chord->name = strdup(chordName.lexeme);
    chord->notes = notes;
    chord->noteCount = noteCount;

    symbol_table_insert_chord(symbolTable, chord);
}

void parse_loopstatement(Parser *parser, SymbolTable *symbolTable) {
    consume(parser, TOKEN_KEYWORD_LOOP, "Expected 'loop'");
    consume(parser, TOKEN_PUNCT_LEFT_PAREN, "Expected '('");

    parse_number(parser);
    consume(parser, TOKEN_COMMA, "Expected ',' after start time");
    parse_number(parser);
    consume(parser, TOKEN_COMMA, "Expected ',' after duration");
    parse_number(parser);

    consume(parser, TOKEN_PUNCT_RIGHT_PAREN, "Expected ')'");

    consume(parser, TOKEN_PUNCT_LEFT_CURLY, "Expected '{'");

    // Parse play statements inside the loop
    while (peek(parser).type != TOKEN_PUNCT_RIGHT_CURLY) {
        if (peek(parser).type == TOKEN_KEYWORD_PLAY) {
            parse_playstatement(parser);
        } else {
            error("Expected 'play' statement inside loop");
        }
    }

    consume(parser, TOKEN_PUNCT_RIGHT_CURLY, "Expected '}'");
}


// void parse_statement(Parser *parser, SymbolTable *symbolTable) {
//     switch (peek(parser).type) {
//         case TOKEN_KEYWORD_PLAY:
//             parse_playstatement(parser);
//             break;
//         case TOKEN_KEYWORD_CHORD:
//             parse_chordstatement(parser, symbolTable);
//             break;
//         default:
//             error("Expected 'play' or 'chord'");
//     }
// }

// void parse_statementlist(Parser *parser, SymbolTable *symbolTable) {
//     while (parser->current < parser->count) {
//         parse_statement(parser, symbolTable);
//     }
// }

// SymbolTable *parse_program(Parser *parser) {
//     SymbolTable *symbolTable = malloc(sizeof(SymbolTable));
//     symbolTable->head = NULL;

//     parse_statementlist(parser, symbolTable);

//     return symbolTable;
// }


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

void print_symbol_table(SymbolTable *table) {
    SymbolNode *current = table->head;
    printf("Symbol Table:\n");
    while (current != NULL) {
        printf("Identifier: %s\n", current->identifier);
        printf("Chord Name: %s\n", current->chord->name);
        printf("Notes:\n");
        for (int i = 0; i < current->chord->noteCount; i++) {
            printf("  Note Name: %s, Waveform: %s\n", current->chord->notes[i].name, current->chord->notes[i].wave);
        }
        printf("\n");
        current = current->next;
    }
}
