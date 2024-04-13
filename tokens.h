#ifndef TOKENS_H
#define TOKENS_H

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
    //TOKEN_PUNCT_END_OF_LINE,
    TOKEN_OP_AT,
    TOKEN_COMMA,
	TOKEN_IDENTIFIER,
	TOKEN_LITERAL_INT,
    TOKEN_LITERAL_FLOAT,
    //TOKEN_LITERAL_CHAR,
    //TOKEN_LITERAL_STRING,
    TOKEN_UNKNOWN
} TokenType;

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

typedef struct {
    char *name; // Note name (e.g., "C4")
    char *wave; // Waveform type (e.g., "saw")
} Note;

typedef struct {
    char *name; // Chord name (e.g., "Cmaj")
    Note *notes; // Array of notes in the chord
    int noteCount; // Number of notes in the chord
} Chord;

typedef struct SymbolNode {
    char *identifier; // Symbol identifier, such as a chord name
    Chord *chord; // Pointer to the chord structure
    struct SymbolNode *next; // Pointer to the next symbol in the table
} SymbolNode;

typedef struct {
    SymbolNode *head; // Pointer to the first symbol in the table
} SymbolTable;

TokenType parse_token_type(int type);
SymbolTable *parse_program(Parser *parser);
void free_symbol_table(SymbolTable *table);
SymbolTable *create_symbol_table(void);
void symbol_table_insert_chord(SymbolTable *table, Chord *chord);
Chord *symbol_table_lookup_chord(SymbolTable *table, const char *identifier);
void free_token_resources(Token *tokens, size_t count);
Token *read_tokens_from_file(const char *filename, size_t *count);
void print_symbol_table(SymbolTable *table);


#endif 
