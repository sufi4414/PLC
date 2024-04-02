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
  double frequency; /*for semantics part to reference to frequency table*/
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

/*semantics analysis given that we have alr tokenize the inputs in the above*/
/*in this portion we check the parsenote*/
typedef struct {
    char *noteName;
    double frequencies[9];
} NoteFreq;

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

double maptoFreq(const NoteCfg *cfg){
    int i;
    char nt[3];
    //nt[0] = cfg->note;
    //nt[1] = cfg->accidentals;
    //nt[2] = '\0';
    snprintf(nt, sizeof(nt), "%c%c", cfg->note, cfg->accidentals);
    printf("that value of nt %s\n",nt);
    
    /*finds and assigns the frequencies acccording to cfg note and octave*/
    for (int i = 0; i < sizeof(noteFrequencies) / sizeof(noteFrequencies[0]); i++){
        if (strcmp(noteFrequencies[i].noteName, nt ) == 0){
            return noteFrequencies[i].frequencies[cfg->octave];
        }
    }
    /*if frequency cannot be mapped*/
    return -1.0;
}


TokenType parseNoteCfg(const char *input, NoteCfg *cfg) {
    // Parse the note
    cfg->note = input[0];
    if (!isValidNote(cfg->note)) {
        fprintf(stderr, "Invalid note: %c\n", cfg->note);
        return TOKEN_ERROR;
    }

    /* Parse the accidentals & octaves tgt in the event that there is no accidentals eg. a4*/
    if(input[1] >='0' && input[1]<= '9'){ 
        cfg->accidentals = '\0'; /*assign accidental to \0 in the event no s/b */
        cfg->octave = input[1] - '0';
        if(!isValidAccidentals(cfg->accidentals) || !isValidOctave(cfg->octave)){
            return TOKEN_ERROR;
        }
    }else{

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
            cfg->frequency = maptoFreq(cfg);
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
    printf("Frequency: %f\n", cfg->frequency);

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
    const char *input = "cb4@triangle";
    NoteCfg cfg;

    TokenType token = parseNoteCfg(input, &cfg);

    if (token == TOKEN_NOTE) {
        printf("Parsed a valid note:\n");
        printNoteCfg(&cfg);
    } else if (token == TOKEN_WAVEFORM) {
        printf("Parsed a valid waveform: %s\n", input);
        printNoteCfg(&cfg);
    } else {
        printf("Failed to parse: %s\n", input);
    }

    return 0;
}