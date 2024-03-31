#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
  WAVE_SIN,
  WAVE_TRIANGLE,
  WAVE_SAW,
  WAVE_SQUARE,
  WAVE_KICK
} WaveForm;

typedef enum {
    TOKEN_NOTE,
    TOKEN_ACCIDENTALS,
    TOKEN_OCTAVE,
    TOKEN_WAVEFORM,
    TOKEN_ERROR
} TokenType;

typedef struct {
  char note;
  char accidentals; // 's' for sharp, 'b' for flat, '\0' for natural
  int octave;
  WaveForm waveform;
} NoteCfg;

bool isValidNote(char note) {
 return note >= 'a' && note <= 'g';
}

bool isValidAccidentals(char accidentals) {
    return accidentals == 's' || accidentals == 'b' || accidentals == '\0';
}

bool isValidOctave(int octave) {
    return octave >= 0 && octave <= 9;
}

bool isValidWaveform(const char *str) {
    return strcmp(str, "sin") == 0 || strcmp(str, "triangle") == 0 ||
           strcmp(str, "saw") == 0 || strcmp(str, "square") == 0 || strcmp(str, "kick") == 0;
}

TokenType parseNoteCfg(const char *input, NoteCfg *cfg) {
    // Parse the note
    cfg->note = input[0];
    if (!isValidNote(cfg->note)) {
        fprintf(stderr, "Invalid note: %c\n", cfg->note);
        return TOKEN_ERROR;
    }

    // Parse the accidentals
    cfg->accidentals = input[1];
    if (!isValidAccidentals(cfg->accidentals)) {
        fprintf(stderr, "Invalid accidentals: %c\n", cfg->accidentals);
        return TOKEN_ERROR;
    }

    // Parse the octave
    cfg->octave = input[2] - '0';
    if (!isValidOctave(cfg->octave)) {
        fprintf(stderr, "Invalid octave: %d\n", cfg->octave);
        return TOKEN_ERROR;
    }

    // Parse the waveform
    const char *waveform_str = strstr(input, "@");
    if (waveform_str != NULL) {
        waveform_str++; // Move past the '@' character
        if (isValidWaveform(waveform_str)) {
            if (strcmp(waveform_str, "sin") == 0) {
                cfg->waveform = WAVE_SIN;
            } else if (strcmp(waveform_str, "triangle") == 0) {
                cfg->waveform = WAVE_TRIANGLE;
            } else if (strcmp(waveform_str, "saw") == 0) {
                cfg->waveform = WAVE_SAW;
            } else if (strcmp(waveform_str, "square") == 0) {
                cfg->waveform = WAVE_SQUARE;
            } else if (strcmp(waveform_str, "kick") == 0) {
                cfg->waveform = WAVE_KICK;
            }
            return TOKEN_WAVEFORM;
        } else {
            fprintf(stderr, "Invalid waveform: %s\n", waveform_str);
            return TOKEN_ERROR;
        }
    } else {
        // If no waveform provided, default to sine wave
        cfg->waveform = WAVE_SIN;
    }

    return TOKEN_NOTE; // Return token type for note
}

void printNoteCfg(const NoteCfg *cfg) {
    printf("Note: %c\n", cfg->note);
    printf("Accidentals: %c\n", cfg->accidentals);
    printf("Octave: %d\n", cfg->octave);

    const char *waveform_str;
    switch (cfg->waveform) {
        case WAVE_SIN:
            waveform_str = "Sine";
            break;
        case WAVE_TRIANGLE:
            waveform_str = "Triangle";
            break;
        case WAVE_SAW:
            waveform_str = "Sawtooth";
            break;
        case WAVE_SQUARE:
            waveform_str = "Square";
            break;
        case WAVE_KICK:
            waveform_str = "Kick";
            break;
    }
    printf("Waveform: %s\n", waveform_str);
}

int main() {
    const char *input = "cs4@triangle";
    NoteCfg cfg;

    TokenType token = parseNoteCfg(input, &cfg);

    if (token == TOKEN_NOTE) {
        printf("Parsed a valid note:\n");
        printNoteCfg(&cfg);
    } else if (token == TOKEN_WAVEFORM) {
        printf("Parsed a valid waveform: %s\n", input);
    } else {
        printf("Failed to parse: %s\n", input);
    }

    return 0;
}