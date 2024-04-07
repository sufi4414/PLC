#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "main.h"

/*start of bootleg parser*/
/*functions to process the token stream*/
bool isValidNote(char note) {
 return note >= 'a' && note <= 'g';
}

bool isValidAccidentals(char accidentals) {
    return accidentals == 's' || accidentals == 'b' || accidentals == '\0';
}

bool isValidOctave(char octave) {
    return octave >= '0' && octave <= '9';
}

bool isValidWaveform(char *str) {
    if(strcmp(str, "sin") == 0 || strcmp(str, "triangle") == 0 ||
           strcmp(str, "saw") == 0 || strcmp(str, "square") == 0 || strcmp(str, "kick") == 0){
            return strcmp(str, "sin") == 0 || strcmp(str, "triangle") == 0 ||
           strcmp(str, "saw") == 0 || strcmp(str, "square") == 0 || strcmp(str, "kick") == 0;
           }
    
    return strcmp(str, "sin") == 0; /*default return sine wave*/ 
    
}

typedef struct {
    const char* noteName;  // Note name (e.g., "c", "cs", "d", etc.)
    double frequencies[9]; // Array of frequencies for each octave
} NoteFreq;

// Define the note frequencies table as a global variable
NoteFreq noteFrequencies[] = {
    {"c", {16.35, 32.7, 65.41, 130.81, 261.63, 523.25, 1046.5, 2093, 4186}},
    {"cs", {17.32, 34.65, 69.3, 138.59, 277.18, 554.37, 1108.73, 2217.46, 4434.92}},
    {"d", {18.35, 36.71, 73.42, 146.83, 293.66, 587.33, 1174.66, 2349.32, 4698.63}},
    {"ds", {19.45, 38.89, 77.78, 155.56, 311.13, 622.25, 1244.51, 2489, 4978}},
    {"e", {20.6, 41.2, 82.41, 164.81, 329.63, 659.25, 1318.51, 2637, 5274}},
    {"f", {21.83, 43.65, 87.31, 174.61, 349.23, 698.46, 1396.91, 2793.83, 5587.65}},
    {"fs", {23.12, 46.25, 92.5, 185, 369.99, 739.99, 1479.98, 2959.96, 5919.91}},
    {"g", {24.5, 49, 98, 196, 392, 783.99, 1567.98, 3135.96, 6271.93}},
    {"gs", {25.96, 51.91, 103.83, 207.65, 415.3, 830.61, 1661.22, 3322.44, 6644.88}},
    {"a", {27.5, 55, 110, 220, 440, 880, 1760, 3520, 7040}},
    {"as", {29.14, 58.27, 116.54, 233.08, 466.16, 932.33, 1864.66, 3729.31, 7458.62}},
    {"b", {30.87, 61.74, 123.47, 246.94, 493.88, 987.77, 1975.53, 3951, 7902.13}}
};



double maptoFreq(char note, int octave, char accidentals){
    int i;
    char nt[3];

    snprintf(nt, sizeof(nt), "%c%c", note, accidentals);
    /*finds and assigns the frequencies acccording to cfg note and octave*/
    for (i = 0; i < sizeof(noteFrequencies) / sizeof(noteFrequencies[0]); i++){
        if (strcmp(noteFrequencies[i].noteName, nt ) == 0){
            return noteFrequencies[i].frequencies[octave];
        }
    }
    /*if frequency cannot be mapped*/
    return -1.0;
}

bool check_play_keyword(Token **token_stream, size_t *index, size_t num_tokens) {
    if (*index == 0 && token_stream[*index]->type == TOKEN_KEYWORD_PLAY) {
        (*index)++;
        return true;
    } else {
        printf("Expect 'play' keyword\n");
        return false;
    }
}

bool check_left_paren(Token **token_stream, size_t *index, size_t num_tokens) {
    if (*index < num_tokens && token_stream[*index]->type == TOKEN_PUNCT_LEFT_PAREN) {
        (*index)++;
        return true;
    } else {
        printf("Expect '(' token \n");
        return false;
    }
}

