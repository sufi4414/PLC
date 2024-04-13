#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

struct soundhdr {
    char riff[4];
    int32_t flength;
    char wave[4];
    char fmt[4];
    int32_t chunk_size;
    int16_t format_tag;
    int16_t num_chans;
    int32_t srate;
    int32_t bytes_per_sec;
    int16_t bytes_per_samp;
    int16_t bits_per_samp;
    char data[4];
    int32_t dlength;
};

struct soundhdr wavh;

#define SAMPLE_RATE 16000
#define BUFFER_SIZE (SAMPLE_RATE * 3) // Assuming 3 seconds of audio
short int buffer[BUFFER_SIZE];

struct Note {
    double freq;
    double start;
    double end;
    char wave_type[10]; // Assuming a maximum length of 10 characters for wave type
};

void generate_audio(struct Note *notes, int num_notes) {
    int sample_index = 0;
    for (int i = 0; i < num_notes; i++) {
        struct Note note = notes[i];
        int start_index = note.start * SAMPLE_RATE;
        int end_index = note.end * SAMPLE_RATE;
        for (int j = start_index; j < end_index && sample_index < BUFFER_SIZE; j++) {
            double t = (double) j / SAMPLE_RATE;
            if (strcmp(note.wave_type, "sin") == 0) {
                buffer[sample_index++] = (short int)(sin(2 * M_PI * note.freq * t) * 3000);
            } else if (strcmp(note.wave_type, "square") == 0) {
                buffer[sample_index++] = (short int)(fmod(t * note.freq, 1.0) > 0.5 ? 3000 : -3000);
            } else if (strcmp(note.wave_type, "saw") == 0) {
                buffer[sample_index++] = (short int)(2 * (fmod(t * note.freq, 1.0) - 0.5) * 3000);
            } else if (strcmp(note.wave_type, "triangle") == 0) {
                buffer[sample_index++] = (short int)(4 * (fabs(fmod(t * note.freq, 1.0) - 0.5) - 0.25) * 3000);
            }
        }
    }
}

int main(void) {
    strncpy(wavh.riff, "RIFF", 4);
    strncpy(wavh.wave, "WAVE", 4);
    strncpy(wavh.fmt, "fmt ", 4);
    strncpy(wavh.data, "data", 4);
    wavh.chunk_size = 16;
    wavh.format_tag = 1;
    wavh.num_chans = 1;
    wavh.srate = SAMPLE_RATE;
    wavh.bits_per_samp = 16;

    // Read data from CSV file
    FILE *csv_file = fopen("output.csv", "r");
    if (!csv_file) {
        perror("Error opening CSV file");
        return 1;
    }

    int num_notes = 0;
    struct Note notes[100]; // Assuming maximum 100 notes
    char line[256];
    while (fgets(line, sizeof(line), csv_file)) {
        double freq, start, end;
        char wave_type[10];
        if (sscanf(line, "%lf,%lf,%lf,%9s", &freq, &start, &end, wave_type) == 4) {
            notes[num_notes].freq = freq;
            notes[num_notes].start = start;
            notes[num_notes].end = end;
            strncpy(notes[num_notes].wave_type, wave_type, 10);
            num_notes++;
        }
    }
    fclose(csv_file);

    // Generate audio samples
    generate_audio(notes, num_notes);

    // Write audio samples to WAV file
    wavh.flength = sizeof(wavh) + sizeof(buffer);
    wavh.dlength = sizeof(buffer);
    FILE *wav_file = fopen("output.wav", "wb");
    if (!wav_file) {
        perror("Error opening WAV file");
        return 1;
    }
    fwrite(&wavh, sizeof(wavh), 1, wav_file);
    fwrite(buffer, sizeof(short int), BUFFER_SIZE, wav_file);
    fclose(wav_file);

    return 0;
}
