// https://kaizen.place/music-tails/writing-wav-files-in-c-630150ea1b3ccbc86a636770
// TODO: figure out how to relate the numbers to the measures
// currently each number is 1 quarter note
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
 
struct soundhdr
{
 char riff[4];           /* "RIFF"                                  */
 int32_t flength;        /* file length in bytes                    */
 char wave[4];           /* "WAVE"                                  */
 char fmt[4];            /* "fmt "                                  */
 int32_t chunk_size;     /* size of FMT chunk in bytes (usually 16) */
 int16_t format_tag;     /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM */
 int16_t num_chans;      /* 1=mono, 2=stereo                        */
 int32_t srate;          /* Sampling rate in samples per second     */
 int32_t bytes_per_sec;  /* bytes per second = srate*bytes_per_samp */
 int16_t bytes_per_samp; /* 2=16-bit mono, 4=16-bit stereo          */
 int16_t bits_per_samp;  /* Number of bits per sample               */
 char data[4];           /* "data"                                  */
 int32_t dlength;        /* data length in bytes (filelength - 44)  */
};
 
struct soundhdr wavh; // write the wav header

struct Note {
  double freq;
  double start;
  double end;
  char wave[10];
};

typedef enum {
SIN_WAVE,
TRIANGLE_WAVE,
SQUARE_WAVE,
SAW_WAVE,
KICK
} wave_shape;


// misc file read function
#define BUF_SIZE 65536

int count_lines(FILE* file) // stolen from https://stackoverflow.com/a/70708991
{
    char buf[BUF_SIZE];
    int counter = 0;
    for(;;)
    {
        size_t res = fread(buf, 1, BUF_SIZE, file);
        if (ferror(file))
            return -1;

        int i;
        for(i = 0; i < res; i++)
            if (buf[i] == '\n')
                counter++;

        if (feof(file))
            break;
    }

    return counter;
}



const int sample_rate = 16000;
const int ms_per_beat = 500; // bpm of 120
const int samples_per_beat = (sample_rate * ms_per_beat) / 1000;
//const int buffer_size = num_measures_to_play * beats_per_measure * samples_per_beat;


// put all the different wave functions here

short int sin_wave(int i, double freq) {
  return (short int) (cos((2 * M_PI * freq * i) / sample_rate) * 3000);
}

short int square_wave(int i, double freq) {
  return fmodf(i*freq/sample_rate, 1.0 )> 0.5 ? 3000 : -3000;
}

short int saw_wave(int i, double freq) {
  return 2*(fmodf( (i*freq/sample_rate), 1.0)-0.5) * 3000;
}

short int triangle_wave(int i, double freq) {
  return 4*(fabs(fmodf( (i*freq/sample_rate), 1.0)-0.5)-0.25) * 3000;
}

short int kick_wave(int i, double freq) {
  if (i >= 6000){return 0;}
//  double freq2 = freq/2  + (freq) * ((1000000-i)/1000000);//*((8000-(i-start_index))/8000);
  double freq2 = (1-( (double)i  /20000))*(1-( (double)i  /20000))*(1-( (double)i  /20000)) * (freq-40) + 40;
  if (freq2<0){return 0;}
  return triangle_wave(i, freq2)*5*(1-(double)i/6000);

  //return (short int) (cos((2 * M_PI * freq2 * i) / sample_rate) * 3000 * (1-((double)i / 10000)));
}


