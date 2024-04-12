#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"


/* Uncomment // after adding loop function in the parser */
typedef enum {
    INITIAL_STATE,
    PARSE_CHORD_STATEMENT,
    PARSE_PLAY_STATEMENT,
    PARSE_LOOP_STATEMENT, // Uncomment after loop function is included
    ERROR_STATE
} ParserState;

typedef struct {
    ParserState currentState;
    char targetLetter;
} FSM;

void parse_playstatement(Parser *parser);
void parse_chordstatement(Parser *parser, SymbolTable *symbolTable);
void parse_loopstatement(Parser *parser, SymbolTable *symbolTable);
// Add the loop function here
SymbolTable *create_symbol_table();
Token peek(Parser *parser);

ParserState current_state = INITIAL_STATE;

void handle_error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
    current_state = ERROR_STATE;
}

void parse_statement(Parser *parser, SymbolTable *symbolTable) {
    switch (current_state) {
        case INITIAL_STATE:
            printf("Parser state: %u\n",peek(parser).type);
            if(peek(parser).type == 0){
                current_state = PARSE_PLAY_STATEMENT;
            }
            else if (peek(parser).type == 2)
            {
                current_state = PARSE_CHORD_STATEMENT;
            }
            // Uncomment after loop is added
            else if (peek(parser).type == 1)
            {
                current_state = PARSE_LOOP_STATEMENT;
            }
            break;
        case PARSE_CHORD_STATEMENT:
            parse_chordstatement(parser, symbolTable);
            current_state = INITIAL_STATE; 
            break;
        case PARSE_PLAY_STATEMENT:
            parse_playstatement(parser);
            current_state = INITIAL_STATE;
            break;
        case PARSE_LOOP_STATEMENT:
            // Add the loop function
            parse_loopstatement(parser, symbolTable);
            current_state = INITIAL_STATE;
            break;
        default:
            handle_error("Syntax Error");
            current_state = ERROR_STATE;
            break;
    }
}

SymbolTable *parse_program(Parser *parser) {
    SymbolTable *symbolTable = create_symbol_table();
    while (current_state != ERROR_STATE && parser->current < parser->count) {
        parse_statement(parser, symbolTable);
    }
    if (current_state == ERROR_STATE) {
        free_symbol_table(symbolTable);
        return NULL;
    }
    return symbolTable;
}