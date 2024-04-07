
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
    TOKEN_IDENTIFIER_WAV,
    TOKEN_IDENTIFIER_FREQ,
    TOKEN_IDENTIFIER_START,
    TOKEN_IDENTIFIER_STOP,
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


/*funcitons from parser.c */
bool isValidNote(char note);
bool isValidAccidentals(char accidentals);
bool isValidOctave(char octave);
bool isValidWaveform(char *str);

double maptoFreq(char note, int octave, char accidentals);
bool check_play_keyword(Token **token_stream, size_t *index, size_t num_tokens);
bool check_left_paren(Token **token_stream, size_t *index, size_t num_tokens);
bool check_right_paren(Token **token_stream, size_t *index, size_t num_tokens);
bool check_comma(Token **token_stream, size_t *index, size_t num_tokens);
bool check_wav(Token **token_stream, size_t *index, size_t num_tokens);
bool check_note(Token **token_stream, size_t *index, size_t num_tokens);
bool check_start(Token **token_stream, size_t *index, size_t num_tokens);
bool check_stop(Token **token_stream, size_t *index, size_t num_tokens);
bool check_at(Token **token_stream, size_t *index, size_t num_tokens);
bool match_play_statement(Token **token_stream, size_t num_tokens);