void play(struct Note note, short int * buffer, int buffer_size)
{
  /*double current_beat = beats_per_measure * measure + beat;
  int start_index = current_beat * samples_per_beat;
  int end_index = start_index + duration * samples_per_beat;
  */
  int start_index = note.start * samples_per_beat;
  int end_index = note.end * samples_per_beat;
  double freq = note.freq;
  wave_shape wave;

  // Figure out what wave we are using
  if (strcmp(note.wave, "triangle") == 0) {wave = TRIANGLE_WAVE;}
  else if (strcmp(note.wave, "saw") == 0) {wave = SAW_WAVE;}
  else if (strcmp(note.wave, "square") == 0) {wave = SQUARE_WAVE;}
  else if (strcmp(note.wave, "kick") == 0) {wave = KICK;}
  else {wave = SIN_WAVE;} // set sin wave as the default


  for (int i = start_index; i < end_index+2000 && i < buffer_size; i++)
  {
    //double percent_through_note = ((double )i - start_index) / (end_index - start_index);
    double amplitude_multiplier = 1.0f;
    int current_position = i-start_index;

    if (current_position < 2000) // attack of 2000 samples
    {
      amplitude_multiplier = current_position / 2000;

    }
    else if (current_position < 1400) // decay of 400 samples to 80%
    {
      amplitude_multiplier = 0.8 + 0.2 * (1400-(current_position - 1000))/400;
    }
    else if (current_position  > end_index - start_index) // release of 2000 samples
    {
      amplitude_multiplier = 0.8 * (2000-(i-end_index))/2000;
    }
    else // sustain
    {
      amplitude_multiplier = 0.8;
    }
    // Now we need to choose which wave we are dealing with
    switch (wave) {
      case SIN_WAVE:
        buffer[i] += sin_wave(current_position, freq) * amplitude_multiplier;
        break;
      case TRIANGLE_WAVE:
        buffer[i] += triangle_wave(current_position, freq) * amplitude_multiplier;
        break;
      case SAW_WAVE:
        buffer[i] += saw_wave(current_position, freq) * amplitude_multiplier;
        break;
      case SQUARE_WAVE:
        buffer[i] += square_wave(current_position, freq) * amplitude_multiplier;
        break;
      case KICK:
        buffer[i] += kick_wave(current_position, freq) * amplitude_multiplier;
        break;
    }

  }
}



int main(int argc, char **argv)
{
  FILE *fp_notes;
  int num_lines;
  int i = 0;
  int max_end = 0;
  double freq;
  double start;
  double end;
  char wave[10];


  //// Read in the csv file and load the notes
  fp_notes = fopen("notes.csv", "r");
  
  

  // count number of lines and allocate memory for the Notes struct array
  num_lines = count_lines(fp_notes);
  printf("Number of notes to play: %d\n\n", num_lines);
  rewind(fp_notes);
  struct Note * notes = (struct Note *) malloc(sizeof(struct Note) * (num_lines+ 9)); // add a bit of space


  // Read the actual notes and add to structs array
  // Also determine the ending time for the notes
  i = 0;
  while (fscanf(fp_notes, "%lf,%lf,%lf,%s", &freq, &start, &end, wave) !=EOF) {
    notes[i].freq = freq; // fill in the note object
    notes[i].start = start;
    notes[i].end = end;
    strcpy(notes[i].wave, wave);
    i++;
    max_end = end > max_end ? end : max_end; // figure out when the last note ends to determine buffer size
  }
  printf("ends at: %d\n", max_end);

  // Now we can decide the buffer size ie how long the resulting file will be
  int buffer_size = (max_end + 1) * samples_per_beat; // add 1 more beat for smoother release



  short int * buffer = (short int *) calloc(buffer_size + 1, sizeof(short int)); // use calloc to initialise to 0
  
  for(int i = 0; i < num_lines; i++) {
    play(notes[i], buffer, buffer_size);
  }



  // write the wav header
 
  strncpy(wavh.riff, "RIFF", 4);
  strncpy(wavh.wave, "WAVE", 4);
  strncpy(wavh.fmt, "fmt ", 4);
  strncpy(wavh.data, "data", 4);

  wavh.chunk_size = 16;
  wavh.format_tag = 1;
  wavh.num_chans = 1;
  wavh.srate = sample_rate;
  wavh.bits_per_samp = 16;
  
  wavh.flength = buffer_size + 44;
  const int num_bytes = 2;
  wavh.dlength = buffer_size * num_bytes;

  FILE *fp = fopen("test.wav", "wb");
  fwrite(&wavh, 1, 44, fp);
  fwrite(buffer, 2, buffer_size, fp);
  free(buffer);
}
