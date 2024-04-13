#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

typedef struct QueueNode {
    Token token;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
} Queue;

int is_valid_waveform(char *wave) {
    char *valid_waveforms[] = {"sin", "triangle", "saw", "square", "kick"}; //TODO add supersaw
    for (int i = 0; i < sizeof(valid_waveforms) / sizeof(valid_waveforms[0]); i++) {
        if (strcmp(wave, valid_waveforms[i]) == 0) {
            return 1; // valid
        }
    }
    return 0; // invalid
}

void queue_enqueue(Queue *queue, Token token) {
    QueueNode *newNode = (QueueNode *)malloc(sizeof(QueueNode));
    newNode->token = token;
    newNode->next = NULL;

    if (queue->rear == NULL) {
        queue->front = queue->rear = newNode;
        return;
    }

    queue->rear->next = newNode;
    queue->rear = newNode;
}

Token queue_dequeue(Queue *queue) {
    if (queue->front == NULL) {
        fprintf(stderr, "Queue underflow\n");
        exit(EXIT_FAILURE);
    }

    QueueNode *temp = queue->front;
    Token token = temp->token;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    free(temp);
    return token;
}

int queue_is_empty(Queue *queue) {
    return queue->front == NULL;
}

void free_queue(Queue *queue) {
    while (!queue_is_empty(queue)) {
        queue_dequeue(queue); 
    }
}

void print_queue(Queue *queue) {
    QueueNode *current = queue->front;

    printf("Queue contents:\n");

    while (current != NULL) {
        printf("Token: %s, Type: %d, Line: %d\n", current->token.lexeme, current->token.type, current->token.line_number);
        current = current->next;
    }
}



void read_tokens_and_enqueue(const char *filename, Queue *queue) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file\n");
        exit(EXIT_FAILURE);
    }

    int type;
    char lexeme[256];
    int line_number;
    while (fscanf(file, " Token { type: %d, lexeme: '%255[^']', line: '%d'}", &type, lexeme, &line_number) == 3) {
        Token token;
        token.type = parse_token_type(type);
        token.lexeme = strdup(lexeme);
        token.line_number = line_number;
        queue_enqueue(queue, token);
    }

    fclose(file);
}
//converting note to freq
typedef struct {
    char *noteName;
    double frequencies[9];
} NoteFreq;

// note freqs with flats also
NoteFreq noteFrequencies[] = {
    {"c", {16.35, 32.7, 65.41, 130.81, 261.63, 523.25, 1046.5, 2093, 4186}},
    {"cs", {17.32, 34.65, 69.3, 138.59, 277.18, 554.37, 1108.73, 2217.46, 4434.92}},
    {"db", {17.32, 34.65, 69.3, 138.59, 277.18, 554.37, 1108.73, 2217.46, 4434.92}},
    {"d", {18.35, 36.71, 73.42, 146.83, 293.66, 587.33, 1174.66, 2349.32, 4698.63}},
    {"ds", {19.45, 38.89, 77.78, 155.56, 311.13, 622.25, 1244.51, 2489, 4978}},
    {"eb", {19.45, 38.89, 77.78, 155.56, 311.13, 622.25, 1244.51, 2489, 4978}},
    {"e", {20.6, 41.2, 82.41, 164.81, 329.63, 659.25, 1318.51, 2637, 5274}},
    {"f", {21.83, 43.65, 87.31, 174.61, 349.23, 698.46, 1396.91, 2793.83, 5587.65}},
    {"fs", {23.12, 46.25, 92.5, 185, 369.99, 739.99, 1479.98, 2959.96, 5919.91}},
    {"gb", {23.12, 46.25, 92.5, 185, 369.99, 739.99, 1479.98, 2959.96, 5919.91}},
    {"g", {24.5, 49, 98, 196, 392, 783.99, 1567.98, 3135.96, 6271.93}},
    {"gs", {25.96, 51.91, 103.83, 207.65, 415.3, 830.61, 1661.22, 3322.44, 6644.88}},
    {"ab", {25.96, 51.91, 103.83, 207.65, 415.3, 830.61, 1661.22, 3322.44, 6644.88}},
    {"a", {27.5, 55, 110, 220, 440, 880, 1760, 3520, 7040}},
    {"as", {29.14, 58.27, 116.54, 233.08, 466.16, 932.33, 1864.66, 3729.31, 7458.62}},
    {"bb", {29.14, 58.27, 116.54, 233.08, 466.16, 932.33, 1864.66, 3729.31, 7458.62}},
    {"b", {30.87, 61.74, 123.47, 246.94, 493.88, 987.77, 1975.53, 3951, 7902.13}}
};

