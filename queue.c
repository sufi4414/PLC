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

void analyze_and_export_to_csv(Queue *queue, const char *csvFileName, SymbolTable *symbolTable) {
    FILE *csvFile = fopen(csvFileName, "w");
    if (!csvFile) {
        perror("Error opening CSV file");
        return;
    }

    while (!queue_is_empty(queue)) {
        Token token = queue_dequeue(queue);

        if (token.type == TOKEN_KEYWORD_PLAY) {
            queue_dequeue(queue); // Skip '(' assuming correct syntax

            Token identifierToken = queue_dequeue(queue); // Could be a note or a chord identifier

            // Check if identifier is a chord in the symbol table
            Chord *chord = symbol_table_lookup_chord(symbolTable, identifierToken.lexeme);
            if (chord) {
                queue_dequeue(queue); // Skip ','
                Token firstNumberToken = queue_dequeue(queue); // First number
                
                queue_dequeue(queue); // Skip ','
                Token secondNumberToken = queue_dequeue(queue); // Second number
                // queue_dequeue(queue); // Skip ')' at the end, assuming correct syntax

                // It's a chord, handle each note in the chord
                for (int i = 0; i < chord->noteCount; i++) {
                    fprintf(csvFile, "%s, ", chord->notes[i].name);
                    fprintf(csvFile, "%s, %s, %s\n", firstNumberToken.lexeme, secondNumberToken.lexeme, chord->notes[i].wave);
                }
                
                queue_dequeue(queue); // Skip ')' at the end, assuming correct syntax
            } else {
                // Assume it's a note if not found as a chord
                fprintf(csvFile, "%s, ", identifierToken.lexeme);

                queue_dequeue(queue); // Skip '@'

                Token waveToken = queue_dequeue(queue); 
                queue_dequeue(queue); // Skip ','

                Token firstNumberToken = queue_dequeue(queue); // First number
                queue_dequeue(queue); // Skip ','

                Token secondNumberToken = queue_dequeue(queue); // Second number
                queue_dequeue(queue); // Skip ')' at the end, assuming correct syntax

                // Continue writing the rest of the data to CSV
                fprintf(csvFile, "%s, %s, %s\n", firstNumberToken.lexeme, secondNumberToken.lexeme, waveToken.lexeme);
            }
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

