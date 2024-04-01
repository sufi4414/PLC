// https://kaizen.place/music-tails/writing-wav-files-in-c-630150ea1b3ccbc86a636770
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
};

 // https://pages.mtu.edu/~suits/notefreqs.html
 const double D2 = 73.42;
 const double G2 = 98.00;
 const double A2 = 110.00;
 const double B2 = 123.47;
 const double D3 = 146.83;
 const double G3 = 196.00;
 const double A3 = 220.00;
 const double B3 = 246.94;
 const double Cs4 = 277.18;
 const double D4 = 293.66;
 const double E4 = 329.63;
 const double Fs4 = 369.99;
 const double G4 = 392.00;
 const double A4 = 440.00;
 const double B4 = 493.88;
 
 /*
 const int num_measures_to_play = 4 * 4;
 const int sample_rate = 16000;
 const int beats_per_measure = 4;
 const int ms_per_beat = 500;
 const int samples_per_beat = (sample_rate * ms_per_beat) / 1000;
 const int buffer_size = num_measures_to_play * beats_per_measure * samples_per_beat;*/
 //short int buffer[buffer_size] = {};
//#define NUM_MEASURES_TO_PLAY 4 * 4
#define NUM_MEASURES_TO_PLAY 1
#define SAMPLE_RATE 16000
#define BEATS_PER_MEASURE 4
#define MS_PER_BEAT 500
#define SAMPLES_PER_BEAT (SAMPLE_RATE * MS_PER_BEAT) / 1000
#define BUFFER_SIZE NUM_MEASURES_TO_PLAY * BEATS_PER_MEASURE * SAMPLES_PER_BEAT
short int buffer[BUFFER_SIZE] = {};

const int num_measures_to_play = 1;
const int sample_rate = 16000;
const int beats_per_measure = 4;
const int ms_per_beat = 500;
const int samples_per_beat = (sample_rate * ms_per_beat) / 1000;
const int buffer_size = num_measures_to_play * beats_per_measure * samples_per_beat;


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

short int sin_squish_wave(int i, double freq) {
  if (i >= 6000){return 0;}
//  double freq2 = freq/2  + (freq) * ((1000000-i)/1000000);//*((8000-(i-start_index))/8000);
  double freq2 = (1-( (double)i  /20000))*(1-( (double)i  /20000))*(1-( (double)i  /20000)) * (freq-40) + 40;
  if (freq2<0){return 0;}
  return sin_wave(i, freq2)*5*(1-(double)i/6000);

  //return (short int) (cos((2 * M_PI * freq2 * i) / sample_rate) * 3000 * (1-((double)i / 10000)));
}


void play(struct Note note)
{
  /*double current_beat = beats_per_measure * measure + beat;
  int start_index = current_beat * samples_per_beat;
  int end_index = start_index + duration * samples_per_beat;
  */
  int start_index = note.start * samples_per_beat;
  int end_index = note.end * samples_per_beat;
  double freq = note.freq;

  for (int i = start_index; i < end_index+2000 && i < buffer_size; i++)
  {
    //double percent_through_note = ((double )i - start_index) / (end_index - start_index);
    double amplitude_multiplier = 1.0f;
    int current_position = i-start_index;

    if (current_position < 2000) // attack of 2000 samples
    {
      //amplitude_multiplier = (percent_through_note / 0.25);
      //amplitude_multiplier = i*(1/2000);
      amplitude_multiplier = current_position / 2000;

    }
    else if (current_position < 1400) // decay of 400 samples to 80%
    {
      //amplitude_multiplier = 0.8 + 0.2 * (1400-i) * 1/400;
      amplitude_multiplier = 0.8 + 0.2 * (1400-(current_position - 1000))/400;
    }
    else if (current_position  > end_index - start_index) // release of 2000 samples
    {
      //amplitude_multiplier = 0.8 * (1 - (i-end_index) * (1/2000));
      amplitude_multiplier = 0.8 * (2000-(i-end_index))/2000;
    }
    else // sustain
    {
      amplitude_multiplier = 0.8;
    }

    //buffer[i] += (short int)((cos((2 * M_PI * freq * i) / sample_rate) * 3000) * amplitude_multiplier);
    buffer[i] += sin_squish_wave(current_position, freq) * amplitude_multiplier;
  }
}



int main(void)
{
  strncpy(wavh.riff, "RIFF", 4);
  strncpy(wavh.wave, "WAVE", 4);
  strncpy(wavh.fmt, "fmt ", 4);
  strncpy(wavh.data, "data", 4);

  wavh.chunk_size = 16;
  wavh.format_tag = 1;
  wavh.num_chans = 1;
  wavh.srate = sample_rate;
  wavh.bits_per_samp = 16;

  int sample_index_in_beat = 0;
  int beat_index = 0;

  int measure = 0;
  
  struct Note notes[] = {
    /*{G3, 0, 1.0},
    {B3, 1.0, 2.0},
    {D4, 2.0, 3.0},*/
    {100.0, 0, 3.0},
    {100.0, 1.0, 3.0},
    {100.0, 2.0, 3.0},
    {0,0,0}
  };
   int noteNum = sizeof(notes)/sizeof(struct Note);
  for(int i = 0; i < noteNum; i++){
    play(notes[i]);
  }


  wavh.flength = buffer_size + 44;
  const int num_bytes = 2;
  wavh.dlength = buffer_size * num_bytes;

  FILE *fp = fopen("test.wav", "wb");
  fwrite(&wavh, 1, 44, fp);
  fwrite(buffer, 2, buffer_size, fp);
}