// Function to convert note name to frequency
double note_to_frequency( char *noteToken) {
    // Implementation of note to frequency conversion goes here
    // You can use a lookup table or a mathematical formula to convert note names to frequencies
    // Extract the note name and octave from the note token
    char noteName[3];
    int octave;
    if (strlen(noteToken) == 3) {
        strncpy(noteName, noteToken, 2);
        noteName[2] = '\0';
        octave = atoi(&noteToken[2]);
    } else {
        noteName[0] = noteToken[0];
        noteName[1] = '\0';
        octave = atoi(&noteToken[1]);
    }
    // Find the corresponding note in the array
    for (int i = 0; i < sizeof(noteFrequencies) / sizeof(noteFrequencies[0]); i++) {
        if (strcmp(noteFrequencies[i].noteName, noteName) == 0) {
            // Return the frequency of the note at the specified octave
            return noteFrequencies[i].frequencies[octave];
        }
    }
    // If note not found, return -1 or handle the error as needed
    return -1.0;

}

void handle_play( Queue *queue, FILE *csvFile, SymbolTable *symbolTable) {
    queue_dequeue(queue); // Skip '(' assuming correct syntax

    Token identifierToken = queue_dequeue(queue); // Could be a note or a chord identifier

    // Check if identifier is a chord in the symbol table
    Chord *chord = symbol_table_lookup_chord(symbolTable, identifierToken.lexeme);
    if (chord) {
        queue_dequeue(queue); // Skip ','
        Token firstNumberToken = queue_dequeue(queue); // First number
        
        queue_dequeue(queue); // Skip ','
        Token secondNumberToken = queue_dequeue(queue); // Second number

        if(atof(firstNumberToken.lexeme) > atof(secondNumberToken.lexeme)){
            fprintf(stderr, "Error: Start time is greater than end time\n");
            exit(1); 
        }

        // It's a chord, handle each note in the chord
        for (int i = 0; i < chord->noteCount; i++) {
            //fprintf(csvFile, "%s, ", chord->notes[i].name);

            if (!is_valid_waveform(chord->notes[i].wave)) {
                fprintf(stderr, "Error: Invalid waveform %s\n", chord->notes[i].wave);
                exit(1);
            }

            double frequency = note_to_frequency(chord->notes[i].name);
            fprintf(csvFile, "%.2f, ", frequency);
            fprintf(csvFile, "%s, %s, %s\n", firstNumberToken.lexeme, secondNumberToken.lexeme, chord->notes[i].wave);
        }
        
        queue_dequeue(queue); // Skip ')' at the end, assuming correct syntax
    } else {
        // Assume it's a note if not found as a chord
        //fprintf(csvFile, "%s, ", identifierToken.lexeme);
        //adding the frequency here
        double frequency = note_to_frequency(identifierToken.lexeme);
        fprintf(csvFile, "%.2f, ", frequency);

        queue_dequeue(queue); // Skip '@'

        Token waveToken = queue_dequeue(queue);
        if (!is_valid_waveform(waveToken.lexeme)) {
            fprintf(stderr, "Error: Invalid waveform %s\n", waveToken.lexeme);
            exit(1);
        }

        queue_dequeue(queue); // Skip ','

        Token firstNumberToken = queue_dequeue(queue); // First number
        queue_dequeue(queue); // Skip ','

        Token secondNumberToken = queue_dequeue(queue); // Second number
        queue_dequeue(queue); // Skip ')' at the end, assuming correct syntax

        //if (strcmp(firstNumberToken.lexeme, secondNumberToken.lexeme) > 0) {
        if(atof(firstNumberToken.lexeme) > atof(secondNumberToken.lexeme)){
            fprintf(stderr, "Error: Start time is greater than end time\n");
            exit(1); // Stop the program due to error
        }
        // Continue writing the rest of the data to CSV
        fprintf(csvFile, "%s, %s, %s\n", firstNumberToken.lexeme, secondNumberToken.lexeme, waveToken.lexeme);
    }
}

