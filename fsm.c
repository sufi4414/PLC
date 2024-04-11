// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "tokens.h"

// typedef enum {
//     INITIAL_STATE,
//     PARSE_STATEMENT,
//     PARSE_CHORD_STATEMENT,
//     PARSE_PLAY_STATEMENT,
//     ERROR_STATE
// } ParserState;

// typedef struct {
//     ParserState currentState;
//     char targetLetter;
// } FSM;

// ParserState current_state = INITIAL_STATE;

// // Function to transition to the parse statement state
// void transition_to_parse_statement() {
//     current_state = PARSE_STATEMENT;
// }

// // Function to transition to the parse chord statement state
// void transition_to_parse_chord_statement() {
//     current_state = PARSE_CHORD_STATEMENT;
// }

// // Function to transition to the parse play statement state
// void transition_to_parse_play_statement() {
//     current_state = PARSE_PLAY_STATEMENT;
// }

// // Function to handle errors
// void handle_error(const char *message) {
//     fprintf(stderr, "Error: %s\n", message);
//     current_state = ERROR_STATE;
// }

// // Function to parse individual statements based on the current state
// void parse_statement(Parser *parser, SymbolTable *symbolTable) {
//     switch (current_state) {
//         case PARSE_CHORD_STATEMENT:
//             parse_chord_statement(parser, symbolTable);
//             break;
//         case PARSE_PLAY_STATEMENT:
//             parse_play_statement(parser);
//             break;
//         default:
//             handle_error("Unexpected statement");
//             break;
//     }
// }

// // Function to parse the program using the FSM
// SymbolTable *parse_program(Parser *parser) {
//     SymbolTable *symbolTable = create_symbol_table();
//     while (current_state != ERROR_STATE && parser->current < parser->count) {
//         parse_statement(parser, symbolTable);
//     }
//     if (current_state == ERROR_STATE) {
//         free_symbol_table(symbolTable);
//         return NULL;
//     }
//     return symbolTable;
// }