bool check_right_paren(Token **token_stream, size_t *index, size_t num_tokens) {
    if (*index < num_tokens && token_stream[*index]->type == TOKEN_PUNCT_RIGHT_PAREN) {
        (*index)++;
        return true;
    } else {
        printf("Expect ')' token \n");
        return false;
    }
}

bool check_comma(Token **token_stream, size_t *index, size_t num_tokens) {
    if (*index < num_tokens && token_stream[*index]->type == TOKEN_COMMA) {
        (*index)++;
        return true;
    } else {
        printf("Expect ',' token \n");
        return false;
    }
}

bool check_wav(Token **token_stream, size_t *index, size_t num_tokens) {
    if (*index < num_tokens && token_stream[*index]->type == TOKEN_IDENTIFIER) {
        if(isValidWaveform(token_stream[*index]->lexeme )){
            token_stream[*index]->type = TOKEN_IDENTIFIER_WAV;
            (*index)++;
        return true;}
    } else {
        printf("Expect valid wave type \n");
        return false;
    }
}

bool check_note(Token **token_stream, size_t *index, size_t num_tokens) {
    char nt[4];
    int octave;
    strncpy(nt, token_stream[*index]->lexeme, sizeof(token_stream[*index]->lexeme));
        nt[sizeof(nt)-1] = '\0';
        double freq;
        char frequencyStr[20];
    if (*index < num_tokens && token_stream[*index]->type == TOKEN_IDENTIFIER){
           
        /*check note validity*/
        if(isValidNote(nt[0]) && isValidOctave(nt[1]) && isValidAccidentals(nt[2])){
            octave = nt[1] -'0';
            freq =  maptoFreq( nt[0],  octave,  nt[2]);
            snprintf(frequencyStr, sizeof(token_stream[*index]->lexeme), "%.2f", freq); /*convert freq back to string*/
            strncpy(token_stream[*index]->lexeme,frequencyStr, sizeof(token_stream[*index]->lexeme));
            token_stream[*index]->lexeme[sizeof(token_stream[*index]->lexeme) - 1] = '\0';
            token_stream[*index]->type = TOKEN_IDENTIFIER_FREQ;
            (*index)++;/*move on to next token*/
            return true;}
        }else{
                printf("Expect valid wave type \n");
                return false;
            }
}

bool check_start(Token **token_stream, size_t *index, size_t num_tokens) {
    if (*index < num_tokens && token_stream[*index]->type == TOKEN_LITERAL_INT) {
        token_stream[*index]->type == TOKEN_IDENTIFIER_START;
        (*index)++;
        return true;
    } else {
        printf("Expect valid start duration \n");
        return false;
    }
}

bool check_stop(Token **token_stream, size_t *index, size_t num_tokens) {
    if (*index < num_tokens && token_stream[*index]->type == TOKEN_LITERAL_INT) {
        token_stream[*index]->type == TOKEN_IDENTIFIER_STOP;
        (*index)++;
        return true;
    } else {
        printf("Expect valid stop duration \n");
        return false;
    }
}
bool check_at(Token **token_stream, size_t *index, size_t num_tokens) {
    if (*index < num_tokens && token_stream[*index]->type == TOKEN_OP_AT) {
        (*index)++;
        return true;
    } else {
        printf("Expect '@' token \n");
        return false;
    }
}

bool match_play_statement(Token **token_stream, size_t num_tokens){
    size_t i = 0;
    if(!check_play_keyword){return false;}
    if(!check_left_paren){return false;}
    if(!check_note){return false;}
    if(!check_at){return false;}
    if(!check_wav){return false;}
    if(!check_comma){return false;}
    if(!check_start){return false;}
    if(!check_comma){return false;}
    if(!check_stop){return false;}
    if(!check_right_paren){return false;}

    return true;

}