void handle_play_offset(Queue *queue, FILE *csvFile, SymbolTable *symbolTable, float startTime, float offset, int repetitions) {
    queue_dequeue(queue); //skip "("
    Token identifierToken = queue_dequeue(queue); // Could be a note or a chord identifier
    Chord *chord = symbol_table_lookup_chord(symbolTable, identifierToken.lexeme);
    if (chord) {
        queue_dequeue(queue); // Skip ','
        Token firstNumberToken = queue_dequeue(queue); // First number
        
        queue_dequeue(queue); // Skip ','
        Token secondNumberToken = queue_dequeue(queue); // Second number

        //if (strcmp(firstNumberToken.lexeme, secondNumberToken.lexeme) > 0) {
        if(atof(firstNumberToken.lexeme) > atof(secondNumberToken.lexeme)){
            fprintf(stderr, "Error: Start time is greater than end time(handplayoffset chord)\n");
            exit(1); 
        }
        for (int i = 0 ; i < repetitions ; i++ ){
        float start = atof(firstNumberToken.lexeme) + startTime + i * offset;
        float end = atof(secondNumberToken.lexeme) + startTime + i * offset;
        // It's a chord, handle each note in the chord
            for (int j = 0; j < chord->noteCount; j++) {
                if (!is_valid_waveform(chord->notes[j].wave)) {
                    fprintf(stderr, "Error: Invalid waveform %s\n", chord->notes[j].wave);
                    exit(1);
                }

                double frequency = note_to_frequency(chord->notes[j].name);
                fprintf(csvFile, "%.2f, ", frequency);
                fprintf(csvFile, "%.2f, ",  start );
                fprintf(csvFile, "%.2f, ",  end );
                fprintf(csvFile, "%s\n", chord->notes[j].wave);
        } }
        
        queue_dequeue(queue); // Skip ')' at the end, assuming correct syntax
    } else {
        // Assume it's a note if not found as a chord
        double frequency = note_to_frequency(identifierToken.lexeme);

        queue_dequeue(queue); // Skip '@'

        Token waveToken = queue_dequeue(queue);
        if (!is_valid_waveform(waveToken.lexeme)) {
            fprintf(stderr, "Error: Invalid waveform %s\n", waveToken.lexeme);
            exit(1);
        }

        queue_dequeue(queue); // Skip ','

        Token firstNumberToken = queue_dequeue(queue); // First number
        printf("firstNumberToken: %s\n", firstNumberToken.lexeme);
        queue_dequeue(queue); // Skip ','

        Token secondNumberToken = queue_dequeue(queue); // Second number
        printf("secondNumberToken: %s\n", secondNumberToken.lexeme);
        

        //if (strcmp(firstNumberToken.lexeme, secondNumberToken.lexeme) > 0) {
        if(atof(firstNumberToken.lexeme) > atof(secondNumberToken.lexeme)){
            fprintf(stderr, "Error: Start time is greater than end time(handleplayoffset note)\n");
            exit(1); // Stop the program due to error
        }
        // Continue writing the rest of the data to CSV
        for (int i = 0 ; i < repetitions ; i++ ){
            fprintf(csvFile, "%.2f, ", frequency);
            fprintf(csvFile, "%.2f, ",  atof(firstNumberToken.lexeme) + startTime + i * offset);
            fprintf(csvFile, "%.2f, ",  atof(secondNumberToken.lexeme) + startTime + i * offset);
            fprintf(csvFile, "%s\n", waveToken.lexeme);
        }
        queue_dequeue(queue); // Skip ')' at the end, assuming correct syntax
    }
    
    }

void handle_loop(Queue *queue, FILE *csvFile,SymbolTable *symbolTable) {
    queue_dequeue(queue); // Skip '(' assuming correct syntax
    Token startTimeToken = queue_dequeue(queue);
    queue_dequeue(queue); // Skip ','
    Token durationToken = queue_dequeue(queue);
    queue_dequeue(queue); // Skip ','
    Token repetitionsToken = queue_dequeue(queue);
    queue_dequeue(queue); // Skip ')' assuming correct syntax
    queue_dequeue(queue); // Skip '{' assuming correct syntax

    int startTime = atof(startTimeToken.lexeme);
    int offset = atof(durationToken.lexeme);
    int repetitions = atoi(repetitionsToken.lexeme);

    while  (!queue_is_empty(queue)) {
        Token token = queue_dequeue(queue);
        if (token.type == TOKEN_KEYWORD_PLAY) {
            handle_play_offset(queue, csvFile, symbolTable, startTime, offset, repetitions);
        } else if (token.type == TOKEN_PUNCT_RIGHT_CURLY) {
            break;
        }
    }
}


void analyze_and_export_to_csv(Queue *queue, const char *csvFileName, SymbolTable *symbolTable) {
    FILE *csvFile = fopen(csvFileName, "w");
    if (!csvFile) {
        perror("Error opening CSV file");
        return;
    }

    while (!queue_is_empty(queue)) {
        Token token = queue_dequeue(queue);

        if (token.type == TOKEN_KEYWORD_PLAY) {
            handle_play(queue, csvFile, symbolTable);
        }
        else if (token.type == TOKEN_KEYWORD_LOOP) {
            handle_loop(queue, csvFile, symbolTable);
        }
    }

    fclose(csvFile);
}

int main() {
    // Initialize and populate symbol table
    size_t num_tokens;
    Token *tokens = read_tokens_from_file("output.txt", &num_tokens);
    Parser parser = {tokens, num_tokens, 0};
    SymbolTable *symbolTable = parse_program(&parser);

    // Print symbol table (optional)
    print_symbol_table(symbolTable);

    // Export analyzed data to CSV
    Queue queue = {NULL, NULL};
    read_tokens_and_enqueue("output.txt", &queue);
    print_queue(&queue);
    analyze_and_export_to_csv(&queue, "output.csv", symbolTable);

    // Free resources
    free_symbol_table(symbolTable);
    free_token_resources(tokens, num_tokens);
    free(symbolTable);

    free_queue(&queue);

    return 0;
